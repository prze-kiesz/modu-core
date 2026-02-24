// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file cucumber_hooks.cpp
 * @brief Cucumber Before/After scenario hooks
 */

#include <cucumber-cpp/autodetect.hpp>
#include <glog/logging.h>

#include "test_hooks.h"
#include "test_fixture.h"

using cucumber::ScenarioScope;

/**
 * @brief Before hook - runs before each scenario
 */
BEFORE() {
  LOG(INFO) << "========================================";
  LOG(INFO) << "Starting new scenario";
  LOG(INFO) << "========================================";
  
  // Reset test state
  test::TestHooks::Reset();
  
  // Note: Don't start application here - let scenarios control startup
}

/**
 * @brief After hook - runs after each scenario
 */
AFTER() {
  LOG(INFO) << "========================================";
  LOG(INFO) << "Scenario completed";
  LOG(INFO) << "========================================";
  
  // Ensure application is stopped
  test::TestFixture::Instance().Reset();
  
  // Clean up test hooks
  test::TestHooks::ClearEvents();
}

/**
 * @brief After all tests hook
 */
AFTER_ALL() {
  LOG(INFO) << "========================================";
  LOG(INFO) << "All acceptance tests completed";
  LOG(INFO) << "========================================";
}

/**
 * @brief Before all tests hook
 */
BEFORE_ALL() {
  LOG(INFO) << "========================================";
  LOG(INFO) << "Starting cucumber-cpp acceptance tests";
  LOG(INFO) << "Test hooks are ENABLED";
  LOG(INFO) << "========================================";
  
  // Initialize Google Logging for tests
  google::InitGoogleLogging("cucumber_tests");
  FLAGS_logtostderr = 1;
  FLAGS_minloglevel = 0; // INFO level
}
