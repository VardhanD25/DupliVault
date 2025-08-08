// tests/chunker_test.cpp
#include <gtest/gtest.h>
#include <duplivault/Chunker.h>
#include <duplivault/Hasher.h> // We need the hasher to compare chunks
#include <sstream>
#include <random> // For generating better test data

class ChunkerTest : public ::testing::Test {
protected:
    dv::Chunker chunker;
    dv::Hasher hasher; // We'll use this to compare sets of chunks

    // Helper function to generate a vector of pseudo-random bytes
    std::vector<char> generate_data(size_t size) {
        std::vector<char> data(size);
        std::mt19937 rng(12345); // Use a fixed seed for reproducible tests
        std::uniform_int_distribution<int> dist(0, 255);
        for (size_t i = 0; i < size; ++i) {
            data[i] = static_cast<char>(dist(rng));
        }
        return data;
    }
};

// This test is still valid.
TEST_F(ChunkerTest, HandlesEmptyStream) {
    std::stringstream empty_stream;
    auto chunks = chunker.chunk(empty_stream);
    EXPECT_TRUE(chunks.empty());
}

// This test is still valid.
TEST_F(ChunkerTest, SmallStreamIsOneChunk) {
    std::string test_data_str(1024, 'a');
    std::stringstream small_stream(test_data_str);
    auto chunks = chunker.chunk(small_stream);
    ASSERT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0].size(), 1024);
}

// This test is still valid.
TEST_F(ChunkerTest, ForcedCutAtMaxChunkSize) {
    auto data_vec = generate_data(dv::Chunker::MAX_CHUNK_SIZE + 100);
    std::string test_data_str(data_vec.begin(), data_vec.end());
    std::stringstream max_stream(test_data_str);
    auto chunks = chunker.chunk(max_stream);
    // It might be 2 or more chunks depending on the random data,
    // but the first one MUST be at most MAX_CHUNK_SIZE.
    ASSERT_FALSE(chunks.empty());
    EXPECT_LE(chunks[0].size(), dv::Chunker::MAX_CHUNK_SIZE);
}

// --- NEW, IMPROVED TEST ---
// This test verifies the core benefit of content-defined chunking.
TEST_F(ChunkerTest, InsertionDoesNotChangeSubsequentChunks) {
    // 1. Create two large blocks of data
    auto prefix_data = generate_data(16 * 1024); // 16KB
    auto suffix_data = generate_data(16 * 1024); // 16KB

    // 2. Create "File A" by combining them
    std::string file_a_str;
    file_a_str.append(prefix_data.begin(), prefix_data.end());
    file_a_str.append(suffix_data.begin(), suffix_data.end());
    std::stringstream file_a_stream(file_a_str);

    // 3. Create "File B" with a small insertion in the middle
    std::string file_b_str;
    file_b_str.append(prefix_data.begin(), prefix_data.end());
    file_b_str.append("...SOME NEW DATA INSERTED HERE..."); // The modification
    file_b_str.append(suffix_data.begin(), suffix_data.end());
    std::stringstream file_b_stream(file_b_str);

    // 4. Chunk both simulated files
    auto chunks_a = chunker.chunk(file_a_stream);
    auto chunks_b = chunker.chunk(file_b_stream);

    ASSERT_GT(chunks_a.size(), 1);
    ASSERT_GT(chunks_b.size(), 1);

    // 5. Get the hash of the LAST chunk from each file.
    //    This chunk should correspond to the 'suffix_data' block.
    std::string last_chunk_hash_a = hasher.compute(chunks_a.back());
    std::string last_chunk_hash_b = hasher.compute(chunks_b.back());

    // 6. THE VERIFICATION:
    //    Even though we inserted data in the middle of File B, the chunking
    //    boundary for the final, unchanged block of data should be the same.
    //    Therefore, the last chunks of both files should be identical.
    EXPECT_EQ(last_chunk_hash_a, last_chunk_hash_b);
}
