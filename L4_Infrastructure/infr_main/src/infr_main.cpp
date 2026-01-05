// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file infr_main.cpp
 * @brief Implementation of Infrastructure layer initialization orchestrator
 * @details Coordinates startup of low-level infrastructure modules
 */

#include "infr_main.h"

#include <glog/logging.h>

namespace infr {

// Error category implementation
std::string InitErrorCategory::message(int ev) const {
  switch (static_cast<InitError>(ev)) {
    case InitError::SUCCESS:
      return "Success";
    case InitError::MODULE_INIT_FAILED:
      return "Module initialization failed";
    case InitError::EXCEPTION_THROWN:
      return "Exception was thrown during operation";
    default:
      return "Unknown error";
  }
}

const std::error_category& getInitErrorCategory() noexcept {
  static InitErrorCategory instance;
  return instance;
}

// Default constructor - no initialization required (modules initialized in init())
Main::Main() = default;

std::error_code Main::init(int  /*argc*/, const char*  /*argv*/[]) {  // NOLINT
  // TODO: Future expansion - use argc/argv for configuration file path or command-line options
  // TODO: Add infrastructure layer modules initialization when implemented

  LOG(INFO) << "Infrastructure layer (L4) initialization completed successfully";
  return {};  // Success - empty error_code
}

std::error_code Main::deinit() {
  // TODO: Add deinitialization logic for Infrastructure layer modules when implemented
  // This is where network connections, message queues, and hardware resources should be released
  
  LOG(INFO) << "Infrastructure layer (L4) deinitialization completed successfully";
  return {};  // Success - empty error_code
}

}  // namespace infr
