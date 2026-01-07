// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_terminate.cpp
 * @brief Graceful shutdown handler with signal management and systemd integration
 * @details Provides cross-thread signal handling for SIGINT, SIGTERM, and SIGQUIT
 *          with support for systemd notify protocol and configurable shutdown delays
 */

#include "comm_terminate.h"

#include <glog/logging.h>
#include <signal.h>
#include <systemd/sd-daemon.h>

#include <csignal>
#include <exception>
#include <string>

namespace {
/**
 * @brief Converts POSIX signal number to human-readable description
 * @param signal POSIX signal number (e.g., SIGINT, SIGTERM)
 * @return Human-readable signal description
 * @note Internal utility function for logging and error reporting
 */
std::string GetSignalName(int signal) {
  switch (signal) {
    case SIGINT:
      return "Interactive attention signal";
    case SIGILL:
      return "Illegal instruction";
    case SIGABRT:
      return "Abnormal termination";
    case SIGFPE:
      return "Erroneous arithmetic operation";
    case SIGSEGV:
      return "Invalid access to storage";
    case SIGTERM:
      return "Termination request";

    /* Historical signals specified by POSIX. */
    case SIGHUP:
      return "Hangup";
    case SIGQUIT:
      return "Quit";
    case SIGTRAP:
      return "Trace/breakpoint trap";
    case SIGKILL:
      return "Killed";
    case SIGPIPE:
      return "Broken pipe";
    case SIGALRM:
      return "Alarm clock";
    default:
      return "Unknown signal " + std::to_string(signal);
  }
}
}  // namespace

