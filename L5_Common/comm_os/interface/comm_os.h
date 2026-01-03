// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#pragma once

#include <condition_variable>
#include <mutex>

namespace drv {

/**
 * @brief Enum with commonly used return code values in the project
 *
 */
typedef enum {  // NOLINT(modernize-use-using)
  OK,
  ERROR,
  EXCEPTION_THROWN,
} code_t;

constexpr double JULIAN_DATE_TIME_FOR_UNIX = 2440587.5F;  // 2440587.5 is the JDN for January 1, 1970, at noon

/**
 * @brief Implementation of semaphore class resources waiting
 *
 */
class Semaphore {
 public:
  explicit Semaphore(int counter) : m_counter(counter) {}

  Semaphore(const Semaphore&) = delete;
  Semaphore& operator=(const Semaphore&) = delete;

  /**
   * @brief Notify threads waiting for resource
   *
   */
  void Signal() {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_counter++;
    m_resource_available.notify_one();
  }

  /**
   * @brief Wait for resource availability
   *
   */
  void Wait() {
    std::unique_lock<std::mutex> lock(m_mtx);
    while (m_counter == 0) {
      // wait on the mutex until notify is called
      m_resource_available.wait(lock);
    }
    m_counter--;
  }

 private:
  std::mutex m_mtx;
  std::condition_variable m_resource_available;
  int m_counter;
};

/**
 * @brief Semaphore with timeout class
 *
 */
class TimeoutSemaphore {
 public:
  explicit TimeoutSemaphore(int counter = 0) : m_counter(counter) {}

  void Signal() {
    std::unique_lock<std::mutex> lock(m_mutex);
    ++m_counter;
    m_resource_available.notify_one();
  }

  bool Wait(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_resource_available.wait_for(lock, timeout, [this] { return m_counter > 0; })) {
      --m_counter;
      return true;
    }
    return false;  // timeout
  }

 private:
  std::mutex m_mutex;
  std::condition_variable m_resource_available;
  int m_counter;
};

/**
 * Attempts to find an available port starting from the given port number.
 *
 * @param port_number The initial port number to try.
 * @return The available port number or 0 if no available port is found up to the maximum port number.
 */
uint16_t FindAvailablePort(uint16_t port_number);

/**
 * @brief Calculate Julian Date Time
 *
 * @return double Julian Date Time
 */
double CalculateJulianDateTime();

}  // namespace drv
