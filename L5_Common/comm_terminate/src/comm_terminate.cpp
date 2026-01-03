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

void Terminate::WaitForTerminateSignal() {
  try {
    // Signal mask (SIGINT, SIGTERM, SIGQUIT) is inherited from main thread via Start()
    // This ensures signals are delivered only to this worker thread, not main thread

    // Notify systemd that daemon initialization is complete (Type=notify service)
    // See: https://www.freedesktop.org/software/systemd/man/sd_notify.html
    sd_notify(0, "READY=1");

    LOG(INFO) << "Application daemon has successfully started up.";
    
    // Block and wait for one of the registered signals (SIGINT, SIGTERM, SIGQUIT)
    int signal = 0;
    int received = sigwait(&m_waited_signals, &signal);
    
    if (received != 0) {
      LOG(ERROR) << "sigwait() failed with error code: " << received;
      return;
    }
    
    // Notify systemd that graceful shutdown has begun
    sd_notify(0, "STOPPING=1");

    // Store termination reason for later retrieval by WaitForTermination()
    // Thread safety: synchronized via m_terminate.release() happens-before m_terminate.acquire()
    m_terminate_reason = GetSignalName(signal);
    LOG(INFO) << "Application daemon shutting down. Received signal " << signal << " (" << m_terminate_reason << ")";

  } catch (const std::exception& l_exception) {
    // Notify systemd and log if any exception occurs during signal handling
    sd_notifyf(0, "STATUS=Failed during signal wait: %s\n",  // NOLINT(cppcoreguidelines-pro-type-vararg) NOLINT(hicpp-vararg)
               l_exception.what());
    LOG(ERROR) << "Exception in signal handler: " << l_exception.what();
  }
  // Signal main thread that shutdown can proceed (via WaitForTermination)
  m_terminate.release();
}

Terminate::Terminate() : m_terminate{0}, m_wait_ms{0}, m_waited_signals{} {
  // Initialize binary_semaphore to 0 (blocked state) - will be released on termination signal
  
  sigemptyset(&m_waited_signals);
  // Register signals to handle for graceful shutdown
  sigaddset(&m_waited_signals, SIGINT);   // Ctrl-C - interactive interrupt
  sigaddset(&m_waited_signals, SIGTERM);  // Standard termination (systemd, kill)
  sigaddset(&m_waited_signals, SIGQUIT);  // Ctrl-\ - quit with core dump signal

  // TODO: Consider adding SIGHUP for config reload without restart
};

Terminate::~Terminate() {
  if (m_signal_wait && m_signal_wait->joinable()) {
    // Signal worker thread to exit and wait for clean shutdown
    m_terminate.release();
    m_signal_wait->join();
  }
  // Note: As a singleton, this destructor is only called at program exit
}

code_t Terminate::Start() {
  code_t ret_code = OK;

  // Block signals in main thread so they are only delivered to the dedicated worker thread
  // Worker thread inherits this mask and uses sigwait() to receive signals synchronously
  // Note: Must be called from main thread before spawning worker thread
  sigprocmask(SIG_BLOCK, &m_waited_signals, nullptr);  // NOLINT(concurrency-mt-unsafe)

  // Spawn dedicated thread to handle termination signals
  m_signal_wait = std::make_unique<std::thread>(&Terminate::WaitForTerminateSignal, this);

  return ret_code;
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

}  // namespace comm
