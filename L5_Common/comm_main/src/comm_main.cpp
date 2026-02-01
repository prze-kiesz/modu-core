// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_main.cpp
 * @brief Implementation of Common layer initialization orchestrator
 * @details Coordinates startup of shared infrastructure modules (logging, signals, etc.)
 */

#include "comm_main.h"

#include <glog/logging.h>
#include "comm_config_core.h"
#include "comm_terminate.h"

#ifndef PROJECT_NAME
#define PROJECT_NAME "modu-core"  // Fallback if not building from main/
#endif

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

std::vector<std::pair<std::string, std::string>> Main::ParseCommandLineOverrides(
    int argc, const char* argv[]) {
  std::vector<std::pair<std::string, std::string>> overrides;
  
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    
    // Look for --set key=value pattern
    if (arg == "--set" && i + 1 < argc) {
      std::string keyval = argv[++i];
      size_t eq_pos = keyval.find('=');
      
      if (eq_pos != std::string::npos && eq_pos > 0 && eq_pos < keyval.length() - 1) {
        std::string key = keyval.substr(0, eq_pos);
        std::string value = keyval.substr(eq_pos + 1);
        overrides.push_back({key, value});
        LOG(INFO) << "Parsed CLI override: " << key << " = " << value;
      } else {
        LOG(WARNING) << "Invalid --set format (expected key=value): " << keyval;
      }
    }
  }
  
  return overrides;
}

std::error_code Main::init(int argc, const char* argv[]) {  // NOLINT
  // Use project name from CMake, fallback to extracting from argv[0]
  std::string app_name = PROJECT_NAME;
  
  // Optionally override from argv[0] if provided (for custom builds)
  if (argc > 0 && argv[0]) {
    std::string full_path = argv[0];
    // Extract basename (last part after /)
    size_t last_slash = full_path.find_last_of('/');
    if (last_slash != std::string::npos) {
      std::string basename = full_path.substr(last_slash + 1);
      if (!basename.empty() && basename != PROJECT_NAME) {
        LOG(INFO) << "Using executable name '" << basename 
                  << "' instead of project name '" << PROJECT_NAME << "'";
        app_name = basename;
      }
    }
  }
  
  // Initialize configuration system with XDG hierarchy
  auto config_init = Config::Instance().Initialize(app_name);
  if (config_init) {
    LOG(ERROR) << "Failed to initialize Config module: " << config_init.message();
    return config_init;
  }

  // Parse and apply CLI overrides (highest priority)
  auto cli_overrides = ParseCommandLineOverrides(argc, argv);
  for (const auto& [key, value] : cli_overrides) {
    Config::Instance().SetOverride(key, value);
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
