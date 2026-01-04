// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file infr_main.h
 * @brief Infrastructure layer initialization and startup orchestration
 * @details Coordinates initialization of all L4_Infrastructure modules
 */
#pragma once

#include <system_error>

namespace infr {

/**
 * @brief Error codes for Infrastructure layer initialization
 */
enum class InitError {
  Success = 0,              ///< Operation completed successfully
  ModuleInitFailed = 1,     ///< Module initialization failed
  ExceptionThrown = 2,      ///< Exception was thrown during operation
};

/**
 * @brief Error category for Infrastructure layer initialization errors
 */
class InitErrorCategory : public std::error_category {
 public:
  const char* name() const noexcept override { return "infr_init"; }
  std::string message(int ev) const override;
};

/**
 * @brief Get the singleton instance of InitErrorCategory
 */
const std::error_category& get_init_error_category() noexcept;

/**
 * @brief Helper function to create std::error_code from InitError
 */
inline std::error_code make_error_code(InitError e) noexcept {
  return {static_cast<int>(e), get_init_error_category()};
}

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
   * @brief Initialize all Infrastructure layer (L4) modules
   * @details Starts low-level infrastructure (networking, messaging, hardware access)
   * @param argc Command-line argument count (reserved for future use)
   * @param argv Command-line argument values (reserved for future use)
   * @return std::error_code - empty on success, error code on failure
   * @note Currently argc/argv are unused but reserved for future configuration
   */
  static std::error_code Init(int argc, const char* argv[]);

  /**
   * @brief Deinitialize all Infrastructure layer (L4) modules
   * @details Performs graceful shutdown of infrastructure components
   * @return std::error_code - empty on success, error code on failure
   * @note Should be called before Common layer (L5) deinitialization
   */
  static std::error_code Deinit();

  // Delete copy constructor and assignment operator (singleton pattern)
  Main(const Main&) = delete;
  Main& operator=(const Main&) = delete;
};

}  // namespace infr
