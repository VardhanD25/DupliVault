// include/duplivault/Chunker.h
#pragma once

#include <vector>
#include <cstddef>
#include <istream>

namespace dv {

// For clarity, we define that a "Chunk" is simply a vector of bytes.
using Chunk = std::vector<std::byte>;

class Chunker {
public:
    // These constants control how the chunking algorithm behaves.
    // They are public so other parts of the system can know about them.
    static constexpr size_t MIN_CHUNK_SIZE = 2 * 1024;   // 2 KB
    static constexpr size_t AVG_CHUNK_SIZE = 8 * 1024;   // 8 KB
    static constexpr size_t MAX_CHUNK_SIZE = 32 * 1024;  // 32 KB
    
    // This is the "magic pattern" we look for in our data's hash to decide
    // where to end a chunk.
    // With a target average of 8KB (which is 2^13), a good pattern is one
    // that statistically appears once every 8192 bytes. We achieve this by
    // checking if the lowest 13 bits of a hash are all zero.
    static constexpr uint32_t CHUNK_PATTERN = (1 << 13) - 1;

    /**
     * @brief Splits a data stream into content-defined chunks.
     * @param stream The input stream to read data from.
     * @return A vector of Chunk objects.
     */
    std::vector<Chunk> chunk(std::istream& stream) const;
};

} // namespace dv