# Module Template Guide

This document provides templates and instructions for creating new modules in the modu-core project.

## Directory Structure

Each module should follow this structure:
```
L5_Common/module_name/
├── module_name-config.cmake      # CMake integration file
├── CMakeLists.txt                # Build configuration
├── interface/
│   └── module_name.h             # Public header
├── src/
│   └── module_name.cpp           # Implementation
└── unit_test/
    ├── CMakeLists.txt            # Test build configuration
    └── module_name_test.cpp      # Unit tests
```

## File Templates

### 1. module_name-config.cmake

**Reference:** `L5_Common/comm_main/comm_main-config.cmake`

```cmake
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############
# module_name-config.cmake
# Configuration file for integrating module_name module with the project
# This file is called by find_package(module_name)
###############

###############
# Define module name
#
set(CURRENT_LIB_NAME "module_name")

# Prevent multiple inclusion
if (TARGET ${PROJECT_NAME}-${CURRENT_LIB_NAME})
    return()
endif()

message(STATUS "Configuring module: ${CURRENT_LIB_NAME}")

###############
# Build the module using its CMakeLists.txt
#
add_subdirectory(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_LIB_NAME})

###############
# Export interface to main project
# Makes module_name headers accessible to other modules
#
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/interface>
    $<INSTALL_INTERFACE:include>
)

###############
# Link module to main project
#
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-${CURRENT_LIB_NAME})
```

**Important:** Replace `module_name` with your actual module name (e.g., `comm_config-toml`)

---

### 2. CMakeLists.txt (Module Root)

**Reference:** `L5_Common/comm_terminate/CMakeLists.txt`

```cmake
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

###############
# Module configuration
#
set(MODULE_NAME "module_name")
set(MODULE_TARGET "${PROJECT_NAME}-${MODULE_NAME}")

message(STATUS "Building module: ${MODULE_NAME}")

###############
# Source files
#
set(MODULE_SOURCES
    src/module_name.cpp
)

set(MODULE_HEADERS
    interface/module_name.h
)

###############
# Create library target
#
add_library(${MODULE_TARGET} OBJECT
    ${MODULE_SOURCES}
    ${MODULE_HEADERS}
)

# Set C++ standard
target_compile_features(${MODULE_TARGET} PUBLIC cxx_std_20)

###############
# Include directories
#
target_include_directories(${MODULE_TARGET}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interface>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

###############
# Dependencies
#
target_link_libraries(${MODULE_TARGET}
    PRIVATE
        # Add dependencies here, e.g.:
        # ${PROJECT_NAME}-other_module
        glog::glog
)

###############
# Unit tests (if testing is enabled)
#
if(BUILD_TESTING)
    add_subdirectory(unit_test)
endif()
```

---

### 3. interface/module_name.h

**Reference:** `L5_Common/comm_terminate/interface/comm_terminate.h`

```cpp
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file module_name.h
 * @brief Brief description of the module
 * @details Detailed description of what this module does
 */

#pragma once

#include <string>
#include <system_error>

namespace comm {

/**
 * @enum ModuleError
 * @brief Error codes for module operations
 */
enum class ModuleError {
  Success = 0,
  InitializationFailed,
  OperationFailed
};

/**
 * @class ModuleErrorCategory
 * @brief Error category for module errors
 */
class ModuleErrorCategory : public std::error_category {
 public:
  const char* name() const noexcept override { return "module_name"; }
  std::string message(int error_value) const override;
};

/**
 * @brief Get the singleton instance of module error category
 * @return Reference to the error category
 */
const std::error_category& get_module_error_category() noexcept;

/**
 * @brief Create an error_code from ModuleError enum
 * @param error Module error value
 * @return std::error_code representing the error
 */
inline std::error_code make_error_code(ModuleError error) {
  return {static_cast<int>(error), get_module_error_category()};
}

/**
 * @class ModuleName
 * @brief Main class for the module (singleton if needed)
 */
class ModuleName {
 public:
  /**
   * @brief Get singleton instance (if singleton pattern is used)
   * @return Reference to the singleton instance
   */
  static ModuleName& Instance();

  // Delete copy/move constructors and assignment operators (for singleton)
  ModuleName(const ModuleName&) = delete;
  ModuleName& operator=(const ModuleName&) = delete;
  ModuleName(ModuleName&&) = delete;
  ModuleName& operator=(ModuleName&&) = delete;

  /**
   * @brief Initialize the module
   * @return Error code indicating success or failure
   */
  std::error_code Initialize();

  // Add your public methods here

 private:
  ModuleName();
  ~ModuleName() = default;

  // Add private member variables here
  bool m_initialized{false};
};

}  // namespace comm

// Enable std::error_code support for ModuleError
namespace std {
template <>
struct is_error_code_enum<comm::ModuleError> : true_type {};
}  // namespace std
```

---

### 4. src/module_name.cpp

**Reference:** `L5_Common/comm_terminate/src/comm_terminate.cpp`

```cpp
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file module_name.cpp
 * @brief Implementation of module_name
 */

#include "module_name.h"

#include <glog/logging.h>

namespace comm {

// Error category implementation
std::string ModuleErrorCategory::message(int error_value) const {
  switch (static_cast<ModuleError>(error_value)) {
    case ModuleError::Success:
      return "Success";
    case ModuleError::InitializationFailed:
      return "Module initialization failed";
    case ModuleError::OperationFailed:
      return "Operation failed";
    default:
      return "Unknown module error";
  }
}

const std::error_category& get_module_error_category() noexcept {
  static ModuleErrorCategory instance;
  return instance;
}

ModuleName& ModuleName::Instance() {
  static ModuleName instance;
  return instance;
}

ModuleName::ModuleName() {
  LOG(INFO) << "ModuleName instance created";
}

std::error_code ModuleName::Initialize() {
  LOG(INFO) << "ModuleName::Initialize() called";
  
  // TODO: Implement initialization logic
  
  m_initialized = true;
  return make_error_code(ModuleError::Success);
}

}  // namespace comm
```

