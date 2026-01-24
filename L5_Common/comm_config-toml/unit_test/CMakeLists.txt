# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############
# Unit tests for comm_config-toml module
###############

set(TEST_TARGET "${MODULE_TARGET}_unittest")

###############
# Test source files
#
set(TEST_SOURCES
    comm_config_toml_test.cpp
    # Include module source directly to avoid linking issues with OBJECT library
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/comm_config_toml.cpp
)

###############
# Create test executable
#
add_executable(${TEST_TARGET}
    ${TEST_SOURCES}
)

###############
# Link with module and dependencies
#
target_link_libraries(${TEST_TARGET}
    PRIVATE
        GTest::gtest_main
        GTest::gmock
        glog::glog
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

