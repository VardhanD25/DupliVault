// src/Chunker.cpp
#include <duplivault/Chunker.h>

namespace dv {

std::vector<Chunk> Chunker::chunk(std::istream& stream) const {
    std::vector<Chunk> all_chunks;
    Chunk current_chunk;
    current_chunk.reserve(AVG_CHUNK_SIZE); // Pre-allocate memory for efficiency

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

            bool should_cut = false;
            
            // Rule 1: Force a cut if we hit the maximum size.
            if (current_chunk.size() >= MAX_CHUNK_SIZE) {
                should_cut = true;
            } 
            // Rule 2: Only consider cutting if we're past the minimum size.
            else if (current_chunk.size() >= MIN_CHUNK_SIZE) {
                // This is a simplified stand-in for a rolling hash.
                // We just check if the current byte's value matches our pattern.
                if ((static_cast<uint32_t>(current_byte) & CHUNK_PATTERN) == 0) {
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

    // After the loop, if there's any data left in the buffer,
    // add it as the final chunk.
    if (!current_chunk.empty()) {
        all_chunks.push_back(std::move(current_chunk));
    }

    return all_chunks;
}

} // namespace dv