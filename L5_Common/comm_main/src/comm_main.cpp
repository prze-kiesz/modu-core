// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include "comm_main.h"

#include <glog/logging.h>
#include "comm_terminate.h"

namespace comm {

Main::Main() = default;

code_t Main::Init(int  /*argc*/, const char*  /*argv*/[]) { // NOLINT
  code_t ret_code = OK;

  // Initialize all modules from driver layer
  // Initialization of config module
  //   ret_code = Config::instance().init(argc, argv);
  //   if (ret_code != OK) {
  //     LOG(ERROR) << "Problem with initialization of 'Config::instance().init(argc, argv)': " << ret_code;
  //   }

  ret_code = Terminate::Instance().Start();
  if (ret_code != OK) {
    LOG(ERROR) << "Problem with starting: Terminate::instance().start()': " << ret_code;
  }

  //
  return ret_code;
}

}  // namespace comm
