// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file infr_config.cpp
 * @brief Implementation of InfrMainConfig serialization and InfrConfig singleton
 */

#include "infr_config.h"
#include <comm_config_client.h>
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

// InfrConfig implementation

InfrConfig& InfrConfig::Instance() {
  static InfrConfig instance;
  return instance;
}

void InfrConfig::Initialize() {
  if (m_initialized) {
    LOG(WARNING) << "InfrConfig already initialized";
    return;
  }

  LOG(INFO) << "Initializing InfrConfig";

  // Load initial configuration
  Reload();

  // Register reload listener with comm::Config
  comm::Config::Instance().RegisterReloadListener([this]() {
    OnConfigReload();
  });

  m_initialized = true;
  LOG(INFO) << "InfrConfig initialized successfully";
}

InfrMainConfig InfrConfig::Get() const {
  std::lock_guard<std::mutex> lock(m_config_mutex);
  return m_config;
}

void InfrConfig::RegisterReloadListener(std::function<void()> listener) {
  std::lock_guard<std::mutex> lock(m_listeners_mutex);
  m_listeners.push_back(std::move(listener));
  LOG(INFO) << "Registered InfrConfig reload listener (total: " << m_listeners.size() << ")";
}

void InfrConfig::OnConfigReload() {
  LOG(INFO) << "InfrConfig received reload notification from comm::Config";
  Reload();
  NotifyListeners();
}

void InfrConfig::Reload() {
  try {
    auto& config = comm::Config::Instance();
    auto new_config = config.Get<InfrMainConfig>("infr_main");

    {
      std::lock_guard<std::mutex> lock(m_config_mutex);
      m_config = new_config;
    }

    LOG(INFO) << "InfrConfig reloaded successfully";
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to reload InfrConfig: " << e.what();
  }
}

void InfrConfig::NotifyListeners() {
  std::vector<std::function<void()>> listeners_copy;
  {
    std::lock_guard<std::mutex> lock(m_listeners_mutex);
    listeners_copy = m_listeners;
  }

  LOG(INFO) << "Notifying " << listeners_copy.size() << " InfrConfig listener(s)";
  for (const auto& listener : listeners_copy) {
    try {
      listener();
    } catch (const std::exception& e) {
      LOG(ERROR) << "InfrConfig listener threw exception: " << e.what();
    }
  }
}

}  // namespace infr
