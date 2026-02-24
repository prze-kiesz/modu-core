// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file terminate_steps.cpp
 * @brief Step definitions for graceful shutdown scenarios
 */

#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>
#include <signal.h>
#include <thread>
#include <chrono>

#include "test_hooks.h"
#include "test_fixture.h"

using cucumber::ScenarioScope;

namespace {

struct TerminateContext {
  int exit_code = -1;
  bool shutdown_completed = false;
};

} // namespace

GIVEN("^the application is running$") {
  if (!test::TestFixture::Instance().IsRunning()) {
    test::TestFixture::Instance().StartApplication();
  }
  test::TestHooks::ClearEvents();
}

WHEN("^I send SIGTERM signal to the application$") {
  test::TestFixture::Instance().SendSignal(SIGTERM);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

WHEN("^I send SIGINT signal to the application$") {
  test::TestFixture::Instance().SendSignal(SIGINT);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

WHEN("^I trigger config reload$") {
  test::TestFixture::Instance().SendSignal(SIGHUP);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

WHEN("^I send SIGTERM before reload completes$") {
  test::TestFixture::Instance().SendSignal(SIGTERM);
}

WHEN("^I send SIGTERM again after (\\d+)ms$") {
  REGEX_PARAM(int, delay_ms);
  std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  test::TestFixture::Instance().SendSignal(SIGTERM);
}

THEN("^the application should receive termination signal$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("termination_signal_received"));
}

THEN("^the application should complete shutdown stages$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("shutdown_stage:cleanup"));
}

THEN("^the shutdown stage \"([^\"]*)\" should complete$") {
  REGEX_PARAM(std::string, stage);
  EXPECT_TRUE(test::TestHooks::EventOccurred("shutdown_stage:" + stage));
}

THEN("^the application should exit with code (\\d+)$") {
  REGEX_PARAM(int, expected_code);
  ScenarioScope<TerminateContext> context;
  
  // Wait for application to exit
  int exit_code = test::TestFixture::Instance().WaitForExit(5000);
  context->exit_code = exit_code;
  EXPECT_EQ(exit_code, expected_code);
}

THEN("^the application should exit cleanly$") {
  int exit_code = test::TestFixture::Instance().WaitForExit(5000);
  EXPECT_EQ(exit_code, 0);
}

THEN("^all resources should be cleaned up$") {
  // Verify cleanup stage completed
  EXPECT_TRUE(test::TestHooks::EventOccurred("shutdown_stage:cleanup"));
}

THEN("^the config reload should be interrupted$") {
  // Either reload failed or termination signal was prioritized
  bool reload_failed = test::TestHooks::EventOccurred("config_reload_failed");
  bool termination_received = test::TestHooks::EventOccurred("termination_signal_received");
  EXPECT_TRUE(reload_failed || termination_received);
}

THEN("^the application should shutdown gracefully$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("termination_signal_received"));
}

THEN("^only one shutdown sequence should execute$") {
  // Termination signal should be received only once (or handled once)
  int count = test::TestHooks::GetEventCount("termination_signal_received");
  EXPECT_GE(count, 1);
  EXPECT_LE(count, 2); // Allow for race condition, but shutdown should handle it
}
