// include/duplivault/StorageRepository.h
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <cstddef> // For std::byte
#include <stdexcept>
#include <optional> // <-- Added for std::optional

// Keep this include for the 'Chunk' type definition
#include "Chunker.h"

// JSON support (nlohmann/json)
#include "json.hpp"
using json = nlohmann::json;

namespace dv {

class StorageRepository {
public:
    /**
     * @brief Constructs a repository manager for a given path.
     * @param repo_path The root path of the DupliVault repository.
     */
    explicit StorageRepository(std::filesystem::path repo_path);

    /**
     * @brief Initializes the repository by creating the necessary directory structure.
     */
    void init();

    /**
     * @brief Checks if a chunk with the given hash already exists in storage.
     * @param hash The hex-encoded SHA-256 hash of the chunk.
     * @return True if the chunk exists, false otherwise.
     */
    bool chunk_exists(const std::string& hash) const;

    /**
     * @brief Stores a chunk's data in the repository.
     * @param hash The hex-encoded SHA-256 hash of the chunk.
     * @param chunk_data The binary data of the chunk to store.
     */
    void store_chunk(const std::string& hash, const Chunk& chunk_data);

    /**
     * @brief Retrieves a chunk's data from the repository.
     * @param hash The hex-encoded SHA-256 hash of the chunk to retrieve.
     * @return A Chunk containing the binary data.
     * @throws std::runtime_error if the chunk does not exist.
     */
    Chunk retrieve_chunk(const std::string& hash) const;

    // --- Per-file metadata API (matches .cpp) ---

    /**
     * @brief Stores metadata for a specific file in the repository.
     * @param original_path The original file path.
     * @param metadata A JSON object containing metadata.
     */
    void store_metadata(const std::filesystem::path& original_path, const json& metadata);

    /**
     * @brief Retrieves metadata for a specific file from the repository.
     * @param original_path The original file path.
     * @return An optional JSON object with the metadata, or std::nullopt if not found.
     */
    std::optional<json> retrieve_metadata(const std::filesystem::path& original_path);

    std::vector<nlohmann::json> list_all_metadata();

private:
    /**
     * @brief Helper function to determine the full path for a given chunk hash.
     * @param hash The hex-encoded SHA-256 hash.
     * @return The complete filesystem path for the chunk file.
     */
    std::filesystem::path path_for_chunk(const std::string& hash) const;

    /**
     * @brief Gets the full path for a metadata file for a given original file path.
     * @param original_path The original file path.
     * @return The complete filesystem path for the metadata file.
     */
    std::filesystem::path path_for_metadata(const std::filesystem::path& original_path);

    std::filesystem::path root_path_;
};

} // namespace dv