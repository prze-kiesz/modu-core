// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_main.cpp
 * @brief Implementation of Common layer initialization orchestrator
 * @details Coordinates startup of shared infrastructure modules (logging, signals, etc.)
 */

#include "comm_main.h"

#include <glog/logging.h>
#include <cstring>
#include "comm_config.h"
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

std::error_code Main::init(int argc, const char* argv[]) {
  // Get global configuration singleton
  Config& config = Config::instance();
  
  // Check for --config command-line argument
  const char* config_path = nullptr;
  for (int idx = 1; idx < argc - 1; ++idx) {
    if (std::strcmp(argv[idx], "--config") == 0) {
      config_path = argv[idx + 1];
      LOG(INFO) << "Using configuration file from command line: " << config_path;
      break;
    }
  }

  std::error_code err;
  if (config_path) {
    // Load from specified path
    err = config.load_from_file(config_path);
    if (err) {
      LOG(WARNING) << "Failed to load config from " << config_path << ": " << err.message();
      LOG(INFO) << "Falling back to XDG hierarchy";
      err = config.load_xdg_hierarchy("modu-core");
    }
  } else {
    // Load using XDG hierarchy: /etc/modu-core/ and ~/.config/modu-core/
    err = config.load_xdg_hierarchy("modu-core");
  }

  if (err && err != make_error_code(ConfigError::FileNotFound)) {
    LOG(ERROR) << "Failed to load configuration: " << err.message();
    return makeErrorCode(InitError::MODULE_INIT_FAILED);
  }
  
  if (err == make_error_code(ConfigError::FileNotFound)) {
    LOG(INFO) << "No configuration files found, using defaults";
  } else {
    LOG(INFO) << "Configuration loaded successfully";
    auto keys = config.get_all_keys();
    LOG(INFO) << "Loaded " << keys.size() << " configuration keys";
  }

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
