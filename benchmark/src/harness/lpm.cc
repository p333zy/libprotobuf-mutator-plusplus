/**
 * Harness for libprotobuf-mutator/libfuzzer
 * 
 */

#include <cstdlib>
#include <cstdint>
#include <sstream>
#include "nlohmann/json.hpp"
#include "proto/vuln.pb.h"
#include "convert/convert.hh"
#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"

#include "../vuln.hh"

using size_t = std::size_t;
  
extern "C" {

DEFINE_BINARY_PROTO_FUZZER(const vuln::fuzz::RPCCall &proto) {
  std::stringstream stream;
  stream << proto;
  vuln::RPCEntry(stream.str());
}

}