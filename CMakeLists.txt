cmake_minimum_required(VERSION 3.14)
project(DupliVault LANGUAGES CXX)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library with core logic
add_library(duplivault_lib
    src/Hasher.cpp
    third_party/sha256.cpp 
    src/Chunker.cpp 
    src/StorageRepository.cpp 
    src/BackupOrchestrator.cpp
)
target_include_directories(duplivault_lib
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party
)

# Main executable
add_executable(duplivault src/main.cpp)
target_link_libraries(duplivault PRIVATE duplivault_lib)

# --- Testing ---
enable_testing()
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(tests)
