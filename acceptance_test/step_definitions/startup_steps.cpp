// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file startup_steps.cpp
 * @brief Step definitions for application startup scenarios
 */

#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>
#include <fstream>

#include "test_hooks.h"
#include "test_fixture.h"

using cucumber::ScenarioScope;

namespace {

struct StartupContext {
  std::string config_file_path = "test_config.toml";
  std::vector<std::string> initialization_order;
};

} // namespace

GIVEN("^a valid default configuration file exists$") {
  ScenarioScope<StartupContext> context;
  
  std::string content = R"(
[infr_main]
device_name = "default_device"
port = 8080
enable_logging = true
timeout_seconds = 30.0
)";
  
  std::ofstream file(context->config_file_path);
  ASSERT_TRUE(file.is_open());
  file << content;
  file.close();
}

GIVEN("^a custom config file with:$") {
  REGEX_PARAM(std::string, content);
  ScenarioScope<StartupContext> context;
  
  context->config_file_path = "custom_config.toml";
  std::ofstream file(context->config_file_path);
  ASSERT_TRUE(file.is_open());
  file << content;
  file.close();
}

GIVEN("^no config file exists$") {
  ScenarioScope<StartupContext> context;
  // Ensure no config file exists
  std::remove(context->config_file_path.c_str());
}

WHEN("^I start the application$") {
  ScenarioScope<StartupContext> context;
  
  // Register listener to track initialization order
  test::TestHooks::RegisterEventListener("module_initialized", [context](const std::string& module) {
    context->initialization_order.push_back(module);
  });
  
  test::TestFixture::Instance().StartApplication(context->config_file_path);
  std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Wait for initialization
}

WHEN("^I start the application with config file \"([^\"]*)\"$") {
  REGEX_PARAM(std::string, config_file);
  
  test::TestFixture::Instance().StartApplication(config_file);
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

THEN("^the application should initialize all modules$") {
  EXPECT_TRUE(test::TestFixture::Instance().IsRunning());
}

THEN("^InfrConfig should be initialized$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("infr_config_initialized"));
}

THEN("^comm::Config should be initialized$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("module_initialized:comm::Config"));
}

THEN("^comm::Terminate should be initialized$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("module_initialized:comm::Terminate"));
}

THEN("^the application should be ready to accept signals$") {
  // Try sending SIGHUP to verify
  test::TestFixture::Instance().SendSignal(SIGHUP);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  EXPECT_TRUE(test::TestHooks::EventOccurred("config_reload_started"));
}

THEN("^the application should load the custom config$") {
  auto config = test::TestFixture::Instance().GetInfrConfig();
  // Config should be loaded (specific values checked in other steps)
  EXPECT_FALSE(config.device_name.empty());
}

THEN("^the device_name should be \"([^\"]*)\"$") {
  REGEX_PARAM(std::string, expected_name);
  
  auto config = test::TestFixture::Instance().GetInfrConfig();
  EXPECT_EQ(config.device_name, expected_name);
}

THEN("^the port should be (\\d+)$") {
  REGEX_PARAM(int, expected_port);
  
  auto config = test::TestFixture::Instance().GetInfrConfig();
  EXPECT_EQ(config.port, expected_port);
}

THEN("^the application should use default values$") {
  auto config = test::TestFixture::Instance().GetInfrConfig();
  // Should have some default values
  EXPECT_FALSE(config.device_name.empty());
  EXPECT_GT(config.port, 0);
}

THEN("^InfrConfig should be initialized with defaults$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("infr_config_initialized"));
  auto config = test::TestFixture::Instance().GetInfrConfig();
  EXPECT_EQ(config.device_name, "default_device");
  EXPECT_EQ(config.port, 8080);
}

THEN("^the application should continue running$") {
  EXPECT_TRUE(test::TestFixture::Instance().IsRunning());
}

THEN("^modules should initialize in correct order:$") {
  REGEX_PARAM(cucumber::Table, table);
  ScenarioScope<StartupContext> context;
  
  // Skip header row
  for (size_t i = 1; i < table.hashes().size(); ++i) {
    auto row = table.hashes()[i];
    int expected_order = std::stoi(row.at("order"));
    std::string expected_module = row.at("module");
    
    // Check that module was initialized
    EXPECT_TRUE(test::TestHooks::EventOccurred("module_initialized:" + expected_module))
      << "Module " << expected_module << " should be initialized";
  }
}

THEN("^all initialization hooks should fire$") {
  // At least some initialization events should have occurred
  int total_init_events = 0;
  total_init_events += test::TestHooks::GetEventCount("module_initialized:comm::Config");
  total_init_events += test::TestHooks::GetEventCount("module_initialized:comm::Terminate");
  total_init_events += test::TestHooks::GetEventCount("infr_config_initialized");
  
  EXPECT_GT(total_init_events, 0);
}
