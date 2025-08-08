// src/Chunker.cpp
#include <duplivault/Chunker.h>
#include <array>
#include <cstdint>

namespace dv {

namespace { // Use an anonymous namespace for implementation details

// This helper struct encapsulates the state and logic of the rolling hash.
// It's a private detail of the Chunker's implementation.
struct RollingHash {
    // The size of the sliding window the hash is calculated over.
    static constexpr size_t WINDOW_SIZE = 64;
    
    // A pre-computed table of random values for each possible byte value.
    // Initialized once and reused.
    const std::array<uint32_t, 256> T = []() {
        std::array<uint32_t, 256> table{};
        // A simple Linear Congruential Generator to produce pseudo-random numbers
        uint64_t state = 1;
        for (int i = 0; i < 256; ++i) {
            state = state * 1103515245 + 12345;
            table[i] = static_cast<uint32_t>(state >> 32);
        }
        return table;
    }();

    // The current window of bytes.
    std::array<std::byte, WINDOW_SIZE> window{};
    // The current index within the circular window.
    size_t window_index = 0;
    // The current hash value.
    uint32_t hash = 0;

    // A helper function for circular left shift.
    uint32_t s(uint32_t x) {
        return (x << 1) | (x >> 31);
    }

    // Update the hash with a new byte entering the window.
    void update(std::byte byte_in) {
        // Get the byte that is about to fall out of the window.
        std::byte byte_out = window[window_index];
        // Replace it with the new byte.
        window[window_index] = byte_in;
        // Move the window index forward.
        window_index = (window_index + 1) % WINDOW_SIZE;

        // Update the hash value. This is the core of Buzhash.
        // It's very fast as it only involves shifts and XORs.
        hash = s(hash) ^ T[static_cast<size_t>(byte_out)] ^ T[static_cast<size_t>(byte_in)];
    }
};

} // anonymous namespace

std::vector<Chunk> Chunker::chunk(std::istream& stream) const {
    std::vector<Chunk> all_chunks;
    Chunk current_chunk;
    current_chunk.reserve(AVG_CHUNK_SIZE);

    RollingHash rh; // Create an instance of our rolling hash helper.

    const size_t buffer_size = 4096;
    std::vector<std::byte> buffer(buffer_size);

    while (stream) {
        stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        size_t bytes_read = stream.gcount();
        if (bytes_read == 0) {
            break;
        }

        for (size_t i = 0; i < bytes_read; ++i) {
            const auto current_byte = buffer[i];
            current_chunk.push_back(current_byte);

            // Update the rolling hash with the new byte.
            rh.update(current_byte);

            bool should_cut = false;
            
            // Rule 1: Force a cut if we hit the maximum size.
            if (current_chunk.size() >= MAX_CHUNK_SIZE) {
                should_cut = true;
            } 
            // Rule 2: Only consider cutting if we're past the minimum size.
            else if (current_chunk.size() >= MIN_CHUNK_SIZE) {
                // --- THIS IS THE UPGRADED LOGIC ---
                // We now check the rolling hash value against our pattern.
                if ((rh.hash & CHUNK_PATTERN) == 0) {
                    should_cut = true;
                }
            }

            if (should_cut) {
                all_chunks.push_back(std::move(current_chunk));
                // current_chunk is now empty and ready for the next chunk.
                current_chunk.reserve(AVG_CHUNK_SIZE);
            }
        }
    }

    // After the loop, if there's any data left, add it as the final chunk.
    if (!current_chunk.empty()) {
        all_chunks.push_back(std::move(current_chunk));
    }

    return all_chunks;
}

} // namespace dv
