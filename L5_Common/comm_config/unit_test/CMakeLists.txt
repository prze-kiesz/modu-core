# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

# Support standalone builds for faster development iteration
if(NOT PROJECT_NAME)
    project(comm_config_unittest CXX)
    
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # Find dependencies
    find_package(GTest 1.16.0 REQUIRED)
    find_package(glog REQUIRED)
    
    # Include FetchContent for toml++
    include(FetchContent)
    FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        v3.4.0
    )
    FetchContent_MakeAvailable(tomlplusplus)
    
    enable_testing()
    
    # Build parent module sources directly
    set(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
    add_library(modu-core-comm_config OBJECT
        ${PARENT_DIR}/src/comm_config.cpp
        ${PARENT_DIR}/interface/comm_config.h
    )
    
    target_include_directories(modu-core-comm_config
        PUBLIC
            ${PARENT_DIR}/interface
        PRIVATE
            ${PARENT_DIR}/src
    )
    
    target_link_libraries(modu-core-comm_config
        PUBLIC
            tomlplusplus::tomlplusplus
            glog::glog
    )
    
    target_compile_features(modu-core-comm_config PUBLIC cxx_std_20)
endif()

###############
# Test configuration
#
set(TEST_NAME "${PROJECT_NAME}-comm_config_unittest")

###############
# Test sources
#
set(TEST_SOURCES
    comm_config_test.cpp
)

###############
# Create test executable
#
add_executable(${TEST_NAME} ${TEST_SOURCES})

# Set C++ standard
target_compile_features(${TEST_NAME} PRIVATE cxx_std_20)

###############
# Link dependencies
#
target_link_libraries(${TEST_NAME} PRIVATE
    modu-core-comm_config
    GTest::gtest
    GTest::gtest_main
    glog::glog
)

###############
# Compiler options
#
target_compile_options(${TEST_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

###############
# Register test with CTest
#
include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
