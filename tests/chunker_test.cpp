// tests/chunker_test.cpp
#include <gtest/gtest.h>
#include <duplivault/Chunker.h>
#include <sstream> // To simulate file streams

class ChunkerTest : public ::testing::Test {
protected:
    dv::Chunker chunker;
};

// Test Case 1: An empty stream should result in zero chunks.
TEST_F(ChunkerTest, HandlesEmptyStream) {
    std::stringstream empty_stream;
    auto chunks = chunker.chunk(empty_stream);
    EXPECT_TRUE(chunks.empty());
}

// Test Case 2: A stream smaller than the minimum chunk size should result in exactly one chunk.
TEST_F(ChunkerTest, SmallStreamIsOneChunk) {
    // 1KB of data, which is less than MIN_CHUNK_SIZE
    std::string test_data_str(1024, 'a');
    std::stringstream small_stream(test_data_str);
    
    auto chunks = chunker.chunk(small_stream);
    
    ASSERT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0].size(), 1024);
}

// Test Case 3: A chunk should be cut when it reaches the maximum size, even if no pattern is found.
TEST_F(ChunkerTest, ForcedCutAtMaxChunkSize) {
    // Create data that is larger than the max size and doesn't contain the magic pattern.
    // We use 'x' (ASCII 120), which won't match our CHUNK_PATTERN.
    std::string test_data_str(dv::Chunker::MAX_CHUNK_SIZE + 100, 'x');
    std::stringstream max_stream(test_data_str);

    auto chunks = chunker.chunk(max_stream);

    ASSERT_EQ(chunks.size(), 2);
    // The first chunk must be exactly MAX_CHUNK_SIZE.
    EXPECT_EQ(chunks[0].size(), dv::Chunker::MAX_CHUNK_SIZE);
    // The second chunk contains the rest.
    EXPECT_EQ(chunks[1].size(), 100);
}

// Test Case 4: A chunk boundary should be created when the magic pattern is found.
TEST_F(ChunkerTest, CreatesBoundaryOnPattern) {
    // Create some initial data that is larger than the minimum size.
    std::vector<char> data_vec(dv::Chunker::MIN_CHUNK_SIZE, 'x');
    
    // Now, insert a character that WILL match our pattern.
    // Our pattern looks for the lowest 13 bits to be 0. The number 0 itself is a perfect match.
    data_vec.push_back(0); // This character will trigger the cut.

    std::string test_data_str(data_vec.begin(), data_vec.end());
    std::stringstream pattern_stream(test_data_str);

    auto chunks = chunker.chunk(pattern_stream);
    
    ASSERT_EQ(chunks.size(), 1);
    // The chunk should contain all the data up to and including the pattern-matching byte.
    EXPECT_EQ(chunks[0].size(), dv::Chunker::MIN_CHUNK_SIZE + 1);
}