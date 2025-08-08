// src/Hasher.cpp

#include <duplivault/Hasher.h>      // The header for our class
#include "sha256.h"                 // Our own SHA-256 implementation's header

namespace dv {

// This is the implementation for the method we declared in Hasher.h
std::string Hasher::compute(const std::vector<std::byte>& data) const {
    // The sha256 function we wrote expects a string_view.
    // We can safely reinterpret the underlying data pointer of the byte vector
    // and create a view of it without making any copies.
    const char* byte_as_char_ptr = reinterpret_cast<const char*>(data.data());
    std::string_view data_view(byte_as_char_ptr, data.size());

    // Delegate the hard work to our globally-tested sha256 function.
    return sha256(data_view);
}

} // namespace dv