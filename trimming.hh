#pragma once

#include <string>
#include <stack>
#include <exception>
#include <cstddef>
#include <cstdlib>

#include <google/protobuf/any.pb.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/message_differencer.h>
#include <google/protobuf/wire_format.h>

namespace lpmpp {

enum TrimType {
  NODES,
  STRINGS, 
  NONE,
};

const char * const TrimTypeDesc[] = {
  "NODES",
  "STRINGS",
  "NONE",
};


class TrimTask {
protected:
  google::protobuf::Message &msg;
  const google::protobuf::FieldDescriptor &field;
  std::string path_;
  int rindex;

private:
  bool started_ = false;

protected:

  /**
   * @brief Construct a new Trimmer Task for `field` at `msg`.
   * 
   * @param msg the message containing field
   * @param field the descriptor for the field to be trimmed
   * @param path the path from the root node to field
   * @param rindex the repeated field index, or -1 if the field is not repeated
   */
  TrimTask(google::protobuf::Message &msg, 
           const google::protobuf::FieldDescriptor &field, 
           std::string path, int rindex) :
    msg(msg),
    field(field),
    path_(path),
    rindex(rindex) {}

public:
  virtual ~TrimTask() {};

  const std::string &path() const { 
    return path_; 
  }
  
  bool is_repeated() const { 
    return rindex != -1; 
  }

  bool started() const {
    return started_;
  }

  void set_started(bool started) {
    started_ = started;
  }

  /**
   * @brief returns true if the task is finished
   * 
   */
  virtual bool done() {
    return true;
  }

  /**
   * @brief Trims the field
   */
  virtual void Trim() = 0;

};


/**
 * @brief Reduce the length of string fields
 * 
 */
class StringTrimTask : public TrimTask {
private:
  int nsteps = 0;

public:
  static bool CanHandle(const google::protobuf::FieldDescriptor &field) {
    return field.cpp_type() == google::protobuf::FieldDescriptor::CppType::CPPTYPE_STRING;
  }

  StringTrimTask(google::protobuf::Message &msg,
                const google::protobuf::FieldDescriptor &field, 
                std::string path, 
                int rindex) :
    TrimTask(msg, field, path, rindex) {}

  void Trim() override;
  bool done() override;

private:
  std::string string() const {
    const google::protobuf::Reflection &reflection = *msg.GetReflection();

    if (is_repeated())
      return reflection.GetRepeatedString(msg, &field, rindex);
  
    return reflection.GetString(msg, &field);
  }

  void set_string(std::string &str) {
    const google::protobuf::Reflection &reflection = *msg.GetReflection();

    if (is_repeated()) {
      reflection.SetRepeatedString(&msg, &field, rindex, str);
    } else {
      reflection.SetString(&msg, &field, str);
    }
  }
};


/**
 * @brief Remove optional nodes
 * 
 */
class NodeTrimTask : public TrimTask {
public:
  static bool CanHandle(const google::protobuf::FieldDescriptor &field) {
    return field.is_optional() || field.is_repeated();
  }

  NodeTrimTask(google::protobuf::Message &msg, 
               const google::protobuf::FieldDescriptor &field, 
               std::string path, 
               int rindex) :
    TrimTask(msg, field, path, rindex) {}

  void Trim() override;
};


class Trimmer {
private:
  std::unique_ptr<google::protobuf::Message> msg;
  std::unique_ptr<google::protobuf::Message> pmsg;
  std::stack<std::shared_ptr<TrimTask>> tasks;
  std::set<std::string> processed;
  std::vector<uint8_t> buf;
  TrimType trim_type_ = TrimType::NODES;

  struct FieldInfo {
    std::string &rootpath;
    google::protobuf::Message &msg;
    const google::protobuf::FieldDescriptor &field;
  };

public:
  typedef std::pair<std::string, google::protobuf::Message&> MessageStackE;
  typedef std::stack<MessageStackE> MessageStack;

  Trimmer(std::unique_ptr<google::protobuf::Message> message);

  bool done() const {
    return trim_type_ == TrimType::NONE;
  }

  const google::protobuf::Message * message() const {
    return msg.get();
  }

  TrimType trim_type() const {
    return trim_type_;
  }

  /**
   * @brief Trims one field in message.
   * 
   */
  void TrimOne();

  /**
   * @brief Revert message to the state before the last TrimOne()
   * 
   */
  void Revert();

  /**
   * @brief Serializes the message and writes a pointer to the bytes to `outbuf`. 
   * The pointer is managed by the Trimmer instance. Returns the length of the
   * output buffer.
   * 
   * @param outbuf location to write the output buffer pointer
   * @return size_t the length of the output buffer
   */
  std::size_t Serialize(uint8_t **outbuf);

private:
  /**
   * @brief Creates TrimTask for all paths reachable via the root message
   * that have not already been processed.
   * 
   */
  void PopulateTasks();

  /**
   * @brief Adds TrimTask to `tasks` for any paths reachable via `info` that
   * have not already been processed. Adds Message instances to accumulator.
   * 
   * @param accumulator accumulator for any Message contained in the field
   * @param info information about the protobuf field
   */
  void CreateTask(MessageStack &accumulator, const FieldInfo &info);
};

}