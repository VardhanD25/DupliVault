// tests/sha256_test.cpp
#include <gtest/gtest.h>
#include "sha256.h" // Our own new header

TEST(SHA256, ComputesCorrectHashForEmptyInput) {
    std::string expected_hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    EXPECT_EQ(sha256(""), expected_hash);
}

TEST(SHA256, ComputesCorrectHashForKnownString) {
    std::string expected_hash = "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9";
    EXPECT_EQ(sha256("hello world"), expected_hash);
}