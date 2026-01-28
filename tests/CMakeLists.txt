cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(GTest REQUIRED)

enable_testing()

# FileStorage Tests
add_executable(FileStorageTests
    FileStorageTests.cpp
)

target_link_libraries(FileStorageTests PRIVATE
    GTest::gtest
    GTest::gtest_main
    S3FileStorage
)

target_include_directories(FileStorageTests PRIVATE
    ${CMAKE_SOURCE_DIR}/S3FileStorage
)

# S3MetaDB Tests (requires mock database, skip if not available)
add_executable(S3MetaDBTests
    S3MetaDBTests.cpp
)

target_link_libraries(S3MetaDBTests PRIVATE
    GTest::gtest
    GTest::gtest_main
    S3MetaDB
)

target_include_directories(S3MetaDBTests PRIVATE
    ${CMAKE_SOURCE_DIR}/S3MetaDB
)

# Add tests to CTest
add_test(NAME FileStorageTests COMMAND FileStorageTests)
add_test(NAME S3MetaDBTests COMMAND S3MetaDBTests)
