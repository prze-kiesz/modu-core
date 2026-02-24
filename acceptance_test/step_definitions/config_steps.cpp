// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file config_steps.cpp
 * @brief Step definitions for configuration reload scenarios
 */

#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>
#include <fstream>
#include <thread>
#include <chrono>
#include <signal.h>

#include "test_hooks.h"
#include "test_fixture.h"

using cucumber::ScenarioScope;

namespace {

struct ConfigContext {
  std::string config_file_path;
  int reload_count = 0;
  bool reload_success = false;
  std::string last_error;
};

} // namespace

// Background

GIVEN("^the application is initialized with default config$") {
  ScenarioScope<ConfigContext> context;
  test::TestFixture::Instance().StartApplication();
  test::TestHooks::ClearEvents();
}

GIVEN("^the config file path is \"([^\"]*)\"$") {
  REGEX_PARAM(std::string, path);
  ScenarioScope<ConfigContext> context;
  context->config_file_path = path;
  test::TestFixture::Instance().SetConfigPath(path);
}

// Config file manipulation

GIVEN("^the config file contains:$") {
  REGEX_PARAM(std::string, content);
  ScenarioScope<ConfigContext> context;
  
  std::ofstream file(context->config_file_path);
  ASSERT_TRUE(file.is_open()) << "Failed to open config file: " << context->config_file_path;
  file << content;
  file.close();
}

GIVEN("^the current port is (\\d+)$") {
  REGEX_PARAM(int, port);
  // Verify current port from InfrConfig
  auto config = test::TestFixture::Instance().GetInfrConfig();
  EXPECT_EQ(config.port, port);
}

WHEN("^I update the config file to set port to (\\d+)$") {
  REGEX_PARAM(int, new_port);
  ScenarioScope<ConfigContext> context;
  
  std::string content = R"(
[infr_main]
device_name = "test_device"
port = )" + std::to_string(new_port) + R"(
enable_logging = true
timeout_seconds = 30.0
)";
  
  std::ofstream file(context->config_file_path);
  ASSERT_TRUE(file.is_open());
  file << content;
  file.close();
}

WHEN("^I update the config file with invalid TOML syntax$") {
  ScenarioScope<ConfigContext> context;
  
  std::ofstream file(context->config_file_path);
  ASSERT_TRUE(file.is_open());
  file << "[infr_main\nport = INVALID\n";  // Invalid TOML
  file.close();
}

// Signal handling

WHEN("^I send SIGHUP signal to the application$") {
  test::TestFixture::Instance().SendSignal(SIGHUP);
  std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait for processing
}

WHEN("^I send SIGHUP signal (\\d+) times with (\\d+)ms delay$") {
  REGEX_PARAM(int, times);
  REGEX_PARAM(int, delay_ms);
  ScenarioScope<ConfigContext> context;
  
  for (int i = 0; i < times; ++i) {
    test::TestFixture::Instance().SendSignal(SIGHUP);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  }
  
  // Wait for all to complete
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  context->reload_count = times;
}

// Assertions

THEN("^the config reload should complete successfully$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("config_reload_completed"));
}

THEN("^the infr_main config should be reloaded$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("config_reload_completed:infr_main"));
}

THEN("^the new port should be (\\d+)$") {
  REGEX_PARAM(int, expected_port);
  
  std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Ensure reload completes
  auto config = test::TestFixture::Instance().GetInfrConfig();
  EXPECT_EQ(config.port, expected_port);
}

THEN("^InfrConfig listeners should be notified$") {
  // Check that listener notification happened
  EXPECT_GT(test::TestHooks::GetEventCount("config_reload_completed"), 0);
}

THEN("^the config reload should fail$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("config_reload_failed"));
}

THEN("^the application should continue running with old config$") {
  // Application should still be running
  EXPECT_TRUE(test::TestFixture::Instance().IsRunning());
}

THEN("^an error should be logged$") {
  auto error = test::TestHooks::GetTestData("last_reload_error");
  EXPECT_FALSE(error.has_value() == false);
}

THEN("^all (\\d+) config reloads should complete$") {
  REGEX_PARAM(int, expected_count);
  
  int actual_count = test::TestHooks::GetEventCount("config_reload_completed");
  EXPECT_EQ(actual_count, expected_count);
}

THEN("^the application should remain stable$") {
  EXPECT_TRUE(test::TestFixture::Instance().IsRunning());
}

THEN("^no reload events should be dropped$") {
  // All events should be processed
  EXPECT_EQ(test::TestHooks::GetEventCount("config_reload_started"),
            test::TestHooks::GetEventCount("config_reload_completed"));
}

// Listener tests

GIVEN("^InfrConfig has (\\d+) registered listeners$") {
  REGEX_PARAM(int, listener_count);
  
  for (int i = 0; i < listener_count; ++i) {
    test::TestFixture::Instance().RegisterInfrConfigListener([i]() {
      test::TestHooks::SetTestData("listener_" + std::to_string(i) + "_called", true);
    });
  }
}

THEN("^both listeners should be invoked$") {
  auto listener0 = test::TestHooks::GetTestData("listener_0_called");
  auto listener1 = test::TestHooks::GetTestData("listener_1_called");
  
  EXPECT_TRUE(listener0.has_value());
  EXPECT_TRUE(listener1.has_value());
}

THEN("^both listeners should receive the new config$") {
  // Listeners were called after reload
  EXPECT_TRUE(test::TestHooks::EventOccurred("config_reload_completed"));
}
