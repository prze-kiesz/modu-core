// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_main.cpp
 * @brief Implementation of Common layer initialization orchestrator
 * @details Coordinates startup of shared infrastructure modules (logging, signals, etc.)
 */

#include "comm_main.h"

#include <glog/logging.h>
#include "comm_config_toml.h"
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

std::error_code Main::init(int argc, const char* argv[]) {  // NOLINT
  // Initialize configuration system as first module
  auto config_init = Config::Instance().Initialize();
  if (config_init) {
    LOG(ERROR) << "Failed to initialize Config module: " << config_init.message();
    return config_init;
  }

  // Load configuration file (default or from command line)
  // TODO: Parse argc/argv for --config or -c flag
  std::string config_path = "/etc/modu-core/config.toml";  // Default path
  if (argc > 1) {
    // Simple argument parsing - first argument is config path
    config_path = argv[1];
  }
  
  auto config_load = Config::Instance().Load(config_path);
  if (config_load) {
    LOG(WARNING) << "Failed to load config from " << config_path << ": " << config_load.message();
    LOG(WARNING) << "Continuing with default configuration";
  } else {
    LOG(INFO) << "Configuration loaded from: " << config_path;
  }

  // Initialize graceful shutdown handler (SIGINT, SIGTERM, SIGQUIT, SIGHUP)
  auto ret_code = Terminate::Instance().Start();
  if (ret_code) {
    LOG(ERROR) << "Failed to start Terminate::instance().start(): " << ret_code.message();
    return ret_code;
  }
  // Register config reload listener for SIGHUP handling
  Terminate::Instance().RegisterConfigReloadListener([]() {
    LOG(INFO) << "SIGHUP received - reloading configuration";
    auto reload_result = Config::Instance().Reload();
    if (reload_result) {
      LOG(ERROR) << "Failed to reload configuration: " << reload_result.message();
    } else {
      LOG(INFO) << "Configuration reloaded successfully";
    }
  });
  LOG(INFO) << "Common layer (L5) initialization completed successfully";
  return {};  // Success - empty error_code
}

std::error_code Main::deinit() {
  // Note: Config and Terminate are singletons - cleanup happens automatically at program exit
  // No explicit deinitialization needed
  
  LOG(INFO) << "Common layer (L5) deinitialization completed successfully";
  return {};  // Success - empty error_code
}

}  // namespace comm
