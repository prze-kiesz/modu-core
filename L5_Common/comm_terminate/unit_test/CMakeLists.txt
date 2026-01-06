# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

# Check if this is being built as part of main project or standalone
if(NOT PROJECT_NAME)
    project(comm_terminate_tests VERSION 1.0.0 LANGUAGES CXX)
    
    # Standalone build - set up dependencies
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # Find all required packages for standalone build
    find_package(GTest REQUIRED)
    find_package(glog REQUIRED)
    find_package(Threads REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET libsystemd)
    
    set(MODULE_TARGET "modu-core-comm_terminate")
    set(TEST_TARGET "${MODULE_TARGET}_unittest")
else()
    # Part of main project build
    set(TEST_TARGET "${MODULE_TARGET}_unittest")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET libsystemd)
endif()

###############
# Test source files
#
set(TEST_SOURCES
    comm_terminate_test.cpp
    # Include module source directly to avoid linking issues with OBJECT library
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/comm_terminate.cpp
)

###############
# Create test executable
#
add_executable(${TEST_TARGET}
    ${TEST_SOURCES}
)

# Set C++20 for test target
target_compile_features(${TEST_TARGET} PRIVATE cxx_std_20)

###############
# Link with module and dependencies
#
target_link_libraries(${TEST_TARGET}
    PRIVATE
        GTest::gtest_main
        GTest::gmock
        glog::glog
        PkgConfig::SYSTEMD
        Threads::Threads
)

###############
# Include directories for tests
#
target_include_directories(${TEST_TARGET}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../interface
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

###############
# Register tests with CTest
#
include(GoogleTest)
gtest_discover_tests(${TEST_TARGET}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PROPERTIES
        TIMEOUT 10
)

# Also add as a manual test
add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

message(STATUS "Configured unit tests for ${MODULE_NAME}")
