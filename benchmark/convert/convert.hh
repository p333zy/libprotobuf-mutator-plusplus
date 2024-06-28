#include <sstream>
#include <string>

#include "proto/vuln.pb.h"

std::stringstream& operator<<(std::stringstream &lhs, const vuln::fuzz::RPCArgument &proto);
std::stringstream& operator<<(std::stringstream &lhs, const vuln::fuzz::RPCCall &proto);