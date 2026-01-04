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
    case InitError::Success:
      return "Success";
    case InitError::ModuleInitFailed:
      return "Module initialization failed";
    case InitError::ExceptionThrown:
      return "Exception was thrown during operation";
    default:
      return "Unknown error";
  }
}

const std::error_category& get_init_error_category() noexcept {
  static InitErrorCategory instance;
  return instance;
}

// Default constructor - no initialization required (modules initialized in Init())
Main::Main() = default;

std::error_code Main::Init(int  /*argc*/, const char*  /*argv*/[]) {  // NOLINT
  // TODO: Future expansion - use argc/argv for configuration file path or command-line options
  // TODO: Add infrastructure layer modules initialization when implemented

  LOG(INFO) << "Infrastructure layer (L4) initialization completed successfully";
  return {};  // Success - empty error_code
}

std::error_code Main::Deinit() {
  // TODO: Add deinitialization logic for Infrastructure layer modules when implemented
  // This is where network connections, message queues, and hardware resources should be released
  
  LOG(INFO) << "Infrastructure layer (L4) deinitialization completed successfully";
  return {};  // Success - empty error_code
}

}  // namespace infr
