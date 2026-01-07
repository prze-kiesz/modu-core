// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include "comm_config.h"

#include <fstream>
#include <functional>
#include <sstream>
#include <toml++/toml.h>
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
    case ConfigError::WriteError:
      return "Failed to write configuration file";
    case ConfigError::InvalidPath:
      return "Invalid file path provided";
    case ConfigError::KeyNotFound:
      return "Configuration key not found";
    case ConfigError::TypeMismatch:
      return "Configuration value type mismatch";
    default:
      return "Unknown error";
  }
}

const std::error_category& get_config_error_category() noexcept {
  static ConfigErrorCategory instance;
  return instance;
}

// Internal implementation class
class Config::ConfigImpl {
 public:
  toml::table data;
};

// Constructor implementations
Config::Config() : m_impl(std::make_unique<ConfigImpl>()) {}

Config::~Config() = default;

Config::Config(const std::filesystem::path& file_path) : m_impl(std::make_unique<ConfigImpl>()) {
  [[maybe_unused]] auto err = load_from_file(file_path);
}

// File operations
std::error_code Config::load_from_file(const std::filesystem::path& file_path) {
  if (!std::filesystem::exists(file_path)) {
    LOG(WARNING) << "Configuration file not found: " << file_path;
    return make_error_code(ConfigError::FileNotFound);
  }

  try {
    m_impl->data = toml::parse_file(file_path.string());
    LOG(INFO) << "Successfully loaded configuration from: " << file_path;
    return make_error_code(ConfigError::Success);
  } catch (const toml::parse_error& err) {
    LOG(ERROR) << "Failed to parse configuration file: " << file_path << " - " << err.description();
    return make_error_code(ConfigError::ParseError);
  }
}

std::error_code Config::save_to_file(const std::filesystem::path& file_path) const {
  try {
    // Create parent directory if it doesn't exist
    std::filesystem::create_directories(file_path.parent_path());

    std::ofstream file(file_path);
    if (!file.is_open()) {
      LOG(ERROR) << "Failed to open file for writing: " << file_path;
      return make_error_code(ConfigError::WriteError);
    }

    file << m_impl->data;
    LOG(INFO) << "Successfully saved configuration to: " << file_path;
    return make_error_code(ConfigError::Success);
  } catch (const std::exception& err) {
    LOG(ERROR) << "Failed to write configuration file: " << file_path << " - " << err.what();
    return make_error_code(ConfigError::WriteError);
  }
}

std::error_code Config::merge_from_file(const std::filesystem::path& file_path) {
  if (!std::filesystem::exists(file_path)) {
    LOG(WARNING) << "Configuration file not found (merge skipped): " << file_path;
    return make_error_code(ConfigError::FileNotFound);
  }

  try {
    toml::table new_data = toml::parse_file(file_path.string());
    
    // Recursive merge function
    std::function<void(toml::table&, const toml::table&)> merge_tables;
    merge_tables = [&](toml::table& target, const toml::table& source) {
      for (const auto& [key, value] : source) {
        if (value.is_table() && target[key].is_table()) {
          merge_tables(*target[key].as_table(), *value.as_table());
        } else {
          target.insert_or_assign(key, value);
        }
      }
    };

    merge_tables(m_impl->data, new_data);
    LOG(INFO) << "Successfully merged configuration from: " << file_path;
    return make_error_code(ConfigError::Success);
  } catch (const toml::parse_error& err) {
    LOG(ERROR) << "Failed to parse configuration file for merge: " << file_path << " - "
               << err.description();
    return make_error_code(ConfigError::ParseError);
  }
}

std::error_code Config::load_xdg_hierarchy(
    const std::string& app_name,
    const std::optional<std::filesystem::path>& system_config_path) {
  
  bool loaded_any = false;

  // 1. System configuration
  std::filesystem::path sys_config = system_config_path.value_or(
      std::filesystem::path("/etc") / app_name / "config.toml");
  
  if (std::filesystem::exists(sys_config)) {
    auto err = load_from_file(sys_config);
    if (!err) {
      loaded_any = true;
    }
  }

  // 2. User configuration (XDG_CONFIG_HOME or ~/.config)
  const char* xdg_config_home = std::getenv("XDG_CONFIG_HOME");
  std::filesystem::path user_config;
  
  if (xdg_config_home) {
    user_config = std::filesystem::path(xdg_config_home) / app_name / "config.toml";
  } else {
    const char* home = std::getenv("HOME");
    if (home) {
      user_config = std::filesystem::path(home) / ".config" / app_name / "config.toml";
    }
  }

  if (!user_config.empty() && std::filesystem::exists(user_config)) {
    auto err = merge_from_file(user_config);
    if (!err) {
      loaded_any = true;
    }
  }

  return loaded_any ? make_error_code(ConfigError::Success)
                    : make_error_code(ConfigError::FileNotFound);
}

// Helper to navigate nested keys
[[maybe_unused]] static toml::node* navigate_to_node(toml::table& table, const std::string& key) {
  std::istringstream stream(key);
  std::string segment;
  toml::table* current_table = &table;

  while (std::getline(stream, segment, '.')) {
    if (stream.eof()) {
      // Last segment - return the node
      return current_table->get(segment);
    }
    
    // Intermediate segment - must be a table
    auto node = current_table->get(segment);
    if (!node || !node->is_table()) {
      return nullptr;
    }
    current_table = node->as_table();
  }

  return nullptr;
}

