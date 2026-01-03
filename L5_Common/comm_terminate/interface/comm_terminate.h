// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#pragma once

#include <csignal>
#include <memory>
#include <semaphore>
#include <string>
#include <thread>

#include "comm_main.h"

namespace comm {

class Terminate {
 private:
  /// Worker thread that waits for termination signals (SIGINT, SIGTERM, SIGQUIT)
  std::unique_ptr<std::thread> m_signal_wait;
  /// Binary semaphore for synchronization between signal handler and main thread (C++20)
  std::binary_semaphore m_terminate;
  /// Delay in milliseconds before final termination (set once, read in destructor)
  uint32_t m_wait_ms;

  /// Set of signals to handle (SIGINT, SIGTERM, SIGQUIT)
  sigset_t m_waited_signals;
  /// Human-readable termination reason (synchronized via m_terminate semaphore)
  std::string m_terminate_reason;

  /**
   * @brief Initializes signal set and semaphore for graceful shutdown handling
   * @note Private constructor - Singleton pattern (Meyers' Singleton)
   */
  Terminate();
  
  /**
   * @brief Destroy the Terminate object
   * @note Singleton instance never destroyed during normal program execution
   */
  ~Terminate();

  /**
   * @brief Waits for termination signal (SIGINT/SIGTERM/SIGQUIT) in dedicated thread
   * @details Notifies systemd when ready, blocks on sigwait(), then signals termination
   * @note Runs in m_signal_wait thread, not main thread
   */
  void WaitForTerminateSignal();

 public:
  /**
   * @brief Returns singleton instance of Terminate class
   * @details Thread-safe initialization using Meyers' Singleton pattern
   * @return Reference to the single Terminate instance
   */
  static Terminate& Instance() {
    static Terminate instance;
    return instance;
  }

  /**
   * @brief Starts the termination handler by blocking signals and spawning worker thread
   * @details Blocks SIGINT/SIGTERM/SIGQUIT in main thread and creates dedicated signal handler thread
   * @return code_t Status code (currently always returns OK)
   * @note Must be called before WaitForTermination()
   */
  code_t Start();

  /**
   * @brief Programmatically triggers application termination (alternative to external signals)
   * @details Signals the worker thread to exit and triggers WaitForTermination() to return
   * @param milis_to_wait Delay in milliseconds before allowing final shutdown (default: 0)
   * @note Useful for graceful shutdown from application logic (e.g., fatal error, admin command)
   */
  void TerminateApp(uint32_t milis_to_wait = 0);

  /**
   * @brief Blocks until termination is requested (via signal or TerminateApp())
   * @details Call this in main() after Start() to keep application running until shutdown
   * @return Human-readable termination reason (e.g., "Termination request", "Interactive attention signal")
   * @note This is a blocking call - returns only when shutdown is triggered
   */
  std::string WaitForTermination();
};

}  // namespace comm