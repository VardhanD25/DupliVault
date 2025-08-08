add_executable(sample_test
    sha256_test.cpp
    hasher_test.cpp 
    chunker_test.cpp 
    storage_repository_test.cpp 
    backup_orchestrator_test.cpp 
    metadata_storage_test.cpp
)


target_link_libraries(sample_test
    PRIVATE
        duplivault_lib
        gtest_main
)

target_include_directories(sample_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/third_party
)

include(GoogleTest)
gtest_discover_tests(sample_test)
