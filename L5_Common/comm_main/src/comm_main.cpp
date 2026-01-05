// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_main.cpp
 * @brief Implementation of Common layer initialization orchestrator
 * @details Coordinates startup of shared infrastructure modules (logging, signals, etc.)
 */

#include "comm_main.h"

#include <glog/logging.h>
#include "comm_terminate.h"

namespace comm {

// Error category implementation
std::string InitErrorCategory::message(int error_value) const {
  switch (static_cast<InitError>(error_value)) {
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
  // TODO: Add Config module initialization when implemented

  // Initialize graceful shutdown handler (SIGINT, SIGTERM, SIGQUIT)
  auto ret_code = Terminate::Instance().Start();
  if (ret_code) {
    LOG(ERROR) << "Failed to start Terminate::instance().start(): " << ret_code.message();
    return ret_code;
  }

  LOG(INFO) << "Common layer (L5) initialization completed successfully";
  return {};  // Success - empty error_code
}

std::error_code Main::deinit() {
  // TODO: Add deinitialization logic for Common layer modules when needed
  // Currently no cleanup is required as Terminate is handled by waitForTermination()
  
  LOG(INFO) << "Common layer (L5) deinitialization completed successfully";
  return {};  // Success - empty error_code
}

}  // namespace comm
