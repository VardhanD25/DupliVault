// src/main.cpp
#include <iostream>
#include <string>
#include <filesystem>

// Our new CLI parsing library
#include "CLI11.hpp"

// The components from our library that we will orchestrate
#include <duplivault/StorageRepository.h>
#include <duplivault/Hasher.h>
#include <duplivault/Chunker.h>
#include <duplivault/BackupOrchestrator.h>

int main(int argc, char** argv) {
    // Create the main application object
    CLI::App app{"DupliVault: A deduplicating backup tool"};
    app.require_subcommand(1); // Require at least one subcommand (e.g., 'init' or 'backup')

    // --- Define the 'init' subcommand ---
    std::string init_repo_path;
    CLI::App* init_cmd = app.add_subcommand("init", "Initialize a new DupliVault repository.");
    init_cmd->add_option("repo_path", init_repo_path, "The path to create the repository at.")->required();
    
    // Set the callback function for what to do when 'init' is called
    init_cmd->callback([&]() {
        try {
            dv::StorageRepository repo(init_repo_path);
            repo.init();
            std::cout << "Successfully initialized empty repository at: " << init_repo_path << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error during initialization: " << e.what() << std::endl;
        }
    });

    // --- Define the 'backup' subcommand ---
    std::string backup_source_path;
    std::string backup_repo_path;
    CLI::App* backup_cmd = app.add_subcommand("backup", "Backs up a source directory to a repository.");
    backup_cmd->add_option("source_path", backup_source_path, "The source directory to back up.")->required();
    backup_cmd->add_option("repo_path", backup_repo_path, "The path of the repository.")->required();
    
    // Set the callback function for what to do when 'backup' is called
    backup_cmd->callback([&]() {
        try {
            // Instantiate all the components of our backup engine
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


    // --- Parse the command line ---
    // The library will automatically handle --help flags and errors,
    // and will call the correct callback function based on the subcommand used.
    CLI11_PARSE(app, argc, argv);

    return 0;
}