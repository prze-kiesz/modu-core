// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_config_client.h
 * @brief Client-facing configuration helpers (serialization + reload hooks)
 */

#pragma once

#include <string>

#include "comm_config_core.h"

namespace comm {

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
void to_toml(const Config& config, const std::string& path, const T& value) {
  toml_serializer<T>::to_toml(config, path, value);
}

/**
 * @brief Helper to deserialize value from TOML
 * @tparam T Type to deserialize to
 * @param config Config instance
 * @param path Key path in TOML
 * @return Deserialized value
 */
template <typename T>
T from_toml(const Config& config, const std::string& path) {
  return toml_serializer<T>::from_toml(config, path);
}

/**
 * @brief Macro to define serialization for simple structs
 * @details Adds a helper to register a config-reload listener that re-fetches
 *          the config section and passes it to the callback.
 * @example
 * COMM_CONFIG_DEFINE_STRUCT(ServerConfig)
 *
 * // Register reload handler (typically in module init)
 * comm::toml_serializer<ServerConfig>::RegisterConfigReloadListener(
 *     "server", [](const ServerConfig& cfg) {
 *       // Apply new config
 *     });
 */
#define COMM_CONFIG_DEFINE_STRUCT(Type)                                         \
  namespace comm {                                                              \
  template <>                                                                   \
  struct toml_serializer<Type> {                                                \
    static void to_toml(const Config& config, const std::string& path,          \
                        const Type& value);                                     \
    static Type from_toml(const Config& config, const std::string& path);       \
    static void RegisterConfigReloadListener(                                   \
        std::string path, std::function<void(const Type&)> callback) {          \
      Config::Instance().RegisterReloadListener(                                \
          [path = std::move(path), callback = std::move(callback)]() {           \
            callback(Config::Instance().Get<Type>(path));                       \
          });                                                                    \
    }                                                                            \
  };                                                                            \
  }

}  // namespace comm
