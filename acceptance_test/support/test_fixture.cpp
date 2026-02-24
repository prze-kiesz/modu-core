// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include "test_fixture.h"

#ifdef ENABLE_TEST_HOOKS

#include <glog/logging.h>
#include <signal.h>
#include <chrono>
#include <thread>

#include <infr_main.h>
#include <infr_config.h>
#include <comm_terminate.h>

namespace test {

TestFixture& TestFixture::Instance() {
  static TestFixture instance;
  return instance;
}

TestFixture::~TestFixture() {
  if (m_running) {
    StopApplication();
  }
}

void TestFixture::StartApplication(const std::string& config_file) {
  if (m_running) {
    LOG(WARNING) << "Application already running, stopping first";
    StopApplication();
  }

  m_config_file = config_file;
  m_running = true;
  m_exit_code = -1;

  m_app_thread = std::make_unique<std::thread>([this]() {
    RunApplication();
  });

  // Wait for initialization
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  LOG(INFO) << "[TEST_FIXTURE] Application started";
}

void TestFixture::StopApplication() {
  if (!m_running) {
    return;
  }

  LOG(INFO) << "[TEST_FIXTURE] Stopping application";
  SendSignal(SIGTERM);

  if (m_app_thread && m_app_thread->joinable()) {
    m_app_thread->join();
  }

  m_running = false;
  LOG(INFO) << "[TEST_FIXTURE] Application stopped";
}

bool TestFixture::IsRunning() const {
  return m_running;
}

void TestFixture::SendSignal(int signal) {
  if (!m_running) {
    LOG(WARNING) << "[TEST_FIXTURE] Cannot send signal, application not running";
    return;
  }

  LOG(INFO) << "[TEST_FIXTURE] Sending signal: " << signal;
  
  // Since we're running in the same process, directly invoke the signal handler
  // In real implementation with separate process, use kill(pid, signal)
  switch (signal) {
    case SIGHUP:
      // Trigger config reload
      comm::Config::Instance().Reload();
      break;
    case SIGTERM:
    case SIGINT:
    case SIGQUIT:
      // Trigger termination
      comm::Terminate::Instance().RequestShutdown();
      break;
    default:
      LOG(WARNING) << "[TEST_FIXTURE] Unhandled signal: " << signal;
  }
}

int TestFixture::WaitForExit(int timeout_ms) {
  if (!m_app_thread) {
    return m_exit_code;
  }

  auto start = std::chrono::steady_clock::now();
  while (m_running) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
    
    if (elapsed.count() > timeout_ms) {
      LOG(WARNING) << "[TEST_FIXTURE] Timeout waiting for exit";
      return -1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (m_app_thread->joinable()) {
    m_app_thread->join();
  }

  return m_exit_code;
}

void TestFixture::SetConfigPath(const std::string& path) {
  m_config_file = path;
}

infr::InfrMainConfig TestFixture::GetInfrConfig() {
  return infr::InfrConfig::Instance().Get();
}

void TestFixture::RegisterInfrConfigListener(std::function<void()> listener) {
  infr::InfrConfig::Instance().RegisterReloadListener(std::move(listener));
}

void TestFixture::Reset() {
  if (m_running) {
    StopApplication();
  }
  m_config_file.clear();
  m_exit_code = -1;
  TestHooks::Reset();
}

void TestFixture::RunApplication() {
  try {
    LOG(INFO) << "[TEST_FIXTURE] Starting application main loop";

    // Initialize Google Logging
    google::InitGoogleLogging("modu-core-test");
    FLAGS_logtostderr = 1;

    // This would normally call your main application entry point
    // For now, we'll initialize the key components
    
    // Initialize config
    if (!m_config_file.empty()) {
      comm::Config::Instance().SetConfigFile(m_config_file);
    }
    comm::Config::Instance().Initialize();
    TestHooks::OnModuleInitialized("comm::Config");

    // Initialize terminate handler
    comm::Terminate::Instance().Initialize();
    TestHooks::OnModuleInitialized("comm::Terminate");

    // Initialize InfrConfig
    infr::InfrConfig::Instance().Initialize();
    TestHooks::OnInfrConfigInitialized();

    LOG(INFO) << "[TEST_FIXTURE] Application initialized, entering wait loop";

    // Wait for termination
    while (m_running && !comm::Terminate::Instance().ShouldTerminate()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    LOG(INFO) << "[TEST_FIXTURE] Shutting down";
    TestHooks::OnShutdownStage("cleanup");

    m_exit_code = 0;

  } catch (const std::exception& e) {
    LOG(ERROR) << "[TEST_FIXTURE] Application error: " << e.what();
    m_exit_code = 1;
  }

  m_running = false;
  LOG(INFO) << "[TEST_FIXTURE] Application exited with code: " << m_exit_code;
}

} // namespace test

#endif // ENABLE_TEST_HOOKS
