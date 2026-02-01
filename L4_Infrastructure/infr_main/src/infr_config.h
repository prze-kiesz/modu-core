#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include <toml.hpp>

namespace infr {

struct InfrMainConfig {
    std::string device_name = "default_device";
    int port = 8080;
    bool enable_logging = true;
    double timeout_seconds = 30.0;
};

// ADL-based serialization functions
void to_toml(toml::value& dest, const InfrMainConfig& value);
void from_toml(const toml::value& src, InfrMainConfig& value);

/**
 * @class InfrConfig
 * @brief Singleton configuration manager for Infrastructure layer
 * @details Provides thread-safe access to InfrMainConfig and propagates
 *          reload notifications from comm::Config to L4 modules
 */
class InfrConfig {
 public:
  /**
   * @brief Get singleton instance
   * @return Reference to the singleton instance
   */
  static InfrConfig& Instance();

  // Delete copy/move constructors and assignment operators
  InfrConfig(const InfrConfig&) = delete;
  InfrConfig& operator=(const InfrConfig&) = delete;
  InfrConfig(InfrConfig&&) = delete;
  InfrConfig& operator=(InfrConfig&&) = delete;

  /**
   * @brief Initialize configuration from comm::Config
   * @details Loads current config and registers reload listener
   */
  void Initialize();

  /**
   * @brief Get current configuration (thread-safe)
   * @return Copy of current InfrMainConfig
   */
  InfrMainConfig Get() const;

  /**
   * @brief Register listener to be notified on config reload
   * @param listener Callback invoked after config reload
   */
  void RegisterReloadListener(std::function<void()> listener);

 private:
  InfrConfig() = default;
  ~InfrConfig() = default;

  /**
   * @brief Called by comm::Config when reload happens
   */
  void OnConfigReload();

  /**
   * @brief Reload configuration from comm::Config
   */
  void Reload();

  /**
   * @brief Notify registered listeners after reload
   */
  void NotifyListeners();

  bool m_initialized{false};
  InfrMainConfig m_config;
  mutable std::mutex m_config_mutex;
  std::vector<std::function<void()>> m_listeners;
  mutable std::mutex m_listeners_mutex;
};

}  // namespace infr
