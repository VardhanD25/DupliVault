// tests/restore_test.cpp
#include <gtest/gtest.h>
#include <duplivault/BackupOrchestrator.h>
#include <duplivault/Chunker.h>
#include <duplivault/Hasher.h>
#include <duplivault/StorageRepository.h>
#include <fstream>
#include <iterator> // For std::istreambuf_iterator

// This fixture sets up a complete environment, runs a backup,
// and then provides the context for our restore tests.
class RestoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 1. Create a temporary directory for our test world
        test_world_path = std::filesystem::temp_directory_path() / "DupliVaultRestoreTest" / std::to_string(std::time(nullptr));
        
        // 2. Create subdirectories
        source_dir = test_world_path / "source";
        repo_dir = test_world_path / "repo";
        restore_dir = test_world_path / "restore";
        std::filesystem::create_directories(source_dir);
        std::filesystem::create_directories(repo_dir);
        std::filesystem::create_directories(restore_dir);

        // 3. Create multiple sample files with known content
        original_file_path1 = source_dir / "my_novel.txt";
        original_content1 = "This is chapter 1.\nThis is chapter 2.\nThis is the end.";
        std::ofstream sample_file1(original_file_path1);
        sample_file1 << original_content1;
        sample_file1.close();

        original_file_path2 = source_dir / "notes.txt";
        original_content2 = "A quick brown fox.";
        std::ofstream sample_file2(original_file_path2);
        sample_file2 << original_content2;
        sample_file2.close();


        // 4. Instantiate all components
        repo = std::make_unique<dv::StorageRepository>(repo_dir);
        repo->init();
        hasher = std::make_unique<dv::Hasher>();
        chunker = std::make_unique<dv::Chunker>();
        orchestrator = std::make_unique<dv::BackupOrchestrator>(*chunker, *hasher, *repo);

        // 5. CRUCIAL: Run a backup first to populate the repository
        orchestrator->run_backup(source_dir);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_world_path);
    }

    // Helper to read a file's content into a string
    std::string read_file_content(const std::filesystem::path& path) {
        std::ifstream file_stream(path);
        return std::string(
            (std::istreambuf_iterator<char>(file_stream)),
            std::istreambuf_iterator<char>()
        );
    }

    std::filesystem::path test_world_path, source_dir, repo_dir, restore_dir;
    std::filesystem::path original_file_path1, original_file_path2;
    std::string original_content1, original_content2;

    std::unique_ptr<dv::StorageRepository> repo;
    std::unique_ptr<dv::Hasher> hasher;
    std::unique_ptr<dv::Chunker> chunker;
    std::unique_ptr<dv::BackupOrchestrator> orchestrator;
};

// Test Case 1: Verify restoring a single, specific file.
TEST_F(RestoreTest, RestoresSingleFileCorrectly) {
    // --- The Action ---
    // Restore only the first file to the destination directory.
    orchestrator->run_restore(restore_dir, original_file_path1);

    // --- The Verification ---
    const auto restored_file1_path = restore_dir / original_file_path1.filename();
    const auto restored_file2_path = restore_dir / original_file_path2.filename();

    // Check that the first file was created and its content is correct.
    ASSERT_TRUE(std::filesystem::exists(restored_file1_path));
    EXPECT_EQ(read_file_content(restored_file1_path), original_content1);

    // Check that the second file was NOT restored.
    EXPECT_FALSE(std::filesystem::exists(restored_file2_path));
}

// Test Case 2: Verify restoring all files from the repository.
TEST_F(RestoreTest, RestoresAllFilesCorrectly) {
    // --- The Action ---
    // Restore everything by passing an empty optional.
    orchestrator->run_restore(restore_dir, std::nullopt);

    // --- The Verification ---
    const auto restored_file1_path = restore_dir / original_file_path1.filename();
    const auto restored_file2_path = restore_dir / original_file_path2.filename();

    // Check that the first file was created and its content is correct.
    ASSERT_TRUE(std::filesystem::exists(restored_file1_path));
    EXPECT_EQ(read_file_content(restored_file1_path), original_content1);

    // Check that the second file was ALSO created and its content is correct.
    ASSERT_TRUE(std::filesystem::exists(restored_file2_path));
    EXPECT_EQ(read_file_content(restored_file2_path), original_content2);
}
