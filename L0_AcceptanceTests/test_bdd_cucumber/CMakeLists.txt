# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

###############
# Module configuration
#
set(MODULE_NAME "test_bdd_cucumber")
set(MODULE_TARGET "${PROJECT_NAME}-${MODULE_NAME}")

message(STATUS "Configuring BDD acceptance tests: ${MODULE_NAME}")

###############
# Enable test hooks in all linked libraries
#
add_compile_definitions(ENABLE_TEST_HOOKS)

###############
# Find dependencies
#
find_package(GTest REQUIRED)
find_package(toml11 REQUIRED)
find_package(glog REQUIRED)

# Check if cucumber-cpp is installed
find_library(CUCUMBER_CPP_LIBRARY NAMES cucumber-cpp PATHS /usr/local/lib)
find_path(CUCUMBER_CPP_INCLUDE_DIR cucumber-cpp/autodetect.hpp PATHS /usr/local/include)

if(NOT CUCUMBER_CPP_LIBRARY OR NOT CUCUMBER_CPP_INCLUDE_DIR)
  message(WARNING "cucumber-cpp not found. Acceptance tests will not be built.")
  message(STATUS "  CUCUMBER_CPP_LIBRARY: ${CUCUMBER_CPP_LIBRARY}")
  message(STATUS "  CUCUMBER_CPP_INCLUDE_DIR: ${CUCUMBER_CPP_INCLUDE_DIR}")
  return()
endif()

message(STATUS "Found cucumber-cpp:")
message(STATUS "  Library: ${CUCUMBER_CPP_LIBRARY}")
message(STATUS "  Include: ${CUCUMBER_CPP_INCLUDE_DIR}")

###############
# Find application modules
#
find_package("comm_main" REQUIRED)
find_package("comm_terminate" REQUIRED)
find_package("comm_config-toml" REQUIRED)
find_package("infr_main" REQUIRED)

###############
# Source files
#
set(SUPPORT_SOURCES
    src/test_hooks.cpp
    src/test_fixture.cpp
    src/cucumber_hooks.cpp
)

set(STEP_DEFINITIONS
    step_definitions/config_steps.cpp
    step_definitions/terminate_steps.cpp
    step_definitions/startup_steps.cpp
)

set(MODULE_HEADERS
    interface/test_hooks.h
    interface/test_fixture.h
)

###############
# Create test executable
#
add_executable(${MODULE_TARGET}
    ${SUPPORT_SOURCES}
    ${MODULE_HEADERS}
)

# Add step definitions with unique prefixes for each file
target_sources(${MODULE_TARGET} PRIVATE step_definitions/config_steps.cpp)
set_source_files_properties(step_definitions/config_steps.cpp 
    PROPERTIES COMPILE_DEFINITIONS "CUKE_OBJECT_PREFIX=CukeConfigSteps")

target_sources(${MODULE_TARGET} PRIVATE step_definitions/terminate_steps.cpp)
set_source_files_properties(step_definitions/terminate_steps.cpp 
    PROPERTIES COMPILE_DEFINITIONS "CUKE_OBJECT_PREFIX=CukeTerminateSteps")

target_sources(${MODULE_TARGET} PRIVATE step_definitions/startup_steps.cpp)
set_source_files_properties(step_definitions/startup_steps.cpp 
    PROPERTIES COMPILE_DEFINITIONS "CUKE_OBJECT_PREFIX=CukeStartupSteps")

###############
# Include directories
#
target_include_directories(${MODULE_TARGET} PRIVATE
    ${CUCUMBER_CPP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/interface
    ${CMAKE_SOURCE_DIR}/L4_Infrastructure/infr_main/interface
    ${CMAKE_SOURCE_DIR}/L4_Infrastructure/infr_main/src
    ${CMAKE_SOURCE_DIR}/L5_Common/comm_config-toml/interface
    ${CMAKE_SOURCE_DIR}/L5_Common/comm_terminate/interface
    ${CMAKE_SOURCE_DIR}/L5_Common/comm_main/interface
)

###############
# Link libraries
#
target_link_libraries(${MODULE_TARGET} PRIVATE
    ${CUCUMBER_CPP_LIBRARY}
    GTest::gtest
    GTest::gmock
    glog::glog
    toml11::toml11
    systemd
    ${PROJECT_NAME}-comm_main
    ${PROJECT_NAME}-comm_terminate
    ${PROJECT_NAME}-comm_config-toml
    ${PROJECT_NAME}-infr_main
)

###############
# Set C++ standard
#
target_compile_features(${MODULE_TARGET} PRIVATE cxx_std_20)

###############
# Copy feature files and wire config to build directory
#
file(GLOB_RECURSE FEATURE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/features/*.feature")
foreach(FEATURE_FILE ${FEATURE_FILES})
    file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${FEATURE_FILE})
    configure_file(${FEATURE_FILE} 
                   ${CMAKE_CURRENT_BINARY_DIR}/${REL_PATH} 
                   COPYONLY)
endforeach()

# Copy wire configuration
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/features/support/cucumber.wire
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/features/support/)

###############
# Create test config directory
#
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_configs)

###############
# Copy test runner script
#
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh
    ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh
    COPYONLY
)

###############
# Add test target
# Runs wireserver and cucumber tests via shell script
#
add_custom_target(run_acceptance_tests
    COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh
    DEPENDS ${MODULE_TARGET}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running BDD acceptance tests with Cucumber-cpp wireserver"
)

message(STATUS "Acceptance tests configured successfully")
message(STATUS "  Build: cmake --build . --target ${MODULE_TARGET}")
message(STATUS "  Run:   cmake --build . --target run_acceptance_tests")
