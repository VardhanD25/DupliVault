// src/main.cpp
#include <iostream>
#include <string>
#include <filesystem>
#include <optional>

#include "CLI11.hpp"
#include <duplivault/StorageRepository.h>
#include <duplivault/Hasher.h>
#include <duplivault/Chunker.h>
#include <duplivault/BackupOrchestrator.h>

int main(int argc, char** argv) {
    CLI::App app{"DupliVault: A deduplicating backup tool"};
    app.require_subcommand(1);

    // --- 'init' subcommand (unchanged) ---
    std::string init_repo_path;
    CLI::App* init_cmd = app.add_subcommand("init", "Initialize a new DupliVault repository.");
    init_cmd->add_option("repo_path", init_repo_path, "The path to create the repository at.")->required();
    init_cmd->callback([&]() {
        try {
            dv::StorageRepository repo(init_repo_path);
            repo.init();
            std::cout << "Successfully initialized empty repository at: " << init_repo_path << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error during initialization: " << e.what() << std::endl;
        }
    });

    // --- 'backup' subcommand (unchanged) ---
    std::string backup_source_path;
    std::string backup_repo_path;
    CLI::App* backup_cmd = app.add_subcommand("backup", "Backs up a source directory to a repository.");
    backup_cmd->add_option("source_path", backup_source_path, "The source directory to back up.")->required();
    backup_cmd->add_option("repo_path", backup_repo_path, "The path of the repository.")->required();
    backup_cmd->callback([&]() {
        try {
            dv::StorageRepository repo(backup_repo_path);
            dv::Hasher hasher;
            dv::Chunker chunker;
            dv::BackupOrchestrator orchestrator(chunker, hasher, repo);
            std::cout << "Starting backup..." << std::endl;
            orchestrator.run_backup(backup_source_path);
            std::cout << "Backup complete." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error during backup: " << e.what() << std::endl;
        }
    });

    // --- 'restore' subcommand (CORRECTED) ---
    std::string restore_repo_path;
    std::string restore_destination_dir;
    std::optional<std::string> restore_original_path_opt;
    CLI::App* restore_cmd = app.add_subcommand("restore", "Restores files from a repository.");
    
    // --- THIS IS THE FIX ---
    // We now define proper named options with short (-p) and long (--path) versions.
    restore_cmd->add_option("-p,--path", restore_original_path_opt, "The original path of the specific file to restore. If omitted, all files are restored.");
    restore_cmd->add_option("-d,--dest", restore_destination_dir, "The folder where files will be restored.")->required();
    restore_cmd->add_option("-r,--repo", restore_repo_path, "The path of the repository.")->required();

    restore_cmd->callback([&]() {
        try {
            dv::StorageRepository repo(restore_repo_path);
            dv::Hasher hasher;
            dv::Chunker chunker;
            dv::BackupOrchestrator orchestrator(chunker, hasher, repo);
            
            std::optional<std::filesystem::path> path_opt;
            if (restore_original_path_opt) {
                path_opt = restore_original_path_opt.value();
            }

            orchestrator.run_restore(restore_destination_dir, path_opt);

        } catch (const std::exception& e) {
            std::cerr << "Error during restore: " << e.what() << std::endl;
        }
    });

    CLI11_PARSE(app, argc, argv);
    return 0;
}
