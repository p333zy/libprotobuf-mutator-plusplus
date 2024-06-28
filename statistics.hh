#include <cstdint>
#include <fstream>

#include "utils.hh"

namespace lpmpp {

static const char * const kMutatorDefaultOutfile = "/tmp/afl-state.csv";

class MutatorStatsBase {
public:
  void inc_customfuzz() {}
  void inc_customfuzz_parsefail() {}
  void inc_customfuzz_addbuf_parsefail() {}
  void add_customfuzz_addbuf_provided(uint64_t x) { UNUSED(x); }

  bool ok() { 
    return true; 
  }

  void begin() {}
  void end() {}
};


class MutatorStats : public MutatorStatsBase {
#ifdef MUTATOR_TRACK_STATS
private:
  uint64_t c = 0;
  uint64_t customfuzz = 0;
  uint64_t customfuzz_parsefail = 0;
  uint64_t customfuzz_addbuf_provided = 0;
  uint64_t customfuzz_addbuf_parsefail = 0;
  std::ofstream outfile;

public:
  MutatorStats() : outfile(kMutatorDefaultOutfile) {}
  MutatorStats(const char *outpath) : outfile(outpath) {}

  void inc_customfuzz() { customfuzz++; }
  void inc_customfuzz_parsefail() { customfuzz_parsefail++; }
  void inc_customfuzz_addbuf_parsefail() { customfuzz_addbuf_parsefail++; }
  void add_customfuzz_addbuf_provided(uint64_t x) { 
    customfuzz_addbuf_provided += x;
  }

  bool ok() {
    return !outfile.fail();  
  }

  void begin() { c++; }

  void end() {
    if (c % 10000 == 0) {
      flush();
      reset();
    }
  }

private:
  /**
   * @brief Flush stats to underlying storage
   * 
   */
  void flush();

  void reset() {
    customfuzz = 0;
    customfuzz_parsefail = 0;
    customfuzz_addbuf_provided = 0;
    customfuzz_addbuf_parsefail = 0;
  }
#else
public:
  MutatorStats() {}
  MutatorStats(const char *outpath) { UNUSED(outpath); }
#endif
};

}