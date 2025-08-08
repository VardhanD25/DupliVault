#pragma once // Ensures this file is included only once per compilation unit

#include <string>
#include <vector>
#include <cstddef> // Required for std::byte

// We'll place all our project's code inside the 'dv' namespace
namespace dv {

class Hasher {
public:
    /**
     * @brief Computes the SHA-256 hash of a block of binary data.
     * @param data A vector of bytes representing the data to be hashed.
     * @return A string containing the hex-encoded SHA-256 hash.
     */
    std::string compute(const std::vector<std::byte>& data) const;
};

} // namespace dv