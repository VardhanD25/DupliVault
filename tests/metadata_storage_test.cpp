// tests/metadata_storage_test.cpp
#include <gtest/gtest.h>
#include <duplivault/StorageRepository.h>
#include "json.hpp" // Our new JSON library

class MetadataStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_repo_path = std::filesystem::temp_directory_path() / "DupliVaultMetadataTest" / std::to_string(std::time(nullptr));
        std::filesystem::create_directories(test_repo_path);
        repo = std::make_unique<dv::StorageRepository>(test_repo_path);
        repo->init();
    }

    void TearDown() override {
        std::filesystem::remove_all(test_repo_path);
    }

    std::filesystem::path test_repo_path;
    std::unique_ptr<dv::StorageRepository> repo;
};

TEST_F(MetadataStorageTest, RetrieveNonExistentMetadata) {
    auto metadata = repo->retrieve_metadata("non_existent_file.txt");
    EXPECT_FALSE(metadata.has_value());
}

TEST_F(MetadataStorageTest, StoreAndRetrieveMetadata) {
    const std::filesystem::path original_file = "/documents/report.txt";
    nlohmann::json original_metadata;
    original_metadata["path"] = original_file.string();
    original_metadata["chunk_hashes"] = {"hash1", "hash2", "hash3"};

    // Store the metadata
    repo->store_metadata(original_file, original_metadata);

    // Retrieve it and check for correctness
    auto retrieved_metadata_opt = repo->retrieve_metadata(original_file);
    ASSERT_TRUE(retrieved_metadata_opt.has_value());
    
    nlohmann::json retrieved_metadata = retrieved_metadata_opt.value();
    EXPECT_EQ(original_metadata.dump(), retrieved_metadata.dump());
}