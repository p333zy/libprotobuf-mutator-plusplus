#include <cmath>

#include "utils.hh"

namespace lpmpp {
    
std::size_t NextPow2(std::size_t n) {
  return std::pow(2, std::ceil(std::log(n) / std::log(2)));
}

}