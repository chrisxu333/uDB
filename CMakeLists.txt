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


# Src Directory

set(LIB_DIR ./lib/)
set(SRC_DIR ./src/)
set(STORAGE_DIR ${SRC_DIR}storage/)

# Header Directory
set(INCLUDE_DIR ${SRC_DIR}include/)
set(INCLUDE_STORAGE_DIR ${INCLUDE_DIR}storage/)

# Test Directory
set(TEST_DIR ./test/)
set(TEST_STORAGE_DIR ${TEST_DIR}storage/)

# Include subdirectories
add_subdirectory(src)

enable_testing()

# Testing executable.
add_executable(
  udb_test
  ${TEST_STORAGE_DIR}test_disk_manager.cpp
  ${TEST_STORAGE_DIR}test_buffer_pool.cpp
  ${TEST_STORAGE_DIR}test_b_tree.cpp
)

target_link_libraries(
  udb_test
  gtest_main
  UDB_SRC
)

include(GoogleTest)
gtest_discover_tests(udb_test)