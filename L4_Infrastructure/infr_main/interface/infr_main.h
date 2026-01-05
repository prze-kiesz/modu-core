// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file infr_main.h
 * @brief Infrastructure layer initialization and startup orchestration
 * @details Coordinates initialization of all L4_Infrastructure modules
 */
#pragma once

#include <cstdint>
#include <system_error>

namespace infr {

/**
 * @brief Error codes for Infrastructure layer initialization
 */
enum class InitError : std::uint8_t {
  SUCCESS = 0,              ///< Operation completed successfully
  MODULE_INIT_FAILED = 1,     ///< Module initialization failed
  EXCEPTION_THROWN = 2,      ///< Exception was thrown during operation
};

/**
 * @brief Error category for Infrastructure layer initialization errors
 */
class InitErrorCategory : public std::error_category {
 public:
  [[nodiscard]] const char* name() const noexcept override { return "infr_init"; }
  [[nodiscard]] std::string message(int error_value) const override;
};

/**
 * @brief Get the singleton instance of InitErrorCategory
 */
[[nodiscard]] const std::error_category& getInitErrorCategory() noexcept;

/**
 * @brief Helper function to create std::error_code from InitError
 */
[[nodiscard]] inline std::error_code makeErrorCode(InitError err) noexcept {
  return {static_cast<int>(err), getInitErrorCategory()};
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
   * @brief Initialize all Infrastructure layer (L4) modules
   * @details Starts low-level infrastructure (networking, messaging, hardware access)
   * @param argc Command-line argument count (reserved for future use)
   * @param argv Command-line argument values (reserved for future use)
   * @return std::error_code - empty on success, error code on failure
   * @note Currently argc/argv are unused but reserved for future configuration
   */
  [[nodiscard]] static std::error_code init(int argc, const char* argv[]);

  /**
   * @brief Deinitialize all Infrastructure layer (L4) modules
   * @details Performs graceful shutdown of infrastructure components
   * @return std::error_code - empty on success, error code on failure
   * @note Should be called before Common layer (L5) deinitialization
   */
  [[nodiscard]] static std::error_code deinit();

  // Delete copy and move constructors/operators (singleton pattern)
  Main(const Main&) = delete;
  Main& operator=(const Main&) = delete;
  Main(Main&&) = delete;
  Main& operator=(Main&&) = delete;
  ~Main() = default;
};

}  // namespace infr
