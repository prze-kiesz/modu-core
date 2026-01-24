// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file infr_config.cpp
 * @brief Implementation of InfrMainConfig serialization
 */

#include "infr_config.h"
#include <glog/logging.h>

namespace infr {

void to_toml(toml::value& dest, const InfrMainConfig& value) {
  dest["device_name"] = value.device_name;
  dest["port"] = value.port;
  dest["enable_logging"] = value.enable_logging;
  dest["timeout_seconds"] = value.timeout_seconds;
  
  LOG(INFO) << "Serialized InfrMainConfig";
  LOG(INFO) << "  device_name: " << value.device_name;
  LOG(INFO) << "  port: " << value.port;
  LOG(INFO) << "  enable_logging: " << (value.enable_logging ? "true" : "false");
  LOG(INFO) << "  timeout_seconds: " << value.timeout_seconds;
}

void from_toml(const toml::value& src, InfrMainConfig& value) {
  // Use default-constructed struct as source of default values (created once)
  static const InfrMainConfig defaults;
  
  try {
    // Parse each field with default values from struct definition
    value.device_name = toml::find_or(src, "device_name", defaults.device_name);
    value.port = toml::find_or(src, "port", defaults.port);
    value.enable_logging = toml::find_or(src, "enable_logging", defaults.enable_logging);
    value.timeout_seconds = toml::find_or(src, "timeout_seconds", defaults.timeout_seconds);
    
    LOG(INFO) << "Loaded InfrMainConfig";
    LOG(INFO) << "  device_name: " << value.device_name;
    LOG(INFO) << "  port: " << value.port;
    LOG(INFO) << "  enable_logging: " << (value.enable_logging ? "true" : "false");
    LOG(INFO) << "  timeout_seconds: " << value.timeout_seconds;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error parsing InfrMainConfig: " << e.what();
    LOG(WARNING) << "Using default values";
    value = defaults;
  }
}

}  // namespace infr