[[maybe_unused]] static const toml::node* navigate_to_node(const toml::table& table, const std::string& key) {
  std::istringstream stream(key);
  std::string segment;
  const toml::table* current_table = &table;

  while (std::getline(stream, segment, '.')) {
    if (stream.eof()) {
      // Last segment - return the node
      return current_table->get(segment);
    }
    
    // Intermediate segment - must be a table
    auto node = current_table->get(segment);
    if (!node || !node->is_table()) {
      return nullptr;
    }
    current_table = node->as_table();
  }

  return nullptr;
}

// Getter implementations
std::optional<std::string> Config::get_string(const std::string& key) const {
  auto* node = navigate_to_node(m_impl->data, key);
  if (node && node->is_string()) {
    return std::string(node->as_string()->get());
  }
  return std::nullopt;
}

std::optional<int64_t> Config::get_int(const std::string& key) const {
  auto* node = navigate_to_node(m_impl->data, key);
  if (node && node->is_integer()) {
    return node->as_integer()->get();
  }
  return std::nullopt;
}

std::optional<double> Config::get_double(const std::string& key) const {
  auto* node = navigate_to_node(m_impl->data, key);
  if (node && node->is_floating_point()) {
    return node->as_floating_point()->get();
  }
  if (node && node->is_integer()) {
    return static_cast<double>(node->as_integer()->get());
  }
  return std::nullopt;
}

std::optional<bool> Config::get_bool(const std::string& key) const {
  auto* node = navigate_to_node(m_impl->data, key);
  if (node && node->is_boolean()) {
    return node->as_boolean()->get();
  }
  return std::nullopt;
}

std::optional<std::vector<std::string>> Config::get_string_array(const std::string& key) const {
  auto* node = navigate_to_node(m_impl->data, key);
  if (node && node->is_array()) {
    auto* arr = node->as_array();
    std::vector<std::string> result;
    result.reserve(arr->size());
    
    for (const auto& elem : *arr) {
      if (elem.is_string()) {
        result.push_back(std::string(elem.as_string()->get()));
      }
    }
    
    return result;
  }
  return std::nullopt;
}

// Helper to create nested tables
static toml::table* create_nested_table(toml::table& root, const std::string& key) {
  std::istringstream stream(key);
  std::string segment;
  toml::table* current_table = &root;

  while (std::getline(stream, segment, '.')) {
    if (stream.eof()) {
      // Last segment - caller will insert value here
      return current_table;
    }
    
    // Intermediate segment - create table if needed
    auto node = current_table->get(segment);
    if (!node) {
      current_table->insert(segment, toml::table{});
      node = current_table->get(segment);
    }
    
    if (!node->is_table()) {
      // Can't navigate further - overwrite with table
      current_table->insert_or_assign(segment, toml::table{});
      node = current_table->get(segment);
    }
    
    current_table = node->as_table();
  }

  return current_table;
}

static std::string get_last_segment(const std::string& key) {
  size_t pos = key.find_last_of('.');
  return pos == std::string::npos ? key : key.substr(pos + 1);
}

// Setter implementations
void Config::set_string(const std::string& key, const std::string& value) {
  auto* table = create_nested_table(m_impl->data, key);
  table->insert_or_assign(get_last_segment(key), value);
}

void Config::set_int(const std::string& key, int64_t value) {
  auto* table = create_nested_table(m_impl->data, key);
  table->insert_or_assign(get_last_segment(key), value);
}

void Config::set_double(const std::string& key, double value) {
  auto* table = create_nested_table(m_impl->data, key);
  table->insert_or_assign(get_last_segment(key), value);
}

void Config::set_bool(const std::string& key, bool value) {
  auto* table = create_nested_table(m_impl->data, key);
  table->insert_or_assign(get_last_segment(key), value);
}

void Config::set_string_array(const std::string& key, const std::vector<std::string>& value) {
  auto* table = create_nested_table(m_impl->data, key);
  toml::array arr;
  for (const auto& str : value) {
    arr.push_back(str);
  }
  table->insert_or_assign(get_last_segment(key), arr);
}

// Utility implementations
bool Config::has_key(const std::string& key) const {
  return navigate_to_node(m_impl->data, key) != nullptr;
}

bool Config::remove_key(const std::string& key) {
  std::istringstream stream(key);
  std::string segment;
  std::vector<std::string> segments;
  
  while (std::getline(stream, segment, '.')) {
    segments.push_back(segment);
  }

  if (segments.empty()) {
    return false;
  }

  toml::table* current_table = &m_impl->data;
  
  // Navigate to parent table
  for (size_t idx = 0; idx < segments.size() - 1; ++idx) {
    auto node = current_table->get(segments[idx]);
    if (!node || !node->is_table()) {
      return false;
    }
    current_table = node->as_table();
  }

  return current_table->erase(segments.back());
}

void Config::clear() {
  m_impl->data.clear();
}

std::vector<std::string> Config::get_all_keys() const {
  std::vector<std::string> keys;
  
  std::function<void(const toml::table&, const std::string&)> collect_keys;
  collect_keys = [&](const toml::table& table, const std::string& prefix) {
    for (const auto& [key, value] : table) {
      std::string full_key = prefix.empty() ? std::string(key) : prefix + "." + std::string(key);
      
      if (value.is_table()) {
        collect_keys(*value.as_table(), full_key);
      } else {
        keys.push_back(full_key);
      }
    }
  };

  collect_keys(m_impl->data, "");
  return keys;
}

}  // namespace comm
