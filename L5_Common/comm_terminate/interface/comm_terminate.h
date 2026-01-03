// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#pragma once

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace drv {

/**
 * @brief Enum with commonly used return code values in the project
 */
typedef enum {
  OK,
  ERROR,
  EXCEPTION_THROWN,
} code_t;

/**
 * @brief Implementation of semaphore class for resources waiting
 */
class Semaphore {
 public:
  explicit Semaphore(int counter) : m_counter(counter) {}

  Semaphore(const Semaphore&) = delete;
  Semaphore& operator=(const Semaphore&) = delete;

  void Signal() {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_counter++;
    m_resource_available.notify_one();
  }

  void Wait() {
    std::unique_lock<std::mutex> lock(m_mtx);
    while (m_counter == 0) {
      m_resource_available.wait(lock);
    }
    m_counter--;
  }

 private:
  std::mutex m_mtx;
  std::condition_variable m_resource_available;
  int m_counter;
};

class Terminate {
 private:
  // holds pointer to thread waiting for terminate signal
  std::unique_ptr<std::thread> m_signal_wait;
  Semaphore m_terminate;
  std::atomic<uint32_t> m_wait_ms;

  sigset_t m_waited_signals;
  std::string m_terminate_reason;

  /**
   * @brief Construct a new Terminate object
   *
   */
  Terminate();
  ~Terminate();

  /**
   * @brief Function waits for termination signal
   *
   */
  void WaitForTerminateSignal();

 public:
  /**
   * @brief Returns instance of Terminate class singleton
   * see: Meyers' Singleton in C++
   *
   * @return Terminate& instance of Config
   */
  static Terminate& Instance() {
    static Terminate instance;
    return instance;
  }

  /**
   * @brief Starts Terminate module
   *
   * @return code_t return code
   */
  code_t Start();

  /**
   * @brief Function should be called for send termination signal application
   * Signal trigers app termination in main software task - causes to exit from waitForTermination()
   *
   * @param milis_to_wait number of miliseconds to wait before termination task will exits
   */
  void TerminateApp(uint32_t milis_to_wait = 0);

  /**
   * @brief Method should be called in main program loop to wait for program/service termination
   *
   * @return std::string String with termination reason
   */
  std::string WaitForTermination(void);
};

}  // namespace drv