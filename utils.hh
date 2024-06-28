#pragma once

#include <string>
#include <concepts>
#include <google/protobuf/message.h>

#define UNUSED(expr) do { (void)(expr); } while (0)

namespace lpmpp {

template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

inline std::string Serialize(const google::protobuf::Message &proto) {
    std::string str;
    proto.SerializeToString(&str);
    return str;
}

std::size_t NextPow2(std::size_t n);

};
