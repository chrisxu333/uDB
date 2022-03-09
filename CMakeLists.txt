cmake_minimum_required(VERSION 3.14)
project(udb)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# Directory

set(LIB_DIR ./lib/)
set(SRC_DIR ./src/)
set(INCLUDE_DIR ${SRC_DIR}include/)
set(TEST_DIR ./test/)

enable_testing()

# Testing executable.
add_executable(
  udb_test
  ${TEST_DIR}test_disk_manager.cpp
  ${SRC_DIR}disk_manager.cpp
)
# Main executable
add_executable(
  disk_manager
  ${SRC_DIR}disk_manager.cpp
  ${INCLUDE_DIR}disk_manager.h
)

target_link_libraries(
  udb_test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(udb_test)