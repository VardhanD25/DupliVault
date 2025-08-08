// third_party/sha256.cpp
#include "sha256.h"
#include <vector>
#include <cstdint> // For uint32_t, uint64_t
#include <sstream> // For stringstream
#include <iomanip> // For setfill, setw
#include <algorithm> // For std::copy

// Helper function for 32-bit right rotation (a common bitwise operation)
constexpr uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

// Helper functions as defined in the FIPS 180-4 standard
constexpr uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

constexpr uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

constexpr uint32_t Sigma0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

constexpr uint32_t Sigma1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

constexpr uint32_t sigma0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

constexpr uint32_t sigma1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

// K constants - fractional parts of the cube roots of the first 64 primes
const std::vector<uint32_t> K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

std::string sha256(std::string_view input) {
    // Initial hash values (H) - fractional parts of the square roots of the first 8 primes
    std::vector<uint32_t> H = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    // --- Pre-processing (Padding) ---
    std::vector<uint8_t> msg(input.begin(), input.end());
    uint64_t original_len_bits = msg.size() * 8;
    
    // Append the '1' bit
    msg.push_back(0x80);

    // Append '0' bits until message length in bits is 448 (mod 512)
    // which means length in bytes is 56 (mod 64)
    while (msg.size() % 64 != 56) {
        msg.push_back(0x00);
    }

    // Append original length in bits as a 64-bit big-endian integer
    for (int i = 7; i >= 0; --i) {
        msg.push_back((original_len_bits >> (i * 8)) & 0xff);
    }
    
    // --- Process the message in successive 64-byte chunks ---
    for (size_t i = 0; i < msg.size(); i += 64) {
        uint32_t W[64];
        const uint8_t* chunk = &msg[i];
        
        // 1. Create message schedule W
        for (int t = 0; t < 16; ++t) {
            W[t] = (uint32_t)(chunk[t * 4]) << 24 |
                   (uint32_t)(chunk[t * 4 + 1]) << 16 |
                   (uint32_t)(chunk[t * 4 + 2]) << 8 |
                   (uint32_t)(chunk[t * 4 + 3]);
        }
        for (int t = 16; t < 64; ++t) {
            W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];
        }

        // 2. Initialize working variables with current hash value
        uint32_t a = H[0], b = H[1], c = H[2], d = H[3],
                 e = H[4], f = H[5], g = H[6], h = H[7];

        // 3. Compression function main loop
        for (int t = 0; t < 64; ++t) {
            uint32_t T1 = h + Sigma1(e) + Ch(e, f, g) + K[t] + W[t];
            uint32_t T2 = Sigma0(a) + Maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        // 4. Compute the intermediate hash value
        H[0] += a; H[1] += b; H[2] += c; H[3] += d;
        H[4] += e; H[5] += f; H[6] += g; H[7] += h;
    }

    // --- Produce the final hash value (big-endian) ---
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint32_t h_val : H) {
        ss << std::setw(8) << h_val;
    }
    return ss.str();
}