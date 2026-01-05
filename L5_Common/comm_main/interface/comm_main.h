// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_main.h
 * @brief Common layer initialization and startup orchestration
 * @details Coordinates initialization of all L5_Common modules (terminate, logging, etc.)
 */
#pragma once

#include <cstdint>
#include <system_error>

namespace comm {

/**
 * @brief Error codes for Common layer initialization
 */
enum class InitError : std::uint8_t {
  SUCCESS = 0,              ///< Operation completed successfully
  MODULE_INIT_FAILED = 1,     ///< Module initialization failed
  EXCEPTION_THROWN = 2,      ///< Exception was thrown during operation
};

/**
 * @brief Error category for Common layer initialization errors
 */
class InitErrorCategory : public std::error_category {
 public:
  [[nodiscard]] const char* name() const noexcept override { return "comm_init"; }
  [[nodiscard]] std::string message(int ev) const override;
};

/**
 * @brief Get the singleton instance of InitErrorCategory
 */
[[nodiscard]] const std::error_category& getInitErrorCategory() noexcept;

/**
 * @brief Helper function to create std::error_code from InitError
 */
[[nodiscard]] inline std::error_code makeErrorCode(InitError e) noexcept {
  return {static_cast<int>(e), getInitErrorCategory()};
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
  [[nodiscard]] static Main& instance() {
    static Main inst;
    return inst;
  }

  /**
   * @brief Initialize all Common layer (L5) modules
   * @details Starts termination handler and prepares common infrastructure
   * @param argc Command-line argument count (reserved for future use)
   * @param argv Command-line argument values (reserved for future use)
   * @return std::error_code - empty on success, error code on failure
   * @note Currently argc/argv are unused but reserved for future configuration
   */
  [[nodiscard]] static std::error_code init(int argc, const char* argv[]);

  /**
   * @brief Deinitialize all Common layer (L5) modules
   * @details Performs graceful shutdown of common infrastructure
   * @return std::error_code - empty on success, error code on failure
   * @note Should be called last, after all higher layers are deinitialized
   */
  [[nodiscard]] static std::error_code deinit();

  // Delete copy constructor and assignment operator (singleton pattern)
  Main(const Main&) = delete;
  Main& operator=(const Main&) = delete;
};

}  // namespace comm