---

### 5. unit_test/CMakeLists.txt

**Reference:** `L5_Common/comm_terminate/unit_test/CMakeLists.txt`

```cmake
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

cmake_minimum_required(VERSION 3.22)

###############
# Unit tests configuration
#
message(STATUS "Configured unit tests for ${MODULE_NAME}")

###############
# Test executable
#
add_executable(${MODULE_TARGET}_unittest
    module_name_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/module_name.cpp
)

###############
# Include directories
#
target_include_directories(${MODULE_TARGET}_unittest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../interface
)

###############
# Link test libraries
#
target_link_libraries(${MODULE_TARGET}_unittest
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        glog::glog
        # Add other dependencies here
)

###############
# Set C++ standard
#
set_target_properties(${MODULE_TARGET}_unittest PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

###############
# Register tests with CTest
#
include(GoogleTest)
gtest_discover_tests(${MODULE_TARGET}_unittest)
```

---

### 6. unit_test/module_name_test.cpp

**Reference:** `L5_Common/comm_terminate/unit_test/comm_terminate_test.cpp`

```cpp
// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file module_name_test.cpp
 * @brief Unit tests for module_name
 */

#include "module_name.h"

#include <gtest/gtest.h>

namespace comm {

class ModuleNameTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Setup code before each test
  }

  void TearDown() override {
    // Cleanup code after each test
  }
};

TEST_F(ModuleNameTest, InstanceReturnsSingleton) {
  ModuleName& instance1 = ModuleName::Instance();
  ModuleName& instance2 = ModuleName::Instance();
  
  EXPECT_EQ(&instance1, &instance2);
}

TEST_F(ModuleNameTest, InitializeReturnsSuccess) {
  ModuleName& module = ModuleName::Instance();
  std::error_code ec = module.Initialize();
  
  EXPECT_FALSE(ec);
}

TEST_F(ModuleNameTest, MakeErrorCodeReturnsValidErrorCode) {
  std::error_code ec = make_error_code(ModuleError::InitializationFailed);
  
  EXPECT_TRUE(ec);
  EXPECT_EQ(ec.value(), static_cast<int>(ModuleError::InitializationFailed));
  EXPECT_STREQ(ec.category().name(), "module_name");
}

TEST_F(ModuleNameTest, ErrorCategoryReturnsCorrectMessages) {
  const std::error_category& category = get_module_error_category();
  
  EXPECT_STREQ(category.message(static_cast<int>(ModuleError::Success)).c_str(), 
               "Success");
  EXPECT_STREQ(category.message(static_cast<int>(ModuleError::InitializationFailed)).c_str(), 
               "Module initialization failed");
}

}  // namespace comm
```

---

## Integration Steps

### 1. Add module to main/CMakeLists.txt

Add your module to the dependencies:
```cmake
# L5 layer modules
find_package("comm_main" REQUIRED)
find_package("comm_terminate" REQUIRED)
find_package("module_name" REQUIRED)  # Add this line
```

### 2. Link module in dependent modules

If another module needs your module, update its CMakeLists.txt:
```cmake
target_link_libraries(${MODULE_TARGET}
    PRIVATE
        ${PROJECT_NAME}-module_name  # Add this line
        glog::glog
)
```

### 3. Include header in code

```cpp
#include "module_name.h"

// Use the module
auto& module = comm::ModuleName::Instance();
auto result = module.Initialize();
if (result) {
    LOG(ERROR) << "Failed: " << result.message();
}
```

---

## Checklist for New Module

- [ ] Create directory structure under appropriate layer (L1-L5)
- [ ] Copy and adapt `module_name-config.cmake` from template
- [ ] Copy and adapt `CMakeLists.txt` from template
- [ ] Create header file in `interface/` directory
- [ ] Create implementation file in `src/` directory
- [ ] Create unit test CMakeLists.txt
- [ ] Create unit test file
- [ ] Add `find_package("module_name" REQUIRED)` to main/CMakeLists.txt
- [ ] Build project: `cd build && cmake .. && make -j4`
- [ ] Run tests: `ctest --output-on-failure`
- [ ] Verify all tests pass

---

## Layer Guidelines

- **L5_Common**: Shared utilities, logging, config, signals
- **L4_Infrastructure**: Low-level services, drivers, OS interfaces
- **L3_Storage**: Data persistence, databases, file systems
- **L2_Services**: Business logic, application services
- **L1_Presentation**: UI, API endpoints, external interfaces

---

## Naming Conventions

- Module names: `layer_module` (e.g., `comm_config-toml`, `infr_driver`)
- Class names: PascalCase (e.g., `Config`, `ModuleName`)
- Functions: PascalCase for public, snake_case for private
- Files: match module name (e.g., `comm_config_toml.h`)
- CMake targets: `${PROJECT_NAME}-${MODULE_NAME}` (e.g., `modu-core-comm_config-toml`)

---

## Reference Modules

Well-implemented modules to use as reference:
- **comm_terminate**: Signal handling, threading, observer pattern
- **comm_main**: Module orchestration, initialization order
- **comm_config-toml**: Singleton pattern, error handling
- **infr_main**: Infrastructure layer example

---

**Last Updated:** 2026-01-10
