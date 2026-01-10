// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_config_toml.h
 * @brief TOML-based configuration management
 * @details Provides configuration loading and access from TOML files with
 *          trait-based serialization using ADL (Argument-Dependent Lookup)
 */

#pragma once

#include <optional>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

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
   * @brief Initialize configuration system
   * @return Error code indicating success or failure
   */
  std::error_code Initialize();

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
   * @brief Check if configuration is initialized
   * @return True if initialized, false otherwise
   */
  bool IsInitialized() const;

 private:
  Config();
  ~Config() = default;

  bool m_initialized{false};
  std::string m_config_path;
};

/**
 * @brief ADL serialization trait - specialize for custom types
 * @tparam T Type to serialize/deserialize
 * 
 * @example Usage for custom struct:
 * struct ServerConfig {
 *   int port;
 *   std::string host;
 * };
 * 
 * namespace comm {
 * template <>
 * struct toml_serializer<ServerConfig> {
 *   static void to_toml(const Config& config, const std::string& path, const ServerConfig& value);
 *   static ServerConfig from_toml(const Config& config, const std::string& path);
 * };
 * }
 */
template <typename T, typename = void>
struct toml_serializer;

/**
 * @brief Helper to serialize value to TOML
 * @tparam T Type to serialize
 * @param config Config instance
 * @param path Key path in TOML
 * @param value Value to serialize
 */
template <typename T>
void to_toml(const Config& config, const std::string& path, const T& value);

/**
 * @brief Helper to deserialize value from TOML
 * @tparam T Type to deserialize to
 * @param config Config instance
 * @param path Key path in TOML
 * @return Deserialized value
 */
template <typename T>
T from_toml(const Config& config, const std::string& path);

/**
 * @brief Macro to define serialization for simple structs
 * @example
 * COMM_CONFIG_DEFINE_STRUCT(ServerConfig, port, host, timeout)
 */
#define COMM_CONFIG_DEFINE_STRUCT(Type, ...)                                   \
  namespace comm {                                                              \
  template <>                                                                   \
  struct toml_serializer<Type> {                                               \
    static void to_toml(const Config& config, const std::string& path,        \
                        const Type& value);                                     \
    static Type from_toml(const Config& config, const std::string& path);     \
  };                                                                            \
  }

}  // namespace comm

// Enable std::error_code support for ConfigError
namespace std {
template <>
struct is_error_code_enum<comm::ConfigError> : true_type {};
}  // namespace std
