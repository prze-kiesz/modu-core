// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include "test_hooks.h"

#ifdef ENABLE_TEST_HOOKS

#include <glog/logging.h>

namespace test {

TestHooks& TestHooks::Instance() {
  static TestHooks instance;
  return instance;
}

void TestHooks::OnConfigReloadStarted() {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.RecordEvent("config_reload_started");
  LOG(INFO) << "[TEST_HOOK] Config reload started";
  
  // Notify listeners
  if (instance.m_listeners.count("config_reload_started")) {
    for (auto& listener : instance.m_listeners["config_reload_started"]) {
      listener("config_reload_started");
    }
  }
}

void TestHooks::OnConfigReloadCompleted(const std::string& section) {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.RecordEvent("config_reload_completed");
  instance.RecordEvent("config_reload_completed:" + section);
  instance.m_test_data["last_reloaded_section"] = section;
  LOG(INFO) << "[TEST_HOOK] Config reload completed for section: " << section;
  
  // Notify listeners
  if (instance.m_listeners.count("config_reload_completed")) {
    for (auto& listener : instance.m_listeners["config_reload_completed"]) {
      listener(section);
    }
  }
}

void TestHooks::OnConfigReloadFailed(const std::string& error) {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.RecordEvent("config_reload_failed");
  instance.m_test_data["last_reload_error"] = error;
  LOG(WARNING) << "[TEST_HOOK] Config reload failed: " << error;
  
  // Notify listeners
  if (instance.m_listeners.count("config_reload_failed")) {
    for (auto& listener : instance.m_listeners["config_reload_failed"]) {
      listener(error);
    }
  }
}

void TestHooks::OnTerminationSignalReceived(int signal) {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.RecordEvent("termination_signal_received");
  instance.m_test_data["termination_signal"] = signal;
  LOG(INFO) << "[TEST_HOOK] Termination signal received: " << signal;
  
  // Notify listeners
  if (instance.m_listeners.count("termination_signal_received")) {
    for (auto& listener : instance.m_listeners["termination_signal_received"]) {
      listener(std::to_string(signal));
    }
  }
}

void TestHooks::OnShutdownStage(const std::string& stage) {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.RecordEvent("shutdown_stage:" + stage);
  instance.m_test_data["last_shutdown_stage"] = stage;
  LOG(INFO) << "[TEST_HOOK] Shutdown stage: " << stage;
  
  // Notify listeners
  if (instance.m_listeners.count("shutdown_stage")) {
    for (auto& listener : instance.m_listeners["shutdown_stage"]) {
      listener(stage);
    }
  }
}

void TestHooks::OnInfrConfigInitialized() {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.RecordEvent("infr_config_initialized");
  LOG(INFO) << "[TEST_HOOK] InfrConfig initialized";
  
  // Notify listeners
  if (instance.m_listeners.count("infr_config_initialized")) {
    for (auto& listener : instance.m_listeners["infr_config_initialized"]) {
      listener("infr_config");
    }
  }
}

void TestHooks::OnModuleInitialized(const std::string& module_name) {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.RecordEvent("module_initialized:" + module_name);
  LOG(INFO) << "[TEST_HOOK] Module initialized: " << module_name;
  
  // Notify listeners
  if (instance.m_listeners.count("module_initialized")) {
    for (auto& listener : instance.m_listeners["module_initialized"]) {
      listener(module_name);
    }
  }
}

void TestHooks::SetTestData(const std::string& key, std::any value) {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.m_test_data[key] = std::move(value);
}

std::any TestHooks::GetTestData(const std::string& key) {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  if (instance.m_test_data.count(key)) {
    return instance.m_test_data[key];
  }
  return std::any{};
}

bool TestHooks::EventOccurred(const std::string& event_name) {
  return GetEventCount(event_name) > 0;
}

int TestHooks::GetEventCount(const std::string& event_name) {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  if (instance.m_event_counts.count(event_name)) {
    return instance.m_event_counts[event_name];
  }
  return 0;
}

void TestHooks::ClearEvents() {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.m_event_counts.clear();
  LOG(INFO) << "[TEST_HOOK] Cleared all events";
}

void TestHooks::Reset() {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.m_event_counts.clear();
  instance.m_test_data.clear();
  instance.m_listeners.clear();
  LOG(INFO) << "[TEST_HOOK] Reset all test state";
}

void TestHooks::RegisterEventListener(const std::string& event_type, EventCallback callback) {
  auto& instance = Instance();
  std::lock_guard<std::mutex> lock(instance.m_mutex);
  instance.m_listeners[event_type].push_back(std::move(callback));
  LOG(INFO) << "[TEST_HOOK] Registered listener for event: " << event_type;
}

void TestHooks::RecordEvent(const std::string& event_name) {
  m_event_counts[event_name]++;
}

} // namespace test

#endif // ENABLE_TEST_HOOKS
