// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include <glog/logging.h>
#include "comm_main.h"
#include "comm_terminate.h"
#include "infr_main.h"

int main(const int argc, const char *argv[]) {

  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 0;

  // Initialize Google’s logging library.
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  
  LOG(INFO) << "Starting application";

  // Initialize all layers starting from the lowest one.
  
  // L5 - Common layer (logging, signals, shared utilities) - LOWEST LAYER
  auto ret_code = comm::Main::Init(argc, argv);
  if (ret_code) {
    LOG(ERROR) << "L5 Common initialization failed: " << ret_code.message();
    return 1;
  }

  // L4 - Infrastructure layer (networking, messaging, hardware access)
  ret_code = infr::Main::Init(argc, argv);
  if (ret_code) {
    LOG(ERROR) << "L4 Infrastructure initialization failed: " << ret_code.message();
    return 1;
  }
      
  // wait for application termination signal
  LOG(INFO) << "Waiting for application termination";
  auto term_reason = comm::Terminate::Instance().WaitForTermination();

  LOG(INFO) << "Application is shutting down, reason: " << term_reason;

  // Deinitialize all layers in reverse order (highest to lowest)
  
  // L4 - Infrastructure layer
  ret_code = infr::Main::Deinit();
  if (ret_code) {
    LOG(ERROR) << "L4 Infrastructure deinitialization failed: " << ret_code.message();
  }

  // L5 - Common layer (last, as it provides base services)
  ret_code = comm::Main::Deinit();
  if (ret_code) {
    LOG(ERROR) << "L5 Common deinitialization failed: " << ret_code.message();
  }

  // Shutdown Google's logging library
  google::ShutdownGoogleLogging();

  return 0;
}
