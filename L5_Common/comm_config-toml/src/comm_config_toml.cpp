// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_config_toml.cpp
 * @brief TOML-based configuration management implementation
 */

#include "comm_config_toml.h"

#include <glog/logging.h>

namespace comm {

// Error category implementation
std::string ConfigErrorCategory::message(int error_value) const {
  switch (static_cast<ConfigError>(error_value)) {
    case ConfigError::Success:
      return "Success";
    case ConfigError::FileNotFound:
      return "Configuration file not found";
    case ConfigError::ParseError:
      return "Failed to parse configuration file";
    case ConfigError::ValidationError:
      return "Configuration validation failed";
    case ConfigError::NotInitialized:
      return "Configuration not initialized";
    default:
      return "Unknown configuration error";
  }
}

const std::error_category& get_config_error_category() noexcept {
  static ConfigErrorCategory instance;
  return instance;
}

Config& Config::Instance() {
  static Config instance;
  return instance;
}

Config::Config() {
  LOG(INFO) << "Config instance created";
}

std::error_code Config::Initialize() {
  LOG(INFO) << "Config::Initialize() called";
  
  // TODO: Implement initialization logic
  
  m_initialized = true;
  return make_error_code(ConfigError::Success);
}

std::error_code Config::Load(const std::string& config_path) {
  LOG(INFO) << "Config::Load() called with path: " << config_path;
  
  try {
    m_data = toml::parse(config_path);
    m_config_path = config_path;
    m_initialized = true;
    LOG(INFO) << "Successfully loaded TOML configuration from: " << config_path;
    return make_error_code(ConfigError::Success);
  } catch (const toml::syntax_error& e) {
    LOG(ERROR) << "TOML parse error: " << e.what();
    return make_error_code(ConfigError::ParseError);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to load config file: " << e.what();
    return make_error_code(ConfigError::FileNotFound);
  }
}

std::error_code Config::Reload() {
  LOG(INFO) << "Config::Reload() called";
  
  if (!m_initialized) {
    LOG(ERROR) << "Cannot reload: configuration not initialized";
    return make_error_code(ConfigError::NotInitialized);
  }
  
  return Load(m_config_path);
}

bool Config::IsInitialized() const {
  return m_initialized;
}

const toml::value& Config::GetData() const {
  return m_data;
}

}  // namespace comm
