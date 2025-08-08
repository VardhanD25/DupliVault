// src/BackupOrchestrator.cpp
#include <duplivault/BackupOrchestrator.h>
#include <duplivault/Chunker.h>
#include <duplivault/Hasher.h>
#include <duplivault/StorageRepository.h>
#include <fstream>
#include <iostream>
#include <optional>
#include "json.hpp"
#include <chrono>

namespace dv {

BackupOrchestrator::BackupOrchestrator(const Chunker& chunker, const Hasher& hasher, StorageRepository& repo)
    : chunker_(chunker), hasher_(hasher), repo_(repo) {}

void BackupOrchestrator::run_backup(const std::filesystem::path& source_path) {
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(source_path)) {
        if (!dir_entry.is_regular_file()) {
            continue;
        }

        const auto& file_path = dir_entry.path();
        auto current_mod_time = std::filesystem::last_write_time(file_path);

        // --- EFFICIENCY CHECK ---
        auto existing_metadata_opt = repo_.retrieve_metadata(file_path);
        if (existing_metadata_opt.has_value()) {
            auto metadata = existing_metadata_opt.value();
            auto stored_mod_time_ns = metadata.value("mod_time_ns", 0);
            
            // Explicitly create the duration and then the time_point to avoid IntelliSense issues
            auto duration_since_epoch = std::filesystem::file_time_type::duration(stored_mod_time_ns);
            auto stored_mod_time = std::filesystem::file_time_type(duration_since_epoch);
            
            if (stored_mod_time == current_mod_time) {
                std::cout << "Skipping unchanged file: " << file_path.string() << std::endl;
                continue;
            }
        }
        
        std::cout << "Processing file: " << file_path.string() << std::endl;
        std::ifstream file_stream(file_path, std::ios::binary);
        if (!file_stream) {
            std::cerr << "Error: Could not open file " << file_path << std::endl;
            continue;
        }

        std::vector<Chunk> chunks = chunker_.chunk(file_stream);
        std::vector<std::string> chunk_hashes;
        chunk_hashes.reserve(chunks.size());

        for (const auto& chunk : chunks) {
            std::string hash = hasher_.compute(chunk);
            chunk_hashes.push_back(hash);

            if (!repo_.chunk_exists(hash)) {
                std::cout << "  Storing new chunk: " << hash << std::endl;
                repo_.store_chunk(hash, chunk);
            } else {
                // --- THIS IS THE FIX ---
                // Add this else block to confirm when deduplication happens.
                std::cout << "  Chunk already exists: " << hash << std::endl;
            }
        }

        // --- METADATA GENERATION ---
        nlohmann::json metadata;
        metadata["original_path"] = file_path.string();
        metadata["mod_time_ns"] = current_mod_time.time_since_epoch().count();
        metadata["chunk_hashes"] = chunk_hashes;

        repo_.store_metadata(file_path, metadata);
        std::cout << "  Saved metadata for " << file_path.filename() << std::endl;
    }
}
void BackupOrchestrator::run_restore(const std::filesystem::path& destination_dir, 
                                     const std::optional<std::filesystem::path>& original_path_opt) {
    
    std::vector<nlohmann::json> metadatas_to_restore;

    if (original_path_opt.has_value()) {
        // --- Case 1: Restore a single, specific file ---
        std::cout << "Attempting to restore single file: " << original_path_opt.value() << std::endl;
        auto metadata_opt = repo_.retrieve_metadata(original_path_opt.value());
        if (metadata_opt) {
            metadatas_to_restore.push_back(metadata_opt.value());
        }
    } else {
        // --- Case 2: Restore all files in the repository ---
        std::cout << "Attempting to restore all files from repository..." << std::endl;
        metadatas_to_restore = repo_.list_all_metadata();
    }

    if (metadatas_to_restore.empty()) {
        std::cout << "No files found to restore." << std::endl;
        return;
    }

    // Ensure the destination directory exists
    std::filesystem::create_directories(destination_dir);

    for (const auto& metadata : metadatas_to_restore) {
        std::filesystem::path original_path = metadata.value("original_path", "");
        if (original_path.empty()) continue;

        // The final destination for the file is the target dir + the original filename
        std::filesystem::path final_destination = destination_dir / original_path.filename();
        
        std::cout << "Restoring '" << original_path.string() << "' to '" << final_destination.string() << "'" << std::endl;
        
        auto chunk_hashes = metadata["chunk_hashes"].get<std::vector<std::string>>();
        
        std::ofstream out_file(final_destination, std::ios::binary | std::ios::trunc);
        if (!out_file) {
            std::cerr << "  Error: Could not open destination file for writing: " << final_destination << std::endl;
            continue; // Skip to next file
        }

        bool success = true;
        for (const auto& hash : chunk_hashes) {
            try {
                Chunk chunk_data = repo_.retrieve_chunk(hash);
                out_file.write(reinterpret_cast<const char*>(chunk_data.data()), chunk_data.size());
            } catch (const std::exception& e) {
                std::cerr << "  Fatal error restoring " << original_path.filename() << ": Could not retrieve chunk " << hash << ". Restore for this file aborted." << std::endl;
                success = false;
                break; // Stop processing chunks for this file
            }
        }

        out_file.close();
        if (!success) {
            // Clean up partially written file on error
            std::filesystem::remove(final_destination);
        }
    }
    std::cout << "Restore process complete." << std::endl;
}

} // namespace dv
