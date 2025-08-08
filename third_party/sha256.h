// third_party/sha256.h
#pragma once

#include <string>
#include <string_view>

// Declares our globally accessible SHA-256 function.
// We will define its body in sha256.cpp.
std::string sha256(std::string_view input);