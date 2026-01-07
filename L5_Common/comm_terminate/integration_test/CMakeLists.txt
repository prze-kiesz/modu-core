# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############
# Integration tests for comm_terminate module
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
    test_sighup.cpp
    # Include module source directly to avoid linking issues with OBJECT library
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/comm_terminate.cpp
)

###############
# Create integration test executable
#
add_executable(${INTEGRATION_TEST_TARGET}
    ${INTEGRATION_TEST_SOURCES}
)

###############
# Link with module and dependencies
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
)

###############
# Register integration test with CTest
# Note: Integration tests typically run longer and test real signal handling
#
add_test(
    NAME ${INTEGRATION_TEST_TARGET}
    COMMAND bash -c "${CMAKE_CURRENT_BINARY_DIR}/${INTEGRATION_TEST_TARGET} > /tmp/test_sighup.log 2>&1 & PID=$$!; sleep 2; kill -SIGHUP $$PID; sleep 2; kill -SIGTERM $$PID; wait $$PID"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(${INTEGRATION_TEST_TARGET} PROPERTIES TIMEOUT 15)

message(STATUS "Configured integration tests for ${MODULE_NAME}")
