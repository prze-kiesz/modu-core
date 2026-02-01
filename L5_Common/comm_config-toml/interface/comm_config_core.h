// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_config_core.h
 * @brief Core TOML-based configuration management (init/reload/overrides)
 * @details Provides configuration loading and access from TOML files with
 *          trait-based serialization using ADL (Argument-Dependent Lookup)
 */

#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <toml.hpp>

namespace comm {

/**
 * @enum ConfigError
 * @brief Error codes for configuration operations
 */
enum class ConfigError {
  Success = 0,
  FileNotFound,
  ParseError,
  ValidationError,
  NotInitialized
};

/**
 * @class ConfigErrorCategory
 * @brief Error category for configuration errors
 */
class ConfigErrorCategory : public std::error_category {
 public:
  const char* name() const noexcept override { return "config"; }
  std::string message(int error_value) const override;
};

/**
 * @brief Get the singleton instance of configuration error category
 * @return Reference to the error category
 */
const std::error_category& get_config_error_category() noexcept;

/**
 * @brief Create an error_code from ConfigError enum
 * @param error Configuration error value
 * @return std::error_code representing the error
 */
inline std::error_code make_error_code(ConfigError error) {
  return {static_cast<int>(error), get_config_error_category()};
}

/**
 * @class Config
 * @brief Singleton configuration manager for TOML-based configuration
 */
class Config {
 public:
  /**
   * @brief Get singleton instance of Config
   * @return Reference to the singleton instance
   */
  static Config& Instance();

  // Delete copy/move constructors and assignment operators
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;
  Config(Config&&) = delete;
  Config& operator=(Config&&) = delete;

  /**
   * @brief Initialize configuration system with XDG Base Directory hierarchy
   * @param app_name Application name for config directory (e.g., "modu-core")
   * @return Error code indicating success or failure
   * @details Loads configuration in order:
   *          1. /etc/<app_name>/config.toml (system defaults)
   *          2. ~/.config/<app_name>/config.toml (user overrides)
   *          Later files override earlier ones
   */
  std::error_code Initialize(const std::string& app_name);

  /**
   * @brief Load configuration from TOML file
   * @param config_path Path to TOML configuration file
   * @return Error code indicating success or failure
   */
  std::error_code Load(const std::string& config_path);

  /**
   * @brief Reload configuration from previously loaded file
   * @return Error code indicating success or failure
   */
  std::error_code Reload();

  /**
   * @brief Override specific configuration value (highest priority)
   * @param path Dot-separated path (e.g., "infr_main.port")
   * @param value String value to set (converted to appropriate type)
   * @details Supports type inference: "123" -> int, "true" -> bool, "text" -> string
   */
  void SetOverride(const std::string& path, const std::string& value);

  /**
   * @brief Register a callback invoked after successful config reload
   * @param callback Function to call after Reload() finishes successfully
   * @note Callback is invoked from the thread that calls Reload()
   * @note Multiple listeners are invoked sequentially in registration order
   * @note Thread-safe: can be called from any thread
   */
  void RegisterReloadListener(std::function<void()> callback);

  /**
   * @brief Check if configuration is initialized
   * @return True if initialized, false otherwise
   */
  bool IsInitialized() const;

  /**
   * @brief Get parsed TOML data
   * @return Reference to parsed TOML table
   */
  const toml::value& GetData() const;

  /**
   * @brief Get configuration value of type T from specified path
   * @tparam T Type to deserialize (must have from_toml function in its namespace)
   * @param path Key path in TOML
   * @return Deserialized value
   */
  template <typename T>
  T Get(const std::string& path) const {
    T result;
    try {
      std::lock_guard<std::mutex> lock(m_data_mutex);
      if (m_data.is_table() && m_data.contains(path)) {
        const auto& section = toml::find(m_data, path);
        from_toml(section, result);  // ADL will find the appropriate from_toml
      } else {
        // Section not found, result will have default values
        from_toml(toml::value{}, result);  // Pass empty toml::value to trigger defaults
      }
    } catch (const std::exception&) {
      // On any error, result will have default values from from_toml
    }
    return result;
  }

 private:
  Config();
  ~Config() = default;

  /**
   * @brief Get XDG config home directory
   * @return Path to config home ($XDG_CONFIG_HOME or ~/.config)
   */
  std::string GetXdgConfigHome() const;

  /**
   * @brief Merge source TOML into destination (recursive)
   * @param dest Destination TOML value to merge into
   * @param src Source TOML value to merge from
   */
  void MergeToml(toml::value& dest, const toml::value& src);

  /**
   * @brief Infer TOML value type from string and convert
   * @param value_str String value to convert
   * @return TOML value with inferred type
   */
  toml::value InferValueType(const std::string& value_str) const;

  /**
   * @brief Apply stored overrides to current data
   */
  void ApplyOverrides();

  /**
   * @brief Apply a single override to current data (does not store it)
   * @param path Dot-separated path (e.g., "infr_main.port")
   * @param value String value to set (converted to appropriate type)
   */
  void ApplyOverrideToData(const std::string& path, const std::string& value);

  /**
   * @brief Apply override without acquiring m_data_mutex (caller must hold lock)
   * @param path Dot-separated path (e.g., "infr_main.port")
   * @param value String value to set (converted to appropriate type)
   */
  void ApplyOverrideToDataNoLock(const std::string& path, const std::string& value);

  /**
   * @brief Notify registered listeners after successful reload
   */
  void NotifyReloadListeners();

  bool m_initialized{false};
  std::string m_app_name;
  std::vector<std::string> m_config_paths;  // For reload
  toml::value m_data;
  mutable std::mutex m_data_mutex;  // Protects m_data access
  std::vector<std::function<void()>> m_reload_listeners;
  mutable std::mutex m_reload_listeners_mutex;
  std::unordered_map<std::string, std::string> m_overrides;  // O(1) lookup
  mutable std::mutex m_overrides_mutex;
};

}  // namespace comm

// Enable std::error_code support for ConfigError
namespace std {
template <>
struct is_error_code_enum<comm::ConfigError> : true_type {};
}  // namespace std
