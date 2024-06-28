#include "nlohmann/json.hpp"
#include "convert.hh"

namespace fuzz = vuln::fuzz;
namespace json = nlohmann::detail;

std::stringstream& operator<<(std::stringstream &lhs, const fuzz::RPCArgument &proto) {
  lhs << "{\"key\":\"" << json::escape(proto.key()) << "\",\"value\":";

  switch (proto.value_case()) {
    case fuzz::RPCArgument::kVstr:
      lhs << '"' << json::escape(proto.vstr()) << '"';
      break;
    case fuzz::RPCArgument::kVint:
      lhs << proto.vint();
      break;
    case fuzz::RPCArgument::kVbool:
      lhs << (proto.vbool() ? "true" : "false");
      break;
    case fuzz::RPCArgument::kVfloat:
      lhs << proto.vfloat();
      break;
    case fuzz::RPCArgument::VALUE_NOT_SET:
      lhs << "null";
      break;
  }

  lhs << "}";
  return lhs;
}

std::stringstream& operator<<(std::stringstream &lhs, const fuzz::RPCCall &proto) {
  lhs << "{\"op\":\"" << json::escape(proto.op()) << "\",\"args\":[";
  
  int nargs = proto.args_size();
  for (const fuzz::RPCArgument &arg : proto.args()) {
    lhs << arg;
    if (--nargs > 0)
      lhs << ",";
  }

  lhs << "]}";
  return lhs;
}
