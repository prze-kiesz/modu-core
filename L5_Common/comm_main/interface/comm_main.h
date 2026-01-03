// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_main.h
 * @brief Main file for Drivers layer startup, initialization and work
 *
 *
 */
#pragma once

namespace comm {

/**
 * @brief Enum with commonly used return code values in the project
 */
typedef enum {
  OK,
  ERROR,
  EXCEPTION_THROWN,
} code_t;

class Main {
 private:
  Main();

 public:
  /**
   * @brief Returns instance of Main class singleton
   * see: Meyers' Singleton in C++
   *
   * @return Main& instance of Config
   */
  static Main& Instance() {
    static Main instance;
    return instance;
  }

  /**
   * @brief Function should be called to initialize porting layer modules
   *
   * @param argc
   * @param argv
   * @return code_t
   */
  static code_t Init(int argc, const char* argv[]);
};

}  // namespace drv
