// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file infr_main.cpp
 * @brief Implementation of Infrastructure layer initialization orchestrator
 * @details Coordinates startup of low-level infrastructure modules
 */

#include "infr_main.h"
#include "infr_config.h"

#include <comm_config_toml.h>
#include <glog/logging.h>

namespace infr {

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
  // TODO: Add infrastructure layer modules initialization when implemented
  
  try {
    // Get configuration singleton (already loaded by comm_main or earlier)
    auto& config = comm::Config::Instance();
    
    // Check if configuration is initialized
    if (!config.IsInitialized()) {
      LOG(ERROR) << "Configuration not initialized";
      return makeErrorCode(InitError::MODULE_INIT_FAILED);
    }
    
    // Read infr_main configuration from "infr_main" section
    auto infr_config = config.Get<infr::InfrMainConfig>("infr_main");
    LOG(INFO) << "Device name: " << infr_config.device_name;
    LOG(INFO) << "Port: " << infr_config.port;
    LOG(INFO) << "Logging enabled: " << infr_config.enable_logging;
    LOG(INFO) << "Timeout: " << infr_config.timeout_seconds << "s";
    
  } catch (const std::exception& e) {
    LOG(ERROR) << "Config load failed: " << e.what();
      return makeErrorCode(InitError::MODULE_INIT_FAILED);
  }

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