namespace comm {

// Error category implementation
std::string TerminateErrorCategory::message(int error_value) const {
  switch (static_cast<TerminateError>(error_value)) {
    case TerminateError::Success:
      return "Success";
    case TerminateError::SignalMaskFailed:
      return "Failed to block signals with sigprocmask()";
    case TerminateError::ThreadCreationFailed:
      return "Failed to create signal handler thread";
    case TerminateError::SignalWaitFailed:
      return "Signal wait operation failed";
    default:
      return "Unknown terminate error";
  }
}

const std::error_category& get_terminate_error_category() noexcept {
  static TerminateErrorCategory instance;
  return instance;
}

void Terminate::WaitForTerminateSignal() {
  try {
    // Signal mask (SIGINT, SIGTERM, SIGQUIT, SIGHUP) is inherited from main thread via Start()
    // This ensures signals are delivered only to this worker thread, not main thread

    // Notify systemd that daemon initialization is complete (Type=notify service)
    // See: https://www.freedesktop.org/software/systemd/man/sd_notify.html
    sd_notify(0, "READY=1");

    LOG(INFO) << "Application daemon has successfully started up.";
    
    // Loop to handle both termination and reload signals
    bool should_terminate = false;
    while (!should_terminate) {
      // Block and wait for one of the registered signals (SIGINT, SIGTERM, SIGQUIT, SIGHUP)
      int signal = 0;
      int received = sigwait(&m_waited_signals, &signal);
      
      if (received != 0) {
        LOG(ERROR) << "sigwait() failed with error code: " << received;
        break;
      }
      
      // Handle SIGHUP separately - config reload without termination
      if (signal == SIGHUP) {
        LOG(INFO) << "Received SIGHUP, queuing config reload event";
        
        // Notify systemd about config reload
        sd_notify(0, "RELOADING=1");
        
        // Queue event for processing thread to handle
        // Note: READY=1 will be sent by event processor after listeners complete
        {
          std::lock_guard<std::mutex> lock(m_event_mutex);
          m_event_queue.push(EventType::ConfigReload);
        }
        m_event_cv.notify_one();
        
        // Continue waiting for signals (don't terminate)
        continue;
      }
      
      // Handle termination signals (SIGINT, SIGTERM, SIGQUIT)
      should_terminate = true;
      
      // Notify systemd that graceful shutdown has begun
      sd_notify(0, "STOPPING=1");

      // Store termination reason for later retrieval by WaitForTermination()
      // Thread safety: synchronized via m_terminate.release() happens-before m_terminate.acquire()
      m_terminate_reason = GetSignalName(signal);
      LOG(INFO) << "Application daemon shutting down. Received signal " << signal << " (" << m_terminate_reason << ")";
    }

  } catch (const std::exception& l_exception) {
    // Notify systemd and log if any exception occurs during signal handling
    sd_notifyf(0, "STATUS=Failed during signal wait: %s\n",  // NOLINT(cppcoreguidelines-pro-type-vararg) NOLINT(hicpp-vararg)
               l_exception.what());
    LOG(ERROR) << "Exception in signal handler: " << l_exception.what();
  }
  // Signal main thread that shutdown can proceed (via WaitForTermination)
  m_terminate.release();
}

Terminate::Terminate() : m_terminate{0}, m_wait_ms{0}, m_waited_signals{}, m_stop_event_processor{false} {
  // Initialize binary_semaphore to 0 (blocked state) - will be released on termination signal
  
  sigemptyset(&m_waited_signals);
  // Register signals to handle for graceful shutdown
  sigaddset(&m_waited_signals, SIGINT);   // Ctrl-C - interactive interrupt
  sigaddset(&m_waited_signals, SIGTERM);  // Standard termination (systemd, kill)
  sigaddset(&m_waited_signals, SIGQUIT);  // Ctrl-\ - quit with core dump signal
  sigaddset(&m_waited_signals, SIGHUP);   // Hangup - config reload without restart
};

Terminate::~Terminate() {
  // Stop event processor thread
  if (m_event_processor && m_event_processor->joinable()) {
    {
      std::lock_guard<std::mutex> lock(m_event_mutex);
      m_stop_event_processor = true;
      m_event_queue.push(EventType::Shutdown);
    }
    m_event_cv.notify_one();
    m_event_processor->join();
  }

  // Stop signal handler thread
  if (m_signal_wait && m_signal_wait->joinable()) {
    // Signal worker thread to exit and wait for clean shutdown
    m_terminate.release();
    m_signal_wait->join();
  }
  // Note: As a singleton, this destructor is only called at program exit
}

std::error_code Terminate::Start() {
  // Block signals in main thread so they are only delivered to the dedicated worker thread
  // Worker thread inherits this mask and uses sigwait() to receive signals synchronously
  // Note: Must be called from main thread before spawning worker thread
  int result = sigprocmask(SIG_BLOCK, &m_waited_signals, nullptr);  // NOLINT(concurrency-mt-unsafe)
  if (result != 0) {
    LOG(ERROR) << "sigprocmask() failed with error code: " << result;
    return make_error_code(TerminateError::SignalMaskFailed);
  }

  // Spawn event processor thread to handle config reload notifications
  try {
    m_event_processor = std::make_unique<std::thread>(&Terminate::ProcessEvents, this);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to create event processor thread: " << e.what();
    // Restore signal mask before returning error
    sigprocmask(SIG_UNBLOCK, &m_waited_signals, nullptr);  // NOLINT(concurrency-mt-unsafe)
    return make_error_code(TerminateError::ThreadCreationFailed);
  }

  // Spawn dedicated thread to handle termination signals
  try {
    m_signal_wait = std::make_unique<std::thread>(&Terminate::WaitForTerminateSignal, this);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to create signal handler thread: " << e.what();
    
    // Cleanup event processor thread before returning error
    {
      std::lock_guard<std::mutex> lock(m_event_mutex);
      m_stop_event_processor = true;
      m_event_queue.push(EventType::Shutdown);
    }
    m_event_cv.notify_one();
    if (m_event_processor && m_event_processor->joinable()) {
      m_event_processor->join();
    }
    
    // Restore signal mask
    sigprocmask(SIG_UNBLOCK, &m_waited_signals, nullptr);  // NOLINT(concurrency-mt-unsafe)
    return make_error_code(TerminateError::ThreadCreationFailed);
  }

  return {};  // Success - empty error_code
}

std::string Terminate::WaitForTermination() {
  // Block until termination signal is received (worker thread calls release())
  m_terminate.acquire();
  
  // Optional delay to allow graceful resource cleanup
  if (m_wait_ms > 0) {
    LOG(INFO) << "Delaying final shutdown by " << m_wait_ms << " ms for graceful cleanup";
    std::this_thread::sleep_for(std::chrono::milliseconds(m_wait_ms));
  }
  
  // Ensure worker thread has fully exited
  m_signal_wait->join();
  return m_terminate_reason;
}

void Terminate::TerminateApp(uint32_t milis_to_wait) {
  m_wait_ms = milis_to_wait;

  LOG(INFO) << "Programmatic termination requested, waiting " << m_wait_ms << " ms before exit";
  // Signal WaitForTermination() to unblock and begin shutdown sequence
  m_terminate.release();
}

void Terminate::RegisterConfigReloadListener(std::function<void()> callback) {
  std::lock_guard<std::mutex> lock(m_listeners_mutex);
  m_config_reload_listeners.push_back(std::move(callback));
  LOG(INFO) << "Registered config reload listener, total listeners: " << m_config_reload_listeners.size();
}

void Terminate::ProcessEvents() {
  LOG(INFO) << "Event processor thread started";
  
  while (!m_stop_event_processor) {
    std::unique_lock<std::mutex> lock(m_event_mutex);
    
    // Wait for events to be queued
    m_event_cv.wait(lock, [this] {
      return !m_event_queue.empty() || m_stop_event_processor;
    });
    
    // Process all queued events
    while (!m_event_queue.empty()) {
      EventType event = m_event_queue.front();
      m_event_queue.pop();
      
      // Release lock while processing to avoid blocking signal handler
      lock.unlock();
      
      // Handle event based on type
      switch (event) {
        case EventType::ConfigReload: {
          LOG(INFO) << "Processing ConfigReload event, invoking listeners";
          
          // Copy listeners under lock to avoid holding lock during callbacks
          std::vector<std::function<void()>> listeners_copy;
          {
            std::lock_guard<std::mutex> listeners_lock(m_listeners_mutex);
            listeners_copy = m_config_reload_listeners;
          }
          
          // Invoke all registered listeners without holding lock
          for (const auto& listener : listeners_copy) {
            try {
              listener();
            } catch (const std::exception& e) {
              LOG(ERROR) << "Exception in config reload listener: " << e.what();
            } catch (...) {
              LOG(ERROR) << "Unknown exception in config reload listener";
            }
          }
          
          LOG(INFO) << "Config reload event processed, invoked " << listeners_copy.size() << " listeners";
          
          // Notify systemd that reload is complete and we're ready again
          sd_notify(0, "READY=1");
          break;
        }
        
        case EventType::Shutdown:
          LOG(INFO) << "Processing Shutdown event, event processor will exit";
          return;  // Exit thread
      }
      
      // Re-acquire lock for next iteration
      lock.lock();
    }
  }
  
  LOG(INFO) << "Event processor thread exiting";
}

}  // namespace comm
