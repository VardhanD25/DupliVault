// tests/backup_orchestrator_test.cpp
#include <gtest/gtest.h>
#include <duplivault/BackupOrchestrator.h>
#include <duplivault/Chunker.h>
#include <duplivault/Hasher.h>
#include <duplivault/StorageRepository.h>
#include <fstream>

// This fixture sets up a complete environment for an integration test.
class BackupOrchestratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 1. Create a temporary directory for our test world
        test_world_path = std::filesystem::temp_directory_path() / "DupliVaultOrchestratorTest" / std::to_string(std::time(nullptr));
        
        // 2. Create subdirectories for the source data and the repository
        source_dir = test_world_path / "source";
        repo_dir = test_world_path / "repo";
        std::filesystem::create_directories(source_dir);
        std::filesystem::create_directories(repo_dir);

        // 3. Create a sample file to be backed up
        std::ofstream sample_file(source_dir / "file1.txt");
        sample_file << "This is a test file for our backup system.";
        sample_file.close();

        // 4. Instantiate all the real components
        repo = std::make_unique<dv::StorageRepository>(repo_dir);
        repo->init(); // Initialize the repository structure

        hasher = std::make_unique<dv::Hasher>();
        chunker = std::make_unique<dv::Chunker>();

        // 5. Instantiate the orchestrator with the components
        orchestrator = std::make_unique<dv::BackupOrchestrator>(*chunker, *hasher, *repo);
    }

    void TearDown() override {
        // Clean up everything
        std::filesystem::remove_all(test_world_path);
    }

    std::filesystem::path test_world_path;
    std::filesystem::path source_dir;
    std::filesystem::path repo_dir;

    std::unique_ptr<dv::StorageRepository> repo;
    std::unique_ptr<dv::Hasher> hasher;
    std::unique_ptr<dv::Chunker> chunker;
    std::unique_ptr<dv::BackupOrchestrator> orchestrator;
};

TEST_F(BackupOrchestratorTest, BackupCreatesChunksInRepository) {
    // Run the backup on our source directory
    orchestrator->run_backup(source_dir);

    // Verification:
    // 1. Create a temporary hasher to find out what the hash SHOULD be.
    std::ifstream file_to_check(source_dir / "file1.txt", std::ios::binary);
    auto chunks = chunker->chunk(file_to_check);
    ASSERT_FALSE(chunks.empty());
    std::string expected_hash = hasher->compute(chunks[0]);

    // 2. Check if the chunk with that hash now exists in the repository.
    EXPECT_TRUE(repo->chunk_exists(expected_hash));
}

TEST_F(BackupOrchestratorTest, SecondBackupOfSameDataIsRedundant) {
    // --- First Backup ---
    orchestrator->run_backup(source_dir);
    
    // Count how many objects are in the repository
    size_t count_after_first_backup = 0;
    if (std::filesystem::exists(repo_dir / "objects")) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(repo_dir / "objects")) {
            if (entry.is_regular_file()) {
                count_after_first_backup++;
            }
        }
    }
    EXPECT_GT(count_after_first_backup, 0);

    // --- Second Backup ---
    // Run the backup again on the exact same data
    orchestrator->run_backup(source_dir);

    // Verification:
    // The number of stored objects should not have changed.
    size_t count_after_second_backup = 0;
    if (std::filesystem::exists(repo_dir / "objects")) {
         for (const auto& entry : std::filesystem::recursive_directory_iterator(repo_dir / "objects")) {
            if (entry.is_regular_file()) {
                count_after_second_backup++;
            }
        }
    }
    EXPECT_EQ(count_after_first_backup, count_after_second_backup);
}