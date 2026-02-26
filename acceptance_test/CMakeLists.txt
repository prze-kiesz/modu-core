# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.20)

# Only build acceptance tests if explicitly enabled
if(NOT ENABLE_ACCEPTANCE_TESTS)
  message(STATUS "Acceptance tests disabled. Use -DENABLE_ACCEPTANCE_TESTS=ON to enable.")
  return()
endif()

message(STATUS "Configuring acceptance tests with cucumber-cpp")

# Enable test hooks in all linked libraries
add_compile_definitions(ENABLE_TEST_HOOKS)

# Find dependencies
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
  message(STATUS "Install with: sudo apt-get install libcucumber-cpp-dev")
  return()
endif()

message(STATUS "Found cucumber-cpp:")
message(STATUS "  Library: ${CUCUMBER_CPP_LIBRARY}")
message(STATUS "  Include: ${CUCUMBER_CPP_INCLUDE_DIR}")

# Cucumber test executable
add_executable(cucumber_tests
  # Support infrastructure
  support/test_hooks.cpp
  support/test_fixture.cpp
  support/cucumber_hooks.cpp

  # Step definitions
  step_definitions/config_steps.cpp
  step_definitions/terminate_steps.cpp
  step_definitions/startup_steps.cpp
  
  # Include object files from application modules (OBJECT libraries)
  $<TARGET_OBJECTS:modu-core-infr_main>
  $<TARGET_OBJECTS:modu-core-comm_config-toml>
  $<TARGET_OBJECTS:modu-core-comm_terminate>
  $<TARGET_OBJECTS:modu-core-comm_main>
)

# Set unique CUKE_OBJECT_PREFIX for each step definition file to avoid conflicts
set_source_files_properties(step_definitions/config_steps.cpp 
  PROPERTIES COMPILE_DEFINITIONS "CUKE_OBJECT_PREFIX=CukeConfig")
set_source_files_properties(step_definitions/startup_steps.cpp 
  PROPERTIES COMPILE_DEFINITIONS "CUKE_OBJECT_PREFIX=CukeStartup")
set_source_files_properties(step_definitions/terminate_steps.cpp 
  PROPERTIES COMPILE_DEFINITIONS "CUKE_OBJECT_PREFIX=CukeTerminate")
set_source_files_properties(support/cucumber_hooks.cpp 
  PROPERTIES COMPILE_DEFINITIONS "CUKE_OBJECT_PREFIX=CukeHooks")

target_include_directories(cucumber_tests PRIVATE
  ${CUCUMBER_CPP_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/support
  ${CMAKE_SOURCE_DIR}/L4_Infrastructure/infr_main/interface
  ${CMAKE_SOURCE_DIR}/L4_Infrastructure/infr_main/src
  ${CMAKE_SOURCE_DIR}/L5_Common/comm_config-toml/interface
  ${CMAKE_SOURCE_DIR}/L5_Common/comm_terminate/interface
  ${CMAKE_SOURCE_DIR}/L5_Common/comm_main/interface
)

target_link_libraries(cucumber_tests PRIVATE
  ${CUCUMBER_CPP_LIBRARY}
  GTest::gtest
  GTest::gmock
  glog::glog
  toml11::toml11
)

target_compile_features(cucumber_tests PRIVATE cxx_std_17)

# Add test target
add_custom_target(run_acceptance_tests
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/cucumber_tests --format json --out cucumber_results.json
  DEPENDS cucumber_tests
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Running BDD acceptance tests with Cucumber-cpp"
)

# Copy feature files to build directory
file(GLOB_RECURSE FEATURE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/features/*.feature")
foreach(FEATURE_FILE ${FEATURE_FILES})
  file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${FEATURE_FILE})
  configure_file(${FEATURE_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${REL_PATH} COPYONLY)
endforeach()

# Create test config directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_configs)

# Install test runner script
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh
  @ONLY
)

message(STATUS "Acceptance tests configured successfully")
message(STATUS "  - Build with: cmake --build . --target cucumber_tests")
message(STATUS "  - Run with: cmake --build . --target run_acceptance_tests")
message(STATUS "  - Or directly: ./build/acceptance_test/cucumber_tests")
