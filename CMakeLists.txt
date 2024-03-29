cmake_minimum_required(VERSION 3.14)
project(udb)

include_directories("./src")

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# Test Directory
set(TEST_DIR ./test/)
set(TEST_STORAGE_DIR ${TEST_DIR}udb_storage/)

set(CLI_DIR ./udb_cli/)

# Include subdirectories
add_subdirectory(src)

enable_testing()

# Testing executable.
add_executable(
  udb_test
  ${TEST_STORAGE_DIR}test_disk_manager.cpp
  ${TEST_STORAGE_DIR}test_buffer_pool.cpp
  ${TEST_STORAGE_DIR}test_bplus_tree.cpp
  ${TEST_STORAGE_DIR}test_hash_index.cpp
)

add_executable(
  udb_util_test
  ${TEST_DIR}util/test_rwlatch.cpp
)

target_link_libraries(
  udb_test
  gtest_main
  UDB_SRC
)

target_link_libraries(
  udb_util_test
  gtest_main
  UDB_SRC
)

include(GoogleTest)
gtest_discover_tests(udb_test)