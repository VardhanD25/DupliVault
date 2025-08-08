// tests/storage_repository_test.cpp
#include <gtest/gtest.h>
#include <duplivault/StorageRepository.h>
#include <fstream>

// This test fixture handles setup and teardown of a temporary repository for each test.
class StorageRepositoryTest : public ::testing::Test {
protected:
    // This runs before each test
    void SetUp() override {
        // Create a unique temporary path for our test repository
        // e.g., /tmp/DupliVaultTest_16541651 or C:\Users\...\AppData\Local\Temp\...
        test_repo_path = std::filesystem::temp_directory_path() / "DupliVaultTest" / std::to_string(std::time(nullptr));
        std::filesystem::create_directories(test_repo_path);
        
        // The object we are testing
        repo = std::make_unique<dv::StorageRepository>(test_repo_path);
    }

    // This runs after each test
    void TearDown() override {
        // Clean up the temporary directory
        std::filesystem::remove_all(test_repo_path);
    }

    std::filesystem::path test_repo_path;
    std::unique_ptr<dv::StorageRepository> repo;
};

TEST_F(StorageRepositoryTest, InitCreatesDirectories) {
    repo->init();
    EXPECT_TRUE(std::filesystem::exists(test_repo_path / "objects"));
    EXPECT_TRUE(std::filesystem::exists(test_repo_path / "metadata"));
}

TEST_F(StorageRepositoryTest, StoreAndCheckExists) {
    repo->init();
    const std::string hash = "0a1b2c3d";
    const dv::Chunk chunk_data = {std::byte('h'), std::byte('i')};

    EXPECT_FALSE(repo->chunk_exists(hash));
    repo->store_chunk(hash, chunk_data);
    EXPECT_TRUE(repo->chunk_exists(hash));
}

TEST_F(StorageRepositoryTest, StoreAndRetrieve) {
    repo->init();
    const std::string hash = "0a1b2c3d";
    const dv::Chunk original_data = {std::byte('h'), std::byte('e'), std::byte('l'), std::byte('l'), std::byte('o')};

    repo->store_chunk(hash, original_data);
    dv::Chunk retrieved_data = repo->retrieve_chunk(hash);

    EXPECT_EQ(original_data, retrieved_data);
}

TEST_F(StorageRepositoryTest, RetrieveNonExistentThrows) {
    repo->init();
    const std::string hash = "nonexistenthash";
    EXPECT_THROW(repo->retrieve_chunk(hash), std::runtime_error);
}