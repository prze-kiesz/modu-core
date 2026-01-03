// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include <glog/logging.h>
#include "comm_main.h"
#include "comm_terminate.h"

int main(const int argc, const char *argv[]) {

  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 0;

  // Initialize Google’s logging library.
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  
  LOG(INFO) << "Starting application";

  // Initialize all layers starting from the lowest one.
  
  auto ret_code = comm::Main::Init(argc, argv);
  if (ret_code != comm::code_t::OK) {
    LOG(ERROR) << "Initialization failed with error code: " << static_cast<int>(ret_code);
    return 1;
  }
      
  // wait for application termination signal
  LOG(INFO) << "Waiting for application termination";
  auto term_reason = comm::Terminate::Instance().WaitForTermination();

  LOG(INFO) << "Application is shutting down, reason: " << term_reason;

  // Shutdown Google's logging library
  google::ShutdownGoogleLogging();

  return 0;
}
