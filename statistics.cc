#include <chrono>
#include <sstream>

#include "statistics.hh"

#ifdef MUTATOR_TRACK_STATS

namespace lpmpp {

void MutatorStats::flush() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t t = std::chrono::system_clock::to_time_t(now);

  std::stringstream s;

  s << t << ","
    << customfuzz << ","
    << customfuzz_addbuf_parsefail << ","
    << customfuzz_addbuf_provided << ","
    << customfuzz_addbuf_parsefail << "\n";

  outfile << s.str();
}

}

#endif
