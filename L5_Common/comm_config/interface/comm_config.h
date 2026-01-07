// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace comm {

/**
 * @brief Error codes for Config module operations
 */
enum class ConfigError {
  Success = 0,           ///< Operation completed successfully
  FileNotFound = 1,      ///< Configuration file not found
  ParseError = 2,        ///< Failed to parse configuration file
  WriteError = 3,        ///< Failed to write configuration file
  InvalidPath = 4,       ///< Invalid file path provided
  KeyNotFound = 5,       ///< Requested configuration key not found
  TypeMismatch = 6,      ///< Configuration value type mismatch
};

/**
 * @brief Error category for Config module errors
 */
class ConfigErrorCategory : public std::error_category {
 public:
  const char* name() const noexcept override { return "comm_config"; }
  std::string message(int error_value) const override;
};

/**
 * @brief Get the singleton instance of ConfigErrorCategory
 */
const std::error_category& get_config_error_category() noexcept;

/**
 * @brief Helper function to create std::error_code from ConfigError
 */
inline std::error_code make_error_code(ConfigError err) noexcept {
  return {static_cast<int>(err), get_config_error_category()};
}

/**
 * @brief Configuration manager using TOML format
 * 
 * Supports XDG Base Directory specification:
 * - System config: /etc/xdg/<app_name>/ or /etc/<app_name>/
 * - User config: $XDG_CONFIG_HOME/<app_name>/ (default: ~/.config/<app_name>/)
 * - Default values: embedded in application
 * 
 * Configuration hierarchy (later overrides earlier):
 * 1. Default values (in-code)
 * 2. System configuration (/etc/)
 * 3. User configuration (~/.config/)
 * 4. Runtime overrides (programmatic)
 */
class Config {
 public:
  /**
   * @brief Construct empty configuration
   */
  Config();

  /**
   * @brief Destructor
   */
  ~Config();

  /**
   * @brief Construct and load configuration from file
   * @param file_path Path to TOML configuration file
   */
  explicit Config(const std::filesystem::path& file_path);

  /**
   * @brief Load configuration from TOML file
   * @param file_path Path to TOML configuration file
   * @return Error code (Success if loaded successfully)
   */
  [[nodiscard]] std::error_code load_from_file(const std::filesystem::path& file_path);

  /**
   * @brief Save current configuration to TOML file
   * @param file_path Path where to save configuration
   * @return Error code (Success if saved successfully)
   */
  [[nodiscard]] std::error_code save_to_file(const std::filesystem::path& file_path) const;

  /**
   * @brief Merge configuration from another file (overrides existing values)
   * @param file_path Path to TOML configuration file
   * @return Error code (Success if merged successfully)
   */
  [[nodiscard]] std::error_code merge_from_file(const std::filesystem::path& file_path);

  /**
   * @brief Load configuration following XDG Base Directory hierarchy
   * @param app_name Application name (used for directory naming)
   * @param system_config_path Optional custom system config path (default: /etc/<app_name>/)
   * @return Error code (Success if at least one file loaded)
   * 
   * Loads in order: system config → user config
   * Missing files are silently skipped (not an error)
   */
  [[nodiscard]] std::error_code load_xdg_hierarchy(
      const std::string& app_name,
      const std::optional<std::filesystem::path>& system_config_path = std::nullopt);

  /**
   * @brief Get string value from configuration
   * @param key Configuration key (supports dot notation: "section.subsection.key")
   * @return Value if exists, std::nullopt otherwise
   */
  [[nodiscard]] std::optional<std::string> get_string(const std::string& key) const;

  /**
   * @brief Get integer value from configuration
   * @param key Configuration key (supports dot notation)
   * @return Value if exists and is integer, std::nullopt otherwise
   */
  [[nodiscard]] std::optional<int64_t> get_int(const std::string& key) const;

  /**
   * @brief Get floating point value from configuration
   * @param key Configuration key (supports dot notation)
   * @return Value if exists and is numeric, std::nullopt otherwise
   */
  [[nodiscard]] std::optional<double> get_double(const std::string& key) const;

  /**
   * @brief Get boolean value from configuration
   * @param key Configuration key (supports dot notation)
   * @return Value if exists and is boolean, std::nullopt otherwise
   */
  [[nodiscard]] std::optional<bool> get_bool(const std::string& key) const;

  /**
   * @brief Get array of strings from configuration
   * @param key Configuration key (supports dot notation)
   * @return Array if exists, std::nullopt otherwise
   */
  [[nodiscard]] std::optional<std::vector<std::string>> get_string_array(
      const std::string& key) const;

  /**
   * @brief Set string value in configuration
   * @param key Configuration key (supports dot notation)
   * @param value Value to set
   */
  void set_string(const std::string& key, const std::string& value);

  /**
   * @brief Set integer value in configuration
   * @param key Configuration key (supports dot notation)
   * @param value Value to set
   */
  void set_int(const std::string& key, int64_t value);

  /**
   * @brief Set floating point value in configuration
   * @param key Configuration key (supports dot notation)
   * @param value Value to set
   */
  void set_double(const std::string& key, double value);

  /**
   * @brief Set boolean value in configuration
   * @param key Configuration key (supports dot notation)
   * @param value Value to set
   */
  void set_bool(const std::string& key, bool value);

  /**
   * @brief Set array of strings in configuration
   * @param key Configuration key (supports dot notation)
   * @param value Array to set
   */
  void set_string_array(const std::string& key, const std::vector<std::string>& value);

  /**
   * @brief Check if configuration key exists
   * @param key Configuration key (supports dot notation)
   * @return true if key exists, false otherwise
   */
  [[nodiscard]] bool has_key(const std::string& key) const;

  /**
   * @brief Remove key from configuration
   * @param key Configuration key (supports dot notation)
   * @return true if key was removed, false if it didn't exist
   */
  bool remove_key(const std::string& key);

  /**
   * @brief Clear all configuration values
   */
  void clear();

  /**
   * @brief Get all configuration keys
   * @return Vector of all keys (in dot notation)
   */
  [[nodiscard]] std::vector<std::string> get_all_keys() const;

 private:
  /// Internal storage for configuration data (TOML table)
  class ConfigImpl;
  std::unique_ptr<ConfigImpl> m_impl;
};

}  // namespace comm

namespace std {
template <>
struct is_error_code_enum<comm::ConfigError> : true_type {};
}  // namespace std
