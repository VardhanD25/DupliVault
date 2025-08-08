#include <gtest/gtest.h>
#include <duplivault/Hasher.h> // The header for the class we are testing
#include <vector>
#include <string>
#include <cstddef>      // For std::byte
#include <algorithm>    // For std::transform

// A "Test Fixture" provides a class context for a group of related tests.
// It's good practice for keeping tests organized.
class HasherTest : public ::testing::Test {
protected:
    // We can create objects here that will be available to all tests in this fixture.
    dv::Hasher hasher;
};

// Test case 1: Verify the Hasher class works correctly on an empty input.
TEST_F(HasherTest, ComputesCorrectHashForEmptyInput) {
    std::vector<std::byte> empty_data;
    
    // This is the known SHA-256 hash for an empty input.
    // We expect our Hasher class to produce this exact value.
    std::string expected_hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    
    std::string actual_hash = hasher.compute(empty_data);
    
    EXPECT_EQ(actual_hash, expected_hash);
}

// Test case 2: Verify the Hasher class works correctly on a known string.
TEST_F(HasherTest, ComputesCorrectHashForKnownString) {
    const std::string test_string = "hello world";
    
    // Convert the plain string to a vector of std::byte for our function's interface
    std::vector<std::byte> data(test_string.size());
    std::transform(test_string.begin(), test_string.end(), data.begin(), [](char c) {
        return std::byte(c);
    });
    
    // The known SHA-256 hash for "hello world"
    std::string expected_hash = "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9";

    std::string actual_hash = hasher.compute(data);

    EXPECT_EQ(actual_hash, expected_hash);
}