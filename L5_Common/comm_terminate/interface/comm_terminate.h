// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#pragma once

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <semaphore>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

namespace comm {

/**
 * @brief Error codes for Terminate module operations
 */
enum class TerminateError {
  Success = 0,              ///< Operation completed successfully
  SignalMaskFailed = 1,     ///< Failed to block signals with sigprocmask()
  ThreadCreationFailed = 2, ///< Failed to create signal handler thread
  SignalWaitFailed = 3,     ///< sigwait() failed to receive signal
};

/**
 * @brief Error category for Terminate module errors
 */
class TerminateErrorCategory : public std::error_category {
 public:
  const char* name() const noexcept override { return "comm_terminate"; }
  std::string message(int error_value) const override;
};

/**
 * @brief Get the singleton instance of TerminateErrorCategory
 */
const std::error_category& get_terminate_error_category() noexcept;

/**
 * @brief Helper function to create std::error_code from TerminateError
 */
inline std::error_code make_error_code(TerminateError err) noexcept {
  return {static_cast<int>(err), get_terminate_error_category()};
}

class Terminate {
 private:
  /// Worker thread that waits for termination signals (SIGINT, SIGTERM, SIGQUIT, SIGHUP)
  std::unique_ptr<std::thread> m_signal_wait;
  
  /// Event processing thread that invokes config reload listeners
  std::unique_ptr<std::thread> m_event_processor;
  
  /// Binary semaphore for synchronization between signal handler and main thread (C++20)
  std::binary_semaphore m_terminate;
  
  /// Delay in milliseconds before final termination (set once, read in destructor)
  uint32_t m_wait_ms;

  /// Set of signals to handle (SIGINT, SIGTERM, SIGQUIT, SIGHUP)
  sigset_t m_waited_signals;
  
  /// Human-readable termination reason (synchronized via m_terminate semaphore)
  std::string m_terminate_reason;

  /// Event types for internal processing
  enum class EventType {
    ConfigReload,  ///< SIGHUP received - reload configuration
    Shutdown       ///< Shutdown event processor thread
  };

  /// Event queue for async processing (signal handler -> event processor thread)
  std::queue<EventType> m_event_queue;
  
  /// Mutex protecting event queue
  std::mutex m_event_mutex;
  
  /// Condition variable for event queue notifications
  std::condition_variable m_event_cv;
  
  /// Flag to stop event processor thread (protected by m_event_mutex)
  bool m_stop_event_processor{false};

  /// Registered callbacks for configuration reload notifications
  std::vector<std::function<void()>> m_config_reload_listeners;
  
  /// Mutex protecting config reload listeners vector
  std::mutex m_listeners_mutex;
  
  /// Flag to track if first SIGINT was received (for double Ctrl-C handling)
  std::atomic<bool> m_first_sigint_received{false};

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
   * @brief Waits for termination signal (SIGINT/SIGTERM/SIGQUIT/SIGHUP) in dedicated thread
   * @details Notifies systemd when ready, blocks on sigwait(), handles SIGHUP for config reload
   * @note Runs in m_signal_wait thread, not main thread
   */
  void WaitForTerminateSignal();

  /**
   * @brief Event processor thread - handles config reload notifications
   * @details Processes events from queue and invokes registered listeners
   * @note Runs in m_event_processor thread, separate from signal handler
   */
  void ProcessEvents();

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
   * @details Blocks SIGINT/SIGTERM/SIGQUIT/SIGHUP in main thread and creates dedicated signal handler thread
   * @return std::error_code - empty on success, error code on failure
   * @note Must be called before WaitForTermination()
   */
  std::error_code Start();

  /**
   * @brief Register a callback to be invoked when configuration is reloaded (SIGHUP)
   * @param callback Function to call when SIGHUP signal is received
   * @details Callback is invoked from event processor thread, not signal handler
   * @note Thread-safe, can be called from any thread
   * @note If registered during active reload processing, the listener may not be
   *       invoked for that specific reload event, but will be called for subsequent reloads
   * 
   * Example usage:
   * @code
   *   Terminate::Instance().RegisterConfigReloadListener([]() {
   *     LOG(INFO) << "Config reloaded, updating module state";
   *     MyModule::instance().reload_settings();
   *   });
   * @endcode
   */
  void RegisterConfigReloadListener(std::function<void()> callback);

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