#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <google/protobuf/unknown_field_set.h>

#include "trimming.hh"
#include "utils.hh"

using namespace google::protobuf;

namespace lpmpp {

/* -------------------------------------- */
/* --- Trimming utils ------------------- */
/* -------------------------------------- */

static const std::size_t kTrimStrLen = 14;
static const std::size_t kMinStrLength = 5;
static const std::size_t kMinStrStep = 4;


/**
 * @brief Return the unique identifier for the field for the TrimType. 
 * Uses the path to the object from the root node, e.g. "NODES.1.2[0].1.2"
 * 
 */
static std::string GetID(const std::string &rootpath, const FieldDescriptor &field, int rindex) {
  std::stringstream s;
  s << rootpath << "." << field.index();
  
  if (rindex != -1) 
    s << "[" << rindex << "]";
  
  return s.str();

}


static std::string& TruncateString(std::string &str) {
  // Simplified logic from AFL++

  if (str.length() < kMinStrLength)
    return str;

  std::size_t step = std::max(NextPow2(str.size()) / 16, kMinStrStep);
  str.erase(str.length() - step, step);
  return str;
}


/**
 * @brief Trim nodes first, then strings. Avoids unnecessary trimming
 * against nodes that do not generate interesting coverage. Downside
 * is we cannot know upfront how many trimming operations there are for 
 * the message.
 *
 */
static TrimType NextTrimType(TrimType type) {
  switch (type) {
    case NODES:
      return STRINGS;
    case STRINGS:
      return NONE;
    case NONE:
      return NONE;
  }

  __builtin_unreachable();
}


static bool ShouldTrim(const TrimType type, const FieldDescriptor &field) {
  switch(type) {
    case STRINGS:
      return StringTrimTask::CanHandle(field);
    case NODES:
      return NodeTrimTask::CanHandle(field);
    case NONE:
      return false;
  }

  __builtin_unreachable();
}


static std::shared_ptr<TrimTask> MakeTrimTask(const TrimType type, Message &msg, 
  const FieldDescriptor &field, std::string path, int rindex) {

  switch(type) {
    case STRINGS:
      return std::make_shared<StringTrimTask>(msg, field, path, rindex);
    case NODES:
      return std::make_shared<NodeTrimTask>(msg, field, path, rindex);
    case NONE:
      throw std::logic_error("Cannot create TrimTask for TrimType::NONE");
  }

  __builtin_unreachable();
}

/* -------------------------------------- */
/* --- TrimTask method definitions ------ */
/* -------------------------------------- */

static const int kMaxStrSteps = 128;

bool StringTrimTask::done() {
  if (nsteps > kMaxStrSteps)
    return true;
  
  const std::string str = string();
  return str.size() < kMinStrLength;
}

void StringTrimTask::Trim() {
  std::string str = string();
  TruncateString(str);
  set_string(str);
  nsteps++;
}

void NodeTrimTask::Trim() {
  const Reflection &reflection = *msg.GetReflection();

  if (!is_repeated()) {
    return reflection.ClearField(&msg, &field);
  }

  // TODO: can this be done without O(n) swaps? Protobuf API only provides
  // capability to pop the last element of a repeated list

  do {
    if (rindex == reflection.FieldSize(msg, &field) - 1) {
      reflection.RemoveLast(&msg, &field);
      break;
    }
    int nindex = rindex + 1;
    reflection.SwapElements(&msg, &field, rindex, nindex);
    rindex = nindex;
  } while (true);
}

/* -------------------------------------- */
/* --- Trimmer method definitions ------- */
/* -------------------------------------- */

Trimmer::Trimmer(std::unique_ptr<Message> message) : 
    msg(std::move(message)), 
    pmsg(msg->New()) {

  buf.resize(0x1000);
  pmsg->CopyFrom(*msg);
  PopulateTasks();
}


void Trimmer::PopulateTasks() {
  if (trim_type_ == TrimType::NONE) {
    return;
  }

  MessageStack messages;

  // Traverse the protobuf message tree, pushing messages onto the stack so 
  // that we process them depth-first. Keep track of the path from the root
  // in order to support reverts.

  messages.push(MessageStackE(TrimTypeDesc[trim_type_], *msg));

  while (!messages.empty()) {
    std::string rootpath = std::move(messages.top().first);
    Message &nmsg = messages.top().second;
    messages.pop();

    const Reflection *reflection = nmsg.GetReflection();
    std::vector<const FieldDescriptor *> descs;
    reflection->ListFields(nmsg, &descs);

    for (const FieldDescriptor *desc : descs) {
      const FieldInfo info {
        .rootpath = rootpath,
        .msg = nmsg,
        .field = *desc
      };

      CreateTask(messages, info);
    }
  }
}


void Trimmer::CreateTask(MessageStack &accumulator, const FieldInfo &info) {
  const FieldDescriptor &desc = info.field;
  const Reflection *reflection = info.msg.GetReflection();

  // Create a TrimTask for all fields valid for the current trim_type. If 
  // the field is a Message, add to the MessageStack. We skip fields and all
  // of its descendants if the field has already been processed.

  if (!desc.is_repeated()) {
    std::string path(GetID(info.rootpath, desc, -1));
    if (processed.contains(path)) {
      return;
    }

    if (desc.cpp_type() == FieldDescriptor::CppType::CPPTYPE_MESSAGE) {
      Message &m = *reflection->MutableMessage(&info.msg, &desc);
      accumulator.push(MessageStackE(path, m));
    }

    if (ShouldTrim(trim_type_, desc)) {
      tasks.push(MakeTrimTask(trim_type_, info.msg, desc, path, -1));
    }
    
    return;
  }

  // For repeated fields, create a TrimTask for each entry in the field.

  for (int i = 0; i < reflection->FieldSize(info.msg, &desc); i++) {
    std::string path(GetID(info.rootpath, desc, i));
    if (processed.contains(path)) {
      return;
    }

    if (desc.cpp_type() == FieldDescriptor::CppType::CPPTYPE_MESSAGE)  {
      Message &m = *reflection->MutableRepeatedMessage(&info.msg, &desc, i);
      accumulator.push(MessageStackE(path, m));
    }

    if (ShouldTrim(trim_type_, desc)) {
      tasks.push(MakeTrimTask(trim_type_, info.msg, desc, path, i));
    }
  }
}


void Trimmer::TrimOne() {
  if (tasks.empty()) {
    trim_type_ = NextTrimType(trim_type_);
    PopulateTasks();
  }

  if (tasks.empty())
    return;
  
  // Stash current state for reverts
  pmsg->CopyFrom(*msg);

  auto task = tasks.top();
  task->set_started(true);
  task->Trim();

  // Keep track of processed paths for reverts
  processed.insert(task->path());
  
  if (task->done())
    tasks.pop();
}


void Trimmer::Revert() {
  // Revert root message to the last state
  msg->CopyFrom(*pmsg);

  if (tasks.empty())
    trim_type_ = NextTrimType(trim_type_);

  // Repopulate the tasks list in all cases since Message::CopyFrom is destructive
  tasks = std::stack<std::shared_ptr<TrimTask>>();
  PopulateTasks();
}


std::size_t Trimmer::Serialize(uint8_t **outbuf) {
  std::size_t len = msg->ByteSizeLong();

  if (buf.size() < len) {
    buf.resize(len);
  }
  
  msg->SerializeToArray(buf.data(), len);
  *outbuf = buf.data();
  return len;
}

}