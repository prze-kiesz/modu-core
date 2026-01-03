// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include "comm_os.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

using drv::CalculateJulianDateTime;
using drv::FindAvailablePort;
using drv::Semaphore;
using drv::TimeoutSemaphore;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, misc-use-anonymous-namespace)

TEST(SemaphoreTest, SignalAndWaitWakeWaitingThread) {
  Semaphore sem(0);
  bool was_awakened = false;

  std::thread thread([&] {
    sem.Wait();
    was_awakened = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  EXPECT_FALSE(was_awakened);

  sem.Signal();  // should wake the waiting thread

  thread.join();
  EXPECT_TRUE(was_awakened);
}

TEST(SemaphoreTest, MultipleSignalsMultipleWaits) {
  Semaphore sem(0);
  sem.Signal();
  sem.Signal();

  std::thread thread1([&] { sem.Wait(); });
  std::thread thread2([&] { sem.Wait(); });

  thread1.join();
  thread2.join();

  // finished so we can assume the test passed
  SUCCEED();
}

TEST(TimeoutSemaphoreTest, WaitWithTimeoutSuccess) {
  TimeoutSemaphore sem(0);

  std::thread threads([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sem.Signal();
  });

  bool result = sem.Wait(std::chrono::milliseconds(500));

  threads.join();
  EXPECT_TRUE(result);
}

TEST(TimeoutSemaphoreTest, WaitWithTimeoutTimeoutOccurs) {
  TimeoutSemaphore sem(0);
  bool result = sem.Wait(std::chrono::milliseconds(100));
  EXPECT_FALSE(result);
}

TEST(TimeoutSemaphoreTest, MultipleSignalsMultipleWaits) {
  TimeoutSemaphore sem(0);
  sem.Signal();
  sem.Signal();

  bool result1 = sem.Wait(std::chrono::milliseconds(100));
  bool result2 = sem.Wait(std::chrono::milliseconds(100));
  EXPECT_TRUE(result1);
  EXPECT_TRUE(result2);
}

// Placeholder – actual implementation would be needed
TEST(UtilTest, CalculateJulianDateTimeReturnsReasonableValue) {
  double julian_date = CalculateJulianDateTime();
  EXPECT_GT(julian_date, 2400000.0);  // Julian Dates are ~2.4M+
}

TEST(UtilTest, FindAvailablePortReturnsNonZero) {
  uint16_t port = FindAvailablePort(10000);
  EXPECT_GT(port, 0);
}

// counter of threads
static void RunInThreads(int count, std::function<void()> th_function) {
  std::vector<std::thread> threads;
  threads.reserve(count);
  for (int i = 0; i < count; ++i) {
    threads.emplace_back(th_function);
  }
  for (auto& thread : threads) {
    thread.join();
  }
}

TEST(SemaphoreTest, WaitWithoutSignalBlocks) {
  Semaphore sem(0);
  std::atomic<bool> flag = false;

  std::thread thread([&] {
    sem.Wait();  // should block
    flag = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_FALSE(flag);  // should still be blocked

  sem.Signal();  // unblock the thread
  thread.join();
  EXPECT_TRUE(flag);
}

TEST(SemaphoreTest, SignalThenWaitDoesNotBlock) {
  Semaphore sem(1);  // one signal already available
  auto start = std::chrono::steady_clock::now();

  sem.Wait();  // should not block

  auto duration = std::chrono::steady_clock::now() - start;
  EXPECT_LT(duration, std::chrono::milliseconds(10));  // should not wait
}

TEST(SemaphoreTest, MultiThreadedAccess) {
  Semaphore sem(0);
  std::atomic<int> counter = 0;

  std::thread signaler([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    for (int i = 0; i < 10; ++i) {
      sem.Signal();
    }
  });

  RunInThreads(10, [&] {
    sem.Wait();
    counter++;
  });

  signaler.join();
  EXPECT_EQ(counter, 10);
}

TEST(TimeoutSemaphoreTest, WaitImmediateTimeout) {
  TimeoutSemaphore sem(0);
  auto start = std::chrono::steady_clock::now();
  bool result = sem.Wait(std::chrono::milliseconds(1));
  auto elapsed = std::chrono::steady_clock::now() - start;

  EXPECT_FALSE(result);
  EXPECT_GE(elapsed, std::chrono::milliseconds(1));
}

TEST(TimeoutSemaphoreTest, WaitLongTimeoutWhenSignalIsMissing) {
  TimeoutSemaphore sem(0);
  auto start = std::chrono::steady_clock::now();
  bool result = sem.Wait(std::chrono::milliseconds(200));
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

  EXPECT_FALSE(result);
  EXPECT_GE(elapsed.count(), 180);  // should be close to 200ms
}

TEST(TimeoutSemaphoreTest, WaitSuccessBeforeTimeout) {
  TimeoutSemaphore sem(0);
  std::thread thread([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sem.Signal();
  });

  bool result = sem.Wait(std::chrono::milliseconds(200));
  thread.join();
  EXPECT_TRUE(result);
}

TEST(TimeoutSemaphoreTest, WaitInParallelWithMultipleThreads) {
  TimeoutSemaphore sem(0);
  std::atomic<int> passed{0};

  std::thread signaler([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    for (int i = 0; i < 5; ++i) {
      sem.Signal();
    }
  });

  RunInThreads(5, [&] {
    if (sem.Wait(std::chrono::milliseconds(100))) {
      passed++;
    }
  });

  signaler.join();
  EXPECT_EQ(passed, 5);
}

TEST(SemaphoreTest, DestructorDoesNotBlockOrThrow) {
  // Check that destroying the Semaphore does not throw or block
  EXPECT_NO_THROW({ std::unique_ptr<Semaphore> sem = std::make_unique<Semaphore>(0); });
}

TEST(TimeoutSemaphoreTest, DestructorDoesNotBlockOrThrow) {
  // Same for TimeoutSemaphore
  EXPECT_NO_THROW({ std::unique_ptr<TimeoutSemaphore> sem = std::make_unique<TimeoutSemaphore>(0); });
}

TEST(SemaphoreTest, MultipleInitialSignalsAccessImmediately) {
  Semaphore sem(5);  // 5 resources available
  std::atomic<int> counter = 0;

  // All 5 threads should pass through Wait() without delay
  RunInThreads(5, [&] {
    sem.Wait();
    counter++;
  });

  EXPECT_EQ(counter, 5);
}

TEST(SemaphoreTest, StressTestThousandThreads) {
  Semaphore sem(0);
  std::atomic<int> counter = 0;

  // Separate thread that sends 1000 signals after a short delay
  std::thread signaler([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    for (int i = 0; i < 1000; ++i) {
      sem.Signal();
    }
  });

  // 1000 threads will block until signaled
  RunInThreads(1000, [&] {
    sem.Wait();
    counter++;
  });

  signaler.join();
  EXPECT_EQ(counter, 1000);  // All threads should have passed Wait()
}

TEST(TimeoutSemaphoreTest, WaitZeroTimeoutAlwaysTimesOut) {
  TimeoutSemaphore sem(0);

  // With 0ms timeout and no signal, Wait should return false immediately
  bool result = sem.Wait(std::chrono::milliseconds(0));
  EXPECT_FALSE(result);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers, misc-use-anonymous-namespace)