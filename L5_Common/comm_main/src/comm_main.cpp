// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_main.cpp
 * @brief Implementation of Common layer initialization orchestrator
 * @details Coordinates startup of shared infrastructure modules (logging, signals, etc.)
 */

#include "comm_main.h"

#include <glog/logging.h>
#include "comm_terminate.h"

namespace comm {

// Default constructor - no initialization required (modules initialized in Init())
Main::Main() = default;

code_t Main::Init(int  /*argc*/, const char*  /*argv*/[]) {  // NOLINT
  code_t ret_code = code_t::OK;

  // TODO: Future expansion - use argc/argv for configuration file path or command-line options
  // TODO: Add Config module initialization when implemented

  // Initialize graceful shutdown handler (SIGINT, SIGTERM, SIGQUIT)
  ret_code = Terminate::Instance().Start();
  if (ret_code != code_t::OK) {
    LOG(ERROR) << "Failed to start Terminate::Instance().Start(): " << static_cast<int>(ret_code);
    return ret_code;
  }

  LOG(INFO) << "Common layer (L5) initialization completed successfully";
  return ret_code;
}

}  // namespace comm
