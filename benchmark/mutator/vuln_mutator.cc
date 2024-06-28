#include <string>
#include <sstream>

#include "mutator.hh"
#include "proto/vuln.pb.h"
#include "nlohmann/json.hpp"
#include "convert/convert.hh"

namespace fuzz = vuln::fuzz;
namespace json = nlohmann::detail;

std::vector<char> OutputBuffer;

extern "C" {

void *afl_custom_init(afl_state_t *afl, unsigned int seed) {
  return lpmpp::init(afl, seed);
}

void afl_custom_deinit(lpmpp::MutatorState *state) {
  return lpmpp::deinit(state);
}

std::size_t afl_custom_fuzz(lpmpp::MutatorState *state, unsigned char *buf, 
  std::size_t buf_size, unsigned char **outbuf, unsigned char *addbuf, 
  std::size_t addbuf_size, std::size_t max_size) {
  
  return lpmpp::fuzz<fuzz::RPCCall>(state, buf, buf_size, outbuf, addbuf, addbuf_size, max_size);
}

int afl_custom_init_trim(lpmpp::MutatorState *state, unsigned char *buf, std::size_t buf_size) {
  return lpmpp::init_trim<fuzz::RPCCall>(state, buf, buf_size);
}

std::size_t afl_custom_trim(lpmpp::MutatorState *state, unsigned char **outbuf) {
  return lpmpp::trim(state, outbuf);
}

int afl_custom_post_trim(lpmpp::MutatorState *state, unsigned char success) {
  return lpmpp::post_trim(state, success);
}

std::size_t afl_custom_post_process(lpmpp::MutatorState *state, unsigned char *buf, 
  std::size_t buf_size, char **outbuf) {
  
  fuzz::RPCCall proto;

  if (!proto.ParseFromArray(buf, buf_size)) {
    *outbuf = nullptr;
    return 0;
  }

  std::stringstream stream;
  stream << proto;
  const std::string str = stream.str();

  if (OutputBuffer.size() < str.size()) {
    OutputBuffer.clear();
    OutputBuffer.resize(str.size());
  }

  char *out = OutputBuffer.data();
  memcpy(out, str.c_str(), str.size());
  *outbuf = out;
  return str.size();

}

}