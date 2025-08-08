// src/StorageRepository.cpp
#include <duplivault/StorageRepository.h>
#include <fstream>
#include <stdexcept>
#include <duplivault/Hasher.h>
#include "json.hpp"

namespace dv {

StorageRepository::StorageRepository(std::filesystem::path repo_path) : root_path_(std::move(repo_path)) {}

void StorageRepository::init() {
    std::filesystem::create_directories(root_path_ / "objects");
    std::filesystem::create_directories(root_path_ / "metadata");
}

std::filesystem::path StorageRepository::path_for_chunk(const std::string& hash) const {
    // Use the first 2 characters of the hash as a subdirectory
    // to prevent having too many files in one folder.
    // e.g., hash "0a1b2c..." -> <repo>/objects/0a/0a1b2c...
    if (hash.length() < 2) {
        throw std::invalid_argument("Hash is too short.");
    }
    return root_path_ / "objects" / hash.substr(0, 2) / hash;
}

bool StorageRepository::chunk_exists(const std::string& hash) const {
    return std::filesystem::exists(path_for_chunk(hash));
}

void StorageRepository::store_chunk(const std::string& hash, const Chunk& chunk_data) {
    const auto final_path = path_for_chunk(hash);
    const auto parent_dir = final_path.parent_path();

    // Create the subdirectory (e.g., '.../objects/0a/') if it doesn't exist
    if (!std::filesystem::exists(parent_dir)) {
        std::filesystem::create_directories(parent_dir);
    }

    // Write the chunk data to the file
    std::ofstream out_file(final_path, std::ios::binary | std::ios::trunc);
    if (!out_file) {
        throw std::runtime_error("Failed to open file for writing: " + final_path.string());
    }
    out_file.write(reinterpret_cast<const char*>(chunk_data.data()), chunk_data.size());
}

Chunk StorageRepository::retrieve_chunk(const std::string& hash) const {
    const auto final_path = path_for_chunk(hash);
    if (!std::filesystem::exists(final_path)) {
        throw std::runtime_error("Chunk does not exist: " + hash);
    }

    std::ifstream in_file(final_path, std::ios::binary | std::ios::ate);
    if (!in_file) {
        throw std::runtime_error("Failed to open file for reading: " + final_path.string());
    }

    std::streamsize size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    Chunk chunk_data(size);
    if (!in_file.read(reinterpret_cast<char*>(chunk_data.data()), size)) {
        throw std::runtime_error("Failed to read chunk data: " + hash);
    }
    
    return chunk_data;
}
// This helper creates a unique, safe filename for a metadata file
// by hashing the original file's canonical path.
std::filesystem::path StorageRepository::path_for_metadata(const std::filesystem::path& original_path) {
    dv::Hasher hasher; // We can create a hasher on the fly
    std::string path_str = std::filesystem::weakly_canonical(original_path).string();
    std::vector<std::byte> path_bytes(path_str.size());
    std::transform(path_str.begin(), path_str.end(), path_bytes.begin(), [](char c){ return std::byte(c); });

    std::string path_hash = hasher.compute(path_bytes);
    return root_path_ / "metadata" / path_hash;
}

void StorageRepository::store_metadata(const std::filesystem::path& original_path, const nlohmann::json& metadata) {
    const auto final_path = path_for_metadata(original_path);
    std::ofstream out_file(final_path);
    out_file << metadata.dump(4); // pretty-print with 4-space indent
}

std::optional<nlohmann::json> StorageRepository::retrieve_metadata(const std::filesystem::path& original_path) {
    const auto final_path = path_for_metadata(original_path);
    if (!std::filesystem::exists(final_path)) {
        return std::nullopt;
    }
    std::ifstream in_file(final_path);
    return nlohmann::json::parse(in_file);
}

} // namespace dv