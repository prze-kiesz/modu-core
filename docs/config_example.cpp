// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file config_example.cpp
 * @brief Example demonstrating how to use configuration from comm_config module
 */

#include <glog/logging.h>
#include "comm_main.h"

void demonstrate_config_usage() {
  // Get global configuration instance
  auto& config = comm::Main::getConfig();

  // Read application metadata
  if (auto app_name = config.get_string("app.name")) {
    LOG(INFO) << "Application name: " << *app_name;
  }

  if (auto app_version = config.get_int("app.version")) {
    LOG(INFO) << "Application version: " << *app_version;
  }

  if (auto debug_mode = config.get_bool("app.debug")) {
    LOG(INFO) << "Debug mode: " << (*debug_mode ? "enabled" : "disabled");
  }

  // Read logging configuration
  if (auto log_level = config.get_string("logging.level")) {
    LOG(INFO) << "Log level: " << *log_level;
  }

  if (auto log_dest = config.get_string("logging.destination")) {
    LOG(INFO) << "Log destination: " << *log_dest;
  }

  // Read system configuration
  if (auto threads = config.get_int("system.worker_threads")) {
    LOG(INFO) << "Worker threads: " << *threads;
  }

  if (auto max_conn = config.get_int("system.max_connections")) {
    LOG(INFO) << "Max connections: " << *max_conn;
  }

  if (auto timeout = config.get_double("system.timeout_seconds")) {
    LOG(INFO) << "Timeout: " << *timeout << " seconds";
  }

  // Read feature flags
  if (auto metrics = config.get_bool("features.enable_metrics")) {
    LOG(INFO) << "Metrics: " << (*metrics ? "enabled" : "disabled");
  }

  if (auto profiling = config.get_bool("features.enable_profiling")) {
    LOG(INFO) << "Profiling: " << (*profiling ? "enabled" : "disabled");
  }

  // Read service endpoints
  if (auto api_port = config.get_int("services.api_port")) {
    LOG(INFO) << "API port: " << *api_port;
  }

  if (auto admin_port = config.get_int("services.admin_port")) {
    LOG(INFO) << "Admin port: " << *admin_port;
  }

  // Set runtime values (can be modified programmatically)
  config.set_string("runtime.startup_time", "2026-01-07T17:15:00Z");
  config.set_int("runtime.pid", 12345);

  // List all configuration keys
  LOG(INFO) << "All configuration keys:";
  for (const auto& key : config.get_all_keys()) {
    LOG(INFO) << "  - " << key;
  }
}
