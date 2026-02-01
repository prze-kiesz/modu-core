// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file test_reload.cpp
 * @brief Integration test for config reload via SIGHUP
 * @details Uses Terminate to handle SIGHUP and triggers Config::Reload,
 *          then verifies listeners observe updated config.
 */

#include <glog/logging.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <thread>

#include "comm_config_core.h"
#include "comm_terminate.h"

namespace {

std::string CreateTempDir() {
  char tmpl[] = "/tmp/modu-core-config-it-XXXXXX";
  char* result = mkdtemp(tmpl);
  if (result == nullptr) {
    LOG(ERROR) << "mkdtemp failed: " << std::strerror(errno);
    throw std::runtime_error("Failed to create secure temp directory");
  }
  return std::string(result);
}

void WriteConfig(const std::string& path, int value) {
  std::ofstream file(path);
  file << "[test]\nvalue = " << value << "\n";
}

int GetValueFromConfig() {
  const auto& data = comm::Config::Instance().GetData();
  if (!data.is_table()) {
    return -1;
  }
  const auto& table = data.as_table();
  if (!table.contains("test")) {
    return -1;
  }
  const auto& test_table = table.at("test").as_table();
  if (!test_table.contains("value")) {
    return -1;
  }
  const auto& value = test_table.at("value");
  if (value.is_integer()) {
    return static_cast<int>(value.as_integer());
  }
  return -1;
}

}  // namespace

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 0;

  LOG(INFO) << "Config reload integration test started";
  LOG(INFO) << "PID: " << getpid();

  const std::string temp_dir = CreateTempDir();
  const std::string xdg_dir = temp_dir + "/xdg";
  const std::string app_dir = xdg_dir + "/test-app";
  const std::string config_path = app_dir + "/config.toml";
  std::filesystem::create_directories(app_dir);
  setenv("XDG_CONFIG_HOME", xdg_dir.c_str(), 1);

  WriteConfig(config_path, 1);

  auto init_ec = comm::Config::Instance().Initialize("test-app");
  if (init_ec) {
    LOG(ERROR) << "Config init failed: " << init_ec.message();
    return 1;
  }

  std::atomic<int> reload_count{0};
  std::atomic<int> last_value{GetValueFromConfig()};
  std::mutex reload_mutex;
  std::condition_variable reload_cv;

  comm::Config::Instance().RegisterReloadListener([&]() {
    ++reload_count;
    last_value = GetValueFromConfig();
    LOG(INFO) << "Reload listener invoked, value=" << last_value.load();
    reload_cv.notify_one();
  });

  comm::Terminate::Instance().RegisterConfigReloadListener([]() {
    LOG(INFO) << "SIGHUP received - reloading configuration";
    auto reload_ec = comm::Config::Instance().Reload();
    if (reload_ec) {
      LOG(ERROR) << "Reload failed: " << reload_ec.message();
    }
  });

  auto start_ec = comm::Terminate::Instance().Start();
  if (start_ec) {
    LOG(ERROR) << "Terminate start failed: " << start_ec.message();
    return 1;
  }

  std::thread signal_thread([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    WriteConfig(config_path, 2);
    kill(getpid(), SIGHUP);
    
    // Wait for reload to complete
    std::unique_lock<std::mutex> lock(reload_mutex);
    reload_cv.wait_for(lock, std::chrono::seconds(2), [&]() { return reload_count.load() >= 1; });
    
    kill(getpid(), SIGTERM);
  });

  std::string reason = comm::Terminate::Instance().WaitForTermination();
  LOG(INFO) << "Termination reason: " << reason;

  if (signal_thread.joinable()) {
    signal_thread.join();
  }

  if (reload_count.load() < 1) {
    LOG(ERROR) << "Reload listener was not invoked";
    return 1;
  }

  if (last_value.load() != 2) {
    LOG(ERROR) << "Expected value 2 after reload, got " << last_value.load();
    return 1;
  }

  LOG(INFO) << "Config reload integration test passed";
  google::ShutdownGoogleLogging();
  return 0;
}
