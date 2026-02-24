// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file test_fixture.h
 * @brief Test fixture for controlling application during BDD tests
 */

#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>

#ifdef ENABLE_TEST_HOOKS

// Forward declarations
namespace infr {
  struct InfrMainConfig;
  class InfrConfig;
}

namespace test {

/**
 * @brief Test fixture to control the application under test
 * @details Manages application lifecycle, signal delivery, and state inspection
 */
class TestFixture {
 public:
  static TestFixture& Instance();

  // Delete copy/move
  TestFixture(const TestFixture&) = delete;
  TestFixture& operator=(const TestFixture&) = delete;

  /**
   * @brief Start the application in a separate thread
   * @param config_file Optional config file path
   */
  void StartApplication(const std::string& config_file = "");

  /**
   * @brief Stop the application gracefully
   */
  void StopApplication();

  /**
   * @brief Check if application is running
   */
  bool IsRunning() const;

  /**
   * @brief Send signal to the application
   * @param signal Signal number (SIGTERM, SIGHUP, etc.)
   */
  void SendSignal(int signal);

  /**
   * @brief Wait for application to exit
   * @param timeout_ms Timeout in milliseconds
   * @return Exit code
   */
  int WaitForExit(int timeout_ms);

  /**
   * @brief Set config file path
   */
  void SetConfigPath(const std::string& path);

  /**
   * @brief Get current InfrMainConfig
   */
  infr::InfrMainConfig GetInfrConfig();

  /**
   * @brief Register a listener on InfrConfig
   */
  void RegisterInfrConfigListener(std::function<void()> listener);

  /**
   * @brief Reset fixture state between scenarios
   */
  void Reset();

 private:
  TestFixture() = default;
  ~TestFixture();

  void RunApplication();

  std::unique_ptr<std::thread> m_app_thread;
  std::atomic<bool> m_running{false};
  std::atomic<int> m_exit_code{-1};
  std::string m_config_file;
};

} // namespace test

#endif // ENABLE_TEST_HOOKS
