# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############
# Integration tests for comm_config-toml module
###############

set(INTEGRATION_TEST_TARGET "${MODULE_TARGET}_integration_test")

###############
# Find dependencies
#
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET systemd)

###############
# Integration test source files
#
set(INTEGRATION_TEST_SOURCES
    test_reload.cpp
    # Include module source directly to avoid linking issues with OBJECT library
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/comm_config_toml.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../comm_terminate/src/comm_terminate.cpp
)

###############
# Create integration test executable
#
add_executable(${INTEGRATION_TEST_TARGET}
    ${INTEGRATION_TEST_SOURCES}
)

###############
# Link with dependencies
#
target_link_libraries(${INTEGRATION_TEST_TARGET}
    PRIVATE
        glog::glog
        systemd
        Threads::Threads
)

###############
# Include directories for integration tests
#
target_include_directories(${INTEGRATION_TEST_TARGET}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../interface
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../comm_terminate/interface
        ${CMAKE_CURRENT_SOURCE_DIR}/../../comm_terminate/src
)

###############
# Register integration test with CTest
#
add_test(
    NAME ${INTEGRATION_TEST_TARGET}
    COMMAND ${INTEGRATION_TEST_TARGET}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(${INTEGRATION_TEST_TARGET} PROPERTIES TIMEOUT 10)

message(STATUS "Configured integration tests for ${MODULE_NAME}")
