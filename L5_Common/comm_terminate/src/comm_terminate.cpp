// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include "comm_terminate.h"

#include <glog/logging.h>
#include <signal.h>
#include <systemd/sd-daemon.h>

#include <csignal>
#include <exception>
#include <string>

namespace {
/**
 * @brief Converts signal number to string
 *
 * @param signal signal number
 * @return std::string signal name
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
    sigprocmask(SIG_BLOCK, &m_waited_signals, nullptr);  // NOLINT(concurrency-mt-unsafe)

    /**
     * Informs systemd about changed daemon state. This takes a number of
     * newline separated environment-style variable assignments in a
     * string. The following variables are known:
     *
     * MAINPID=... The main PID of a daemon, in case systemd did not
     * fork off the process itself. Example: "MAINPID=4711"
     *
     * READY=1 Tells systemd that daemon startup or daemon reload
     * is finished (only relevant for services of Type=notify).
     * The passed argument is a boolean "1" or "0". Since there
     * is little value in signaling non-readiness the only
     * value daemons should send is "READY=1".
     */
    sd_notify(0, "READY=1");

    LOG(INFO) << "The office daemon has successfully started up.";
    int signal = 0;
    int received = sigwait(&m_waited_signals, &signal);
    sd_notify(0, "STOPPING=1");

    m_terminate_reason = GetSignalName(signal);
    LOG(INFO) << "The office daemon has been successfully shut down. Signal " << received << " signal " << signal << " name "
              << m_terminate_reason;

  } catch (std::exception& l_exception) {
    sd_notifyf(0, "STATUS=Failed to start up: %s\n",  // NOLINT(cppcoreguidelines-pro-type-vararg) NOLINT(hicpp-vararg)
               l_exception.what());
  }
  // notify waiting thread that should finish - in any
  m_terminate.Signal();
}

Terminate::Terminate() : m_terminate{0}, m_wait_ms{0}, m_waited_signals{} {
  sigemptyset(&m_waited_signals);
  // add list of signals which are handled in this module
  sigaddset(&m_waited_signals, SIGINT);   // Ctrl-C (graceful)
  sigaddset(&m_waited_signals, SIGTERM);  // systemd/kill (graceful)
  sigaddset(&m_waited_signals, SIGQUIT);  // Ctrl-\ (graceful)

  // TODO: in future sigaddset(&m_waited_signals, SIGHUP);   // reload config (optional)
};

Terminate::~Terminate() {
  if (m_signal_wait->joinable()) {
    // notify waiting thread that should finish
    m_terminate.Signal();
    pthread_kill(m_signal_wait->native_handle(), SIGTERM);

    m_signal_wait->join();
  }
}

code_t Terminate::Start() {
  code_t ret_code = OK;

  // we are assuming that initialization methods are called from main thread.
  // for correct signal handling we have to block delivery of signals to main thread
  sigprocmask(SIG_SETMASK, &m_waited_signals, nullptr);  // NOLINT(concurrency-mt-unsafe)

  m_signal_wait = std::make_unique<std::thread>(&Terminate::WaitForTerminateSignal, this);

  return ret_code;
}

std::string Terminate::WaitForTermination() {
  m_terminate.Wait();
  LOG(INFO) << "Sleep for " << m_wait_ms << " ms before exit app";
  std::this_thread::sleep_for(std::chrono::milliseconds(m_wait_ms));
  m_signal_wait->join();
  return m_terminate_reason;
}

void Terminate::TerminateApp(uint32_t milis_to_wait) {
  m_wait_ms = milis_to_wait;

  LOG(INFO) << "Notify to quit GRPC application after " << m_wait_ms;
  // notify waiting thread that should finish
  m_terminate.Signal();

  pthread_kill(m_signal_wait->native_handle(), SIGTERM);
}

}  // namespace comm
