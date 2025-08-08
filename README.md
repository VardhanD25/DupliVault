# DupliVault: A Deduplicating Backup Utility

DupliVault is a robust, command-line backup utility written in modern C++. It is designed to efficiently store data by using content-defined chunking to split files and a content-addressable storage system to ensure that any given piece of data is only ever stored once.

This project was built from the ground up to demonstrate core concepts in software architecture, data structures, algorithms, and system-level C++ programming.

## Core Concepts

DupliVault's efficiency comes from two key principles:

1.  **Content-Defined Chunking (CDC):** Instead of splitting files into fixed-size blocks (e.g., every 4KB), DupliVault uses a **rolling hash** algorithm (Buzhash) to find "natural" boundaries in the file's content. This means that if you insert a single byte at the beginning of a large file, only the first chunk is affected, while all subsequent chunks remain identical. This dramatically improves deduplication efficiency for files that change over time.

2.  **Content-Addressable Storage:** After a file is split into chunks, each chunk is identified by its own unique cryptographic hash (SHA-256). The chunk's data is stored in the repository in a file named after its hash. This means that if two different files happen to contain the exact same chunk of data, that chunk is only stored on disk once.

## Architecture

The project is designed with a clean, modular architecture, separating concerns into distinct, testable components.

* **`Hasher`:** The "Fingerprint Specialist." This component is responsible for computing the SHA-256 hash of a data chunk, providing a unique identifier for it. The SHA-256 algorithm was implemented from scratch based on the FIPS 180-4 standard.

* **`Chunker`:** The "Receiving Department Foreman." This component implements the rolling hash algorithm to split a data stream into variable-sized chunks. It operates based on `MIN_CHUNK_SIZE`, `MAX_CHUNK_SIZE`, and a statistical pattern to determine chunk boundaries.

* **`StorageRepository`:** The "Warehouse Manager." This class is the sole interface to the filesystem. It manages the repository's directory structure, stores and retrieves data chunks by their hash, and handles the storage of metadata "manifest" files.

* **`BackupOrchestrator`:** The "General Manager." This is the brains of the operation. It uses the other three components in sequence to perform `backup` and `restore` operations. It is responsible for the high-level logic of checking file modification times, orchestrating the chunk-hash-store process, and reassembling files during a restore.

* **`main.cpp` (CLI):** The user-facing "Control Panel." It uses the **CLI11** library to provide a professional, subcommand-based command-line interface (`init`, `backup`, `restore`) for the user.

## Build & Setup

The project is built using CMake and requires a C++17 compliant compiler.

### Prerequisites

* A C++17 compiler (e.g., GCC, Clang, MSVC)
* CMake (version 3.14 or newer)
* Git

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone <your-repo-url>
    cd DupliVault
    ```

2.  **Configure CMake:**
    This command will generate the build files in a `build/` directory and automatically fetch dependencies (GoogleTest, CLI11).
    ```bash
    (In root directory)
    mkdir build
    cd build
    cmake -G "MinGW Makefiles" ..
    ```

3.  **Build the project:**
    This will compile the `duplivault` executable and the test runner.
    ```bash
    (In build directory)
    cmake --build .
    ```

4.  **(Optional) Run tests:**
    To verify that all components are working correctly, run the unit and integration tests.
    ```bash
    (In build directory)
    ctest 
    ```

The final executable will be located at `build/duplivault.exe` (on Windows) or `build/duplivault` (on Linux/macOS).

## Usage

DupliVault is operated via the command line with several subcommands.

### Initialize a Repository

You must first create an empty repository.

```bash
./build/duplivault.exe init <path-to-your-repo>

Example: ./build/duplivault.exe init ./my-repo

```

### Back Up Data

This command backs up a source directory into the specified repository. It will automatically skip unchanged files on subsequent runs.

```bash
./build/duplivault.exe backup <path-to-source-data> <path-to-your-repo>

Example: ./build/duplivault.exe backup ./my_documents ./my-repo
```
### Restore Data

You can restore all files from the repository or a single, specific file.

To restore ALL files:

```bash
./build/Debug/duplivault.exe restore -d <path-to-destination-folder> -r <path-to-your-repo>

Example: ./build/duplivault.exe restore -d ./restored_files -r ./my-repo
```
To restore a SINGLE file:

You must provide the original path of the file as it was backed up.

```bash
./build/duplivault.exe restore -p <original-file-path> -d <path-to-destination-folder> -r <path-to-your-repo>

Example: ./build/duplivault.exe restore -p ./my_documents/report.txt -d ./restored_files -r ./my-repo
```

### Future Improvements

This project provides a solid foundation that can be extended with many professional features:

- Multi-Threading: The chunking and hashing of files are CPU-bound tasks that are highly parallelizable. The backup process could be significantly sped up by using a thread pool to process multiple files or chunks concurrently.

- Encryption: Chunks could be encrypted with a user-provided key before being stored in the repository, ensuring the backup is secure from unauthorized access.

- Compression: Data chunks could be compressed (e.g., using zlib or Zstandard) before storage to further reduce the repository's disk footprint.

- Network Support: The StorageRepository could be abstracted to allow for different storage backends, such as an S3 bucket or a remote server accessed over SSH/HTTP, turning this into a true client-server backup tool.

- Snapshot Management: Implement a concept of "snapshots" so the user can list all backups and choose to restore the state of their files from a specific point in time.