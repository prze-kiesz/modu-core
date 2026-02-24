// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file test_hooks.h
 * @brief Test hooks for BDD acceptance tests
 * @details These hooks are only compiled when ENABLE_TEST_HOOKS is defined.
 *          They allow introspection and control of application state during
 *          cucumber-cpp tests without affecting production builds.
 */

#pragma once

#ifdef ENABLE_TEST_HOOKS

#include <functional>
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <any>

namespace test {

/**
 * @brief Central registry for test hooks
 * @details Thread-safe singleton that allows step definitions to observe
 *          application events and state changes.
 */
class TestHooks {
 public:
  static TestHooks& Instance();

  // Delete copy/move
  TestHooks(const TestHooks&) = delete;
  TestHooks& operator=(const TestHooks&) = delete;

  // ============================================================
  // Event Hooks - called from production code at key points
  // ============================================================

  /**
   * @brief Called when config is about to be reloaded
   */
  static void OnConfigReloadStarted();

  /**
   * @brief Called after config reload completes successfully
   * @param section Config section that was reloaded (e.g., "infr_main")
   */
  static void OnConfigReloadCompleted(const std::string& section);

  /**
   * @brief Called when config reload fails
   * @param error Error message
   */
  static void OnConfigReloadFailed(const std::string& error);

  /**
   * @brief Called when application receives termination signal
   * @param signal Signal number (SIGTERM, SIGINT, etc.)
   */
  static void OnTerminationSignalReceived(int signal);

  /**
   * @brief Called during graceful shutdown
   * @param stage Shutdown stage (e.g., "cleanup", "finalize")
   */
  static void OnShutdownStage(const std::string& stage);

  /**
   * @brief Called when InfrConfig is initialized
   */
  static void OnInfrConfigInitialized();

  /**
   * @brief Called when any module completes initialization
   * @param module_name Name of the module
   */
  static void OnModuleInitialized(const std::string& module_name);

  // ============================================================
  // State Access - for test assertions
  // ============================================================

  /**
   * @brief Store arbitrary test data
   * @param key Data key
   * @param value Any data value
   */
  static void SetTestData(const std::string& key, std::any value);

  /**
   * @brief Retrieve test data
   * @param key Data key
   * @return Stored value, or empty any if not found
   */
  static std::any GetTestData(const std::string& key);

  /**
   * @brief Check if specific event occurred
   * @param event_name Event name
   * @return True if event was triggered
   */
  static bool EventOccurred(const std::string& event_name);

  /**
   * @brief Get count of how many times event occurred
   * @param event_name Event name
   * @return Number of times event was triggered
   */
  static int GetEventCount(const std::string& event_name);

  /**
   * @brief Clear all recorded events (for scenario setup)
   */
  static void ClearEvents();

  /**
   * @brief Reset all test state (called between scenarios)
   */
  static void Reset();

  // ============================================================
  // Listener Registration - for step definitions
  // ============================================================

  using EventCallback = std::function<void(const std::string&)>;

  /**
   * @brief Register callback for specific event type
   * @param event_type Event type to listen for
   * @param callback Callback to invoke
   */
  static void RegisterEventListener(const std::string& event_type, EventCallback callback);

 private:
  TestHooks() = default;
  ~TestHooks() = default;

  void RecordEvent(const std::string& event_name);

  std::mutex m_mutex;
  std::map<std::string, int> m_event_counts;
  std::map<std::string, std::any> m_test_data;
  std::map<std::string, std::vector<EventCallback>> m_listeners;
};

// Helper macros for common patterns
#define TEST_HOOK(hook_call) test::TestHooks::hook_call
#define TEST_HOOK_EVENT(event_name) test::TestHooks::RecordEvent(event_name)

} // namespace test

#else

// No-op macros when test hooks are disabled
#define TEST_HOOK(hook_call) ((void)0)
#define TEST_HOOK_EVENT(event_name) ((void)0)

#endif // ENABLE_TEST_HOOKS
