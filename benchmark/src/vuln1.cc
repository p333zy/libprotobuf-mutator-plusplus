#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace vuln {

/*
{
  "op": "EVAL",
  "args": [
    {"key": "command", "value": "!KILL"}
  ]
}
*/

int Eval(json payload) {
  const json args = payload["args"];
  if (!args.is_array()) {
    std::cerr << "Missing args array\n";
    return 1;
  }

  if (args.size() < 1) {
    std::cerr << "Missing args objects\n";
    return 1;
  }

  const json arg0 = args[0];
  if (!arg0.is_object()) {
    std::cerr << "Invalid args object at index 0\n";
    return 1;
  }

  const json key = arg0["key"];
  if (!key.is_string()) {
    std::cerr << "Invalid arg key\n";
    return 1;
  }

  const json val = arg0["value"];
  if (!val.is_string()) {
    std::cerr << "Invalid arg value\n";
    return 1;
  }

  const auto keystr = key.get<std::string>();
  const auto valstr = val.get<std::string>();

  if (keystr != "command") {
    std::cerr << "Missing command argument\n";
    return 1;
  }

  if (valstr == "!HELLO") {
    std::cout << "Hello world\n";
    return 0;
  }

  if (valstr == "!GOODBYE") {
    std::cout << "Goodbye\n";
    return 0;
  }

  if (valstr == "!KILL") {
    std::cerr << "Found the bug\n";
    std::abort();
  }

  return 0;
}

/*
{
  "op": "LOGIN",
  "args": [
    {"key": "username", "value": "admin"},
    {"key": "password", "value": "admin"}
  ]
}
*/

int Login(json payload) {
  const json args = payload["args"];
  if (!args.is_array()) {
    std::cerr << "Missing args array\n";
    return 1;
  }

  if (args.size() != 2) {
    std::cerr << "Login requires two arguments\n";
    return 1;
  }

  std::string username;
  std::string password;
  for (const json &arg : args.array()) {
    if (!arg.is_object()) {
      std::cerr << "Invalid argument object\n";
      return 1;
    }

    const json key = arg["key"];
    const json val = arg["value"];

    if (!key.is_string()) {
      std::cerr << "Invalid arg key\n";
      return 1;
    }

    if (!val.is_string()) {
      std::cerr << "Invalid arg value, expected string\n";
      return 1;
    }

    if (key == "username") {
      username = val.get<std::string>();
    } else if (key == "password") {
      password = val.get<std::string>();
    } else {
      std::cerr << "Unknown field\n";
      return 1;
    }
  }

  if (username == "admin" && password == "admin") {
    std::cout << "Login successful\n";
    return 0;
  }

  std::cout << "Incorrect credentials\n";
  return 1;
}

int RPCEntry(const std::string data) {
  json payload;

  try {
     payload = json::parse(data);
  } catch (...) {
    std::cerr << "Error: invalid JSON\n";
    return 1;
  }

  if (!payload.is_object()) {
    std::cerr << "Invalid payload\n";
    return 1;
  }

  const json op = payload["op"];

  if (!op.is_string()) {
    std::cerr << "Error: op must be string\n";
    return 1;
  }

  const auto opstr = op.get<std::string>();

  if (opstr == "LOGIN") {
    return Login(payload);
  }

  if (opstr == "EVAL") {
    return Eval(payload);
  }

  return 1;
}

};

