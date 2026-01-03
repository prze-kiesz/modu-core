// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_main.h
 * @brief Common layer initialization and startup orchestration
 * @details Coordinates initialization of all L5_Common modules (terminate, logging, etc.)
 */
#pragma once

namespace comm {

/**
 * @brief Status codes for initialization and operation results
 * @note Using enum class for type safety (C++11)
 */
enum class code_t {
  OK,                 ///< Operation completed successfully
  ERROR,              ///< Generic error occurred
  EXCEPTION_THROWN,   ///< Exception was thrown during operation
};

class Main {
 private:
  /**
   * @brief Private constructor for singleton pattern
   * @note Default constructor - no initialization needed in constructor
   */
  Main();

 public:
  /**
   * @brief Returns singleton instance of Main class
   * @details Thread-safe initialization using Meyers' Singleton pattern
   * @return Reference to the single Main instance
   */
  static Main& Instance() {
    static Main instance;
    return instance;
  }

  /**
   * @brief Initialize all Common layer (L5) modules
   * @details Starts termination handler and prepares common infrastructure
   * @param argc Command-line argument count (reserved for future use)
   * @param argv Command-line argument values (reserved for future use)
   * @return code_t::OK on success, code_t::ERROR on failure
   * @note Currently argc/argv are unused but reserved for future configuration
   */
  static code_t Init(int argc, const char* argv[]);
};

}  // namespace comm
