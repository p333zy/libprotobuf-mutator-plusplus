#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <google/protobuf/message.h>
#include <afl/afl-fuzz.h>
#include <afl/alloc-inl.h>

#include "libprotobuf-mutator/src/mutator.h"
#include "statistics.hh"
#include "trimming.hh"
#include "utils.hh"


namespace lpmpp {

class Mutator : public protobuf_mutator::Mutator {
  
};

class MutatorState {
private:
  std::vector<uint8_t> buf_;
  std::size_t bufsize_ = 0;
  std::unique_ptr<Trimmer> trimmer_;
  MutatorStats stats_;
  Mutator mutator_;
  int trimindex_;

public:
  MutatorState(unsigned int seed) {
    mutator_.Seed(seed);
  }

  uint8_t * buf() {
    return buf_.data();
  }

  std::size_t bufsize() const {
    return bufsize_;
  }

  Trimmer * trimmer() const {
    return trimmer_.get();
  }

  void set_trimmer(std::unique_ptr<Trimmer> trimmer) {
    trimmer_ = std::move(trimmer);
  }

  MutatorStats & stats()  {
    return stats_;
  }

  Mutator & mutator() {
    return mutator_;
  }

  int trimindex() const {
    return trimindex_;
  }

  void set_trimindex(int index) {
    trimindex_ = index;
  }

  std::size_t Store(const uint8_t *buf, std::size_t size) {
    if (buf_.size() < size) {
      buf_.resize(size);
    }

    bufsize_ = size;
    memcpy(buf_.data(), buf, size);
    return size;
  }

  MutatorState& operator=(MutatorState& other) = delete;
  MutatorState& operator=(MutatorState&& other) = delete;
};


void Mutate(MutatorState *state, google::protobuf::Message &value, 
              std::size_t max_size) {
  /* From libprotobuf-mutator source: "Method does not guarantee
  that real result will be strictly smaller than value. Caller
  should repeat mutation if result was larger than expected" */

  int i = 0;

  do {
    state->mutator().Mutate(&value, max_size);
  } while(value.ByteSizeLong() > max_size && ++i < 10);
}

inline bool ShouldCrossover() {
  return rand() % 100 < 6;
}

/* ------------------------------ */
/* ---- AFL Bindings ------------ */
/* ------------------------------ */

void *init(afl_state_t *afl, unsigned int seed) {
  srand(seed);
  MutatorState *state = new MutatorState(seed);
  return static_cast<void *>(state);
}

void deinit(MutatorState *state) {
  delete state;
}

template<Derived<google::protobuf::Message> T>
std::size_t fuzz(MutatorState *state, unsigned char *buf, 
  std::size_t buf_size, unsigned char **outbuf, unsigned char *add_buf, 
  std::size_t add_buf_size, std::size_t max_size) {
  
  google::protobuf::LogSilencer silencer;
  T proto;
  
  state->stats().begin();
  state->stats().inc_customfuzz();

  // Parse buf into proto
  if (!proto.ParseFromArray(buf, buf_size)) {
    *outbuf = NULL;
    state->stats().inc_customfuzz_parsefail();
    return 0;
  }

  // Mutate values
  Mutate(state, proto, max_size);

  // Optionally merge in contents from add_buf
  state->stats().add_customfuzz_addbuf_provided(add_buf_size > 0 ? 1 : 0);
  if (ShouldCrossover()) {
    if (T merge_proto; merge_proto.ParseFromArray(add_buf, add_buf_size)) {
      state->mutator().CrossOver(merge_proto, &proto, max_size);
    } else {
      state->stats().inc_customfuzz_addbuf_parsefail();
    }
  }

  // Convert protobuf to string 
  std::string str(Serialize(proto));

  // Copy to state->mutator_out
  state->Store((const uint8_t *) str.c_str(), str.length());

  // Write mutator_out to out_buf
  *outbuf = state->buf();

  state->stats().end();
  return state->bufsize();
}

template<Derived<google::protobuf::Message> T>
int init_trim(MutatorState *state, unsigned char *buf, std::size_t buf_size) {
  google::protobuf::LogSilencer silencer;
  std::unique_ptr<google::protobuf::Message> proto = std::make_unique<T>();

  if (!proto->ParseFromArray(buf, buf_size))
    return 0;

  state->set_trimindex(0);
  state->set_trimmer(std::make_unique<Trimmer>(std::move(proto)));
  return 1;
}

std::size_t trim(MutatorState *state, unsigned char **outbuf) {
  state->trimmer()->TrimOne();
  return state->trimmer()->Serialize(outbuf);
}

int post_trim(MutatorState *state, unsigned char success) {
  if (!success)
    state->trimmer()->Revert();
  
  return state->trimmer()->done() == 0 ? 1 : 0;
}

std::size_t post_process(MutatorState *state, unsigned char *buf, 
  std::size_t buf_size, unsigned char **outbuf) {
  
  // TODO: end stats here
  *outbuf = buf;
  return buf_size;
}

};
