// include/duplivault/BackupOrchestrator.h
#pragma once

#include <filesystem>

// Forward declare the classes we depend on to avoid including their full headers.
// This is a good practice that can speed up compilation times.
namespace dv {
    class Chunker;
    class Hasher;
    class StorageRepository;
}

namespace dv {

class BackupOrchestrator {
public:
    /**
     * @brief Constructs the orchestrator with its required components.
     * @param chunker The chunker to use for splitting files.
     * @param hasher The hasher to use for fingerprinting chunks.
     * @param repo The repository where data will be stored.
     */
    BackupOrchestrator(const Chunker& chunker, const Hasher& hasher, StorageRepository& repo);

    /**
     * @brief Runs the backup process for a given source path.
     * @param source_path The file or directory to back up.
     */
    void run_backup(const std::filesystem::path& source_path);

private:
    // References to the components we will use. We don't own them.
    const Chunker& chunker_;
    const Hasher& hasher_;
    StorageRepository& repo_;
};

} // namespace dv