/**
 * Harness for AFL++ and libprotobuf-mutator++
 */

#include <cstdlib>
#include <cstdint>

#include "../vuln.hh"

extern "C" {

int LLVMFuzzerTestOneInput(const char *Data, std::size_t Size) {
  std::string payload(Data, Size);
  vuln::RPCEntry(payload);
  return 0;
}

}