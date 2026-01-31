// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file test_double_sigint.cpp
 * @brief Integration test for double SIGINT (Ctrl-C) handling
 * @details Tests that:
 *          1. First SIGINT initiates graceful shutdown
 *          2. Second SIGINT immediately terminates the process
 * 
 * Usage:
 *   1. Run the program: ./test_double_sigint
 *   2. In another terminal, send first SIGINT: kill -SIGINT <pid>
 *   3. Verify graceful shutdown starts (with delay)
 *   4. Send second SIGINT: kill -SIGINT <pid>
 *   5. Verify immediate termination (process dies instantly)
 */

#include <glog/logging.h>
#include <unistd.h>

#include <iostream>
#include <atomic>

#include "comm_terminate.h"

using namespace comm;

int main(int argc, char* argv[]) {
  // Initialize Google logging
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 0;  // Show all logs

  LOG(INFO) << "Double SIGINT Integration Test Started";
  LOG(INFO) << "Process PID: " << getpid();
  LOG(INFO) << "First Ctrl-C will start graceful shutdown with 5 second delay";
  LOG(INFO) << "Second Ctrl-C will immediately terminate the process";

  // Start the terminate handler
  auto error = Terminate::Instance().Start();
  if (error) {
    LOG(ERROR) << "Failed to start terminate handler: " << error.message();
    return 1;
  }

  LOG(INFO) << "Terminate handler started successfully";
  LOG(INFO) << "Waiting for SIGINT... (press Ctrl-C or kill -SIGINT to test)";

  // Wait for termination signal with 5 second delay
  // This gives time to test the second SIGINT
  std::string reason = Terminate::Instance().WaitForTermination();
  
  LOG(INFO) << "Graceful shutdown initiated: " << reason;
  LOG(INFO) << "Simulating cleanup work for 5 seconds...";
  LOG(INFO) << "Press Ctrl-C again NOW to test immediate termination!";
  
  // Simulate long-running cleanup that can be interrupted
  for (int i = 5; i > 0; --i) {
    LOG(INFO) << "Cleanup in progress... " << i << " seconds remaining";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  LOG(INFO) << "Graceful shutdown completed normally";
  LOG(INFO) << "If you see this message, second SIGINT was NOT tested";

  google::ShutdownGoogleLogging();
  return 0;
}
