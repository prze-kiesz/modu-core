// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_config_toml.cpp
 * @brief TOML-based configuration management implementation
 */

#include "comm_config_toml.h"

#include <glog/logging.h>
#include <cstdlib>
#include <fstream>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

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
  // Ensure default data is a table to avoid null state before initialization
  m_data = toml::table{};
}

std::string Config::GetXdgConfigHome() const {
  // Check XDG_CONFIG_HOME environment variable
  const char* xdg_config_home = std::getenv("XDG_CONFIG_HOME");
  if (xdg_config_home && xdg_config_home[0] == '/') {
    return xdg_config_home;
  }
  
  // Fallback to ~/.config
  const char* home = std::getenv("HOME");
  if (home) {
    return std::string(home) + "/.config";
  }
  
  // Last resort: use getpwuid
  struct passwd* pw = getpwuid(getuid());
  if (pw && pw->pw_dir) {
    return std::string(pw->pw_dir) + "/.config";
  }
  
  LOG(ERROR) << "Unable to determine home directory";
  return "/tmp/.config";
}

void Config::MergeToml(toml::value& dest, const toml::value& src) {
  if (!src.is_table() || !dest.is_table()) {
    // If not both tables, source overwrites destination
    dest = src;
    return;
  }
  
  // Merge tables recursively
  const auto& src_table = src.as_table();
  auto& dest_table = dest.as_table();
  
  for (const auto& [key, value] : src_table) {
    if (dest_table.contains(key) && dest_table[key].is_table() && value.is_table()) {
      // Both are tables - merge recursively
      MergeToml(dest_table[key], value);
    } else {
      // Source overwrites destination
      dest_table[key] = value;
    }
  }
}

std::error_code Config::Initialize(const std::string& app_name) {
  LOG(INFO) << "Config::Initialize() called for app: " << app_name;
  
  m_app_name = app_name;
  m_config_paths.clear();
  m_data = toml::value{};  // Empty table
  
  // Build paths in XDG hierarchy
  std::vector<std::string> candidate_paths = {
    "/etc/" + app_name + "/config.toml",              // System config
    GetXdgConfigHome() + "/" + app_name + "/config.toml"  // User config
  };
  
  bool loaded_any = false;
  
  // Load and merge each config file that exists
  for (const auto& path : candidate_paths) {
    std::ifstream file(path);
    if (!file.good()) {
      LOG(INFO) << "Config file not found (optional): " << path;
      continue;
    }
    
    try {
      toml::value config_data = toml::parse(path);
      LOG(INFO) << "Loaded config from: " << path;
      
      if (m_data.is_table()) {
        MergeToml(m_data, config_data);
        LOG(INFO) << "Merged config from: " << path;
      } else {
        m_data = config_data;
      }
      
      m_config_paths.push_back(path);
      loaded_any = true;
    } catch (const toml::syntax_error& e) {
      LOG(ERROR) << "TOML parse error in " << path << ": " << e.what();
      return make_error_code(ConfigError::ParseError);
    } catch (const std::exception& e) {
      LOG(ERROR) << "Error loading " << path << ": " << e.what();
      // Continue with other files
    }
  }
  
  if (!loaded_any) {
    LOG(WARNING) << "No configuration files found, using defaults";
  }
  
  m_initialized = true;
  LOG(INFO) << "Configuration initialized with " << m_config_paths.size() << " file(s)";
  return make_error_code(ConfigError::Success);
}

std::error_code Config::Load(const std::string& config_path) {
  LOG(INFO) << "Config::Load() called with path: " << config_path;
  
  try {
    m_data = toml::parse(config_path);
    m_config_paths = {config_path};
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
  
  if (m_config_paths.empty()) {
    LOG(ERROR) << "Cannot reload: no configuration paths stored";
    return make_error_code(ConfigError::NotInitialized);
  }
  
  // If initialized with app_name (XDG hierarchy), re-initialize
  if (!m_app_name.empty()) {
    LOG(INFO) << "Reloading XDG hierarchy for app: " << m_app_name;
    return Initialize(m_app_name);
  }
  
  // Otherwise reload single file
  LOG(INFO) << "Reloading single config file: " << m_config_paths[0];
  return Load(m_config_paths[0]);
}

bool Config::IsInitialized() const {
  return m_initialized;
}

const toml::value& Config::GetData() const {
  return m_data;
}

toml::value Config::InferValueType(const std::string& value_str) const {
  // Try to infer type from string
  
  // Boolean
  if (value_str == "true" || value_str == "TRUE") {
    return toml::value(true);
  }
  if (value_str == "false" || value_str == "FALSE") {
    return toml::value(false);
  }
  
  // Integer
  try {
    size_t pos;
    int64_t int_val = std::stoll(value_str, &pos);
    if (pos == value_str.length()) {
      return toml::value(int_val);
    }
  } catch (...) {
    // Not an integer, continue
  }
  
  // Float
  try {
    size_t pos;
    double float_val = std::stod(value_str, &pos);
    if (pos == value_str.length()) {
      return toml::value(float_val);
    }
  } catch (...) {
    // Not a float, continue
  }
  
  // Default to string
  return toml::value(value_str);
}

void Config::SetOverride(const std::string& path, const std::string& value) {
  LOG(INFO) << "Setting override: " << path << " = " << value;
  
  // Parse path: "infr_main.port" -> ["infr_main", "port"]
  std::vector<std::string> keys;
  size_t start = 0;
  size_t end = path.find('.');
  
  while (end != std::string::npos) {
    keys.push_back(path.substr(start, end - start));
    start = end + 1;
    end = path.find('.', start);
  }
  keys.push_back(path.substr(start));
  
  if (keys.empty()) {
    LOG(ERROR) << "Invalid override path: " << path;
    return;
  }
  
  // Ensure m_data is a table
  if (!m_data.is_table()) {
    m_data = toml::table{};
  }
  
  // Navigate to the target location, creating tables as needed
  toml::value* current = &m_data;
  for (size_t i = 0; i < keys.size() - 1; ++i) {
    const auto& key = keys[i];
    
    if (!current->is_table()) {
      LOG(ERROR) << "Cannot set override: " << key << " is not a table";
      return;
    }
    
    auto& table = current->as_table();
    if (!table.contains(key)) {
      table[key] = toml::table{};
    }
    current = &table[key];
  }
  
  // Set the final value with type inference
  if (current->is_table()) {
    const auto& last_key = keys.back();
    auto& table = current->as_table();
    table[last_key] = InferValueType(value);
    LOG(INFO) << "Successfully set override: " << path << " = " << value;
  } else {
    LOG(ERROR) << "Cannot set override: parent is not a table";
  }
}

}  // namespace comm
