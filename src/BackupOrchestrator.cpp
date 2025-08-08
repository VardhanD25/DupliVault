// src/BackupOrchestrator.cpp
#include <duplivault/BackupOrchestrator.h>
#include <duplivault/Chunker.h>
#include <duplivault/Hasher.h>
#include <duplivault/StorageRepository.h>
#include <fstream>
#include <iostream>
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

} // namespace dv
