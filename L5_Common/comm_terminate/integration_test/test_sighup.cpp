// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file test_sighup.cpp
 * @brief Integration test for SIGHUP config reload with listener pattern
 * @details Demonstrates how to register config reload listeners that are
 *          invoked from a separate event processing thread when SIGHUP is received
 * 
 * Usage:
 *   1. Run the program: ./test_sighup
 *   2. In another terminal, send SIGHUP: kill -SIGHUP <pid>
 *   3. Verify the listener is called
 *   4. Send SIGTERM to terminate: kill -SIGTERM <pid>
 */

#include <glog/logging.h>
#include <unistd.h>

#include <iostream>
#include <atomic>

#include "comm_terminate.h"

using namespace comm;

// Global counter to track config reloads
std::atomic<int> reload_count{0};

int main(int argc, char* argv[]) {
  // Initialize Google logging
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 0;  // Show all logs

  LOG(INFO) << "SIGHUP Integration Test Started";
  LOG(INFO) << "Process PID: " << getpid();
  LOG(INFO) << "Send SIGHUP to this process to trigger config reload";
  LOG(INFO) << "Send SIGTERM to terminate gracefully";

  // Register config reload listener
  Terminate::Instance().RegisterConfigReloadListener([]() {
    int count = ++reload_count;
    LOG(INFO) << ">>> Config reload listener invoked! Reload #" << count;
    LOG(INFO) << ">>> This callback is running in event processor thread";
    
    // Simulate config reload work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    LOG(INFO) << ">>> Config reload completed";
  });

  // Register a second listener to demonstrate multiple listeners
  Terminate::Instance().RegisterConfigReloadListener([]() {
    LOG(INFO) << ">>> Second listener: Updating module state";
  });

  LOG(INFO) << "Registered 2 config reload listeners";

  // Start the terminate handler (spawns signal and event processor threads)
  auto error = Terminate::Instance().Start();
  if (error) {
    LOG(ERROR) << "Failed to start terminate handler: " << error.message();
    return 1;
  }

  LOG(INFO) << "Terminate handler started successfully";
  LOG(INFO) << "Waiting for signals... (use Ctrl-C or kill -SIGTERM to exit)";

  // Wait for termination signal
  std::string reason = Terminate::Instance().WaitForTermination();
  
  LOG(INFO) << "Application terminated: " << reason;
  LOG(INFO) << "Total config reloads: " << reload_count.load();

  google::ShutdownGoogleLogging();
  return 0;
}
