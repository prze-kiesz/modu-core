// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include <iostream>
#include <glog/logging.h>

int main(const int argc, const char *argv[]) {

  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 0;

  // Initialize Google’s logging library.
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  
  LOG(INFO) << "Starting application with " << argc << " arguments.";
  return 0;
}
