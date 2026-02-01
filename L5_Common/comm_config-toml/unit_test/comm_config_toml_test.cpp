// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_config_toml_test.cpp
 * @brief Unit tests for TOML configuration management
 */

#include "comm_config_core.h"

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <cstdlib>

namespace comm {

class ConfigTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // Create shared test directory for all tests
    test_dir_ = "/tmp/modu-core-test-" + std::to_string(getpid());
    std::filesystem::create_directories(test_dir_);
  }
  
  static void TearDownTestSuite() {
    // Cleanup test directory after all tests
    if (std::filesystem::exists(test_dir_)) {
      std::filesystem::remove_all(test_dir_);
    }
  }
  
  void SetUp() override {
    // Reset config for each test by creating a fresh instance
    // Note: Since Config is a singleton, we can't truly reset it,
    // but we can ensure each test uses a unique config file
    test_config_path_ = test_dir_ + "/test_config_" + 
                        std::to_string(test_counter_++) + ".toml";
  }

  void TearDown() override {
    // Cleanup individual test config file
    if (std::filesystem::exists(test_config_path_)) {
      std::filesystem::remove(test_config_path_);
    }
  }
  
  void CreateTestConfig(const std::string& content) {
    std::ofstream file(test_config_path_);
    file << content;
    file.close();
  }
  
  static std::string test_dir_;
  std::string test_config_path_;
  static int test_counter_;
};

std::string ConfigTest::test_dir_;
int ConfigTest::test_counter_ = 0;

TEST_F(ConfigTest, InstanceReturnsSingleton) {
  Config& instance1 = Config::Instance();
  Config& instance2 = Config::Instance();
  
  EXPECT_EQ(&instance1, &instance2);
}

TEST_F(ConfigTest, InitializeReturnsSuccess) {
  // This test initializes the singleton for the first time
  Config& config = Config::Instance();
  std::error_code ec = config.Initialize("test-app");
  
  EXPECT_FALSE(ec);
  EXPECT_TRUE(config.IsInitialized());
}

TEST_F(ConfigTest, LoadAcceptsPath) {
  CreateTestConfig("[test]\nvalue = 42\n");
  
  Config& config = Config::Instance();
  std::error_code ec = config.Load(test_config_path_);
  
  EXPECT_FALSE(ec);
}

TEST_F(ConfigTest, LoadNonExistentFileReturnsError) {
  Config& config = Config::Instance();
  std::error_code ec = config.Load("/nonexistent/path/config.toml");
  
  EXPECT_TRUE(ec);
}

TEST_F(ConfigTest, LoadInvalidTomlReturnsError) {
  CreateTestConfig("this is not valid TOML {]]}");
  
  Config& config = Config::Instance();
  std::error_code ec = config.Load(test_config_path_);
  
  EXPECT_TRUE(ec);
}

TEST_F(ConfigTest, ReloadSucceedsAfterLoad) {
  CreateTestConfig("[test]\nvalue = 42\n");
  
  Config& config = Config::Instance();
  config.Load(test_config_path_);
  
  std::error_code ec = config.Reload();
  
  EXPECT_FALSE(ec);
}

TEST_F(ConfigTest, OverridesPersistAfterReload) {
  CreateTestConfig("[test]\nport = 1000\n");

  Config& config = Config::Instance();
  std::error_code load_ec = config.Load(test_config_path_);
  EXPECT_FALSE(load_ec);

  config.SetOverride("test.port", "2000");

  std::error_code reload_ec = config.Reload();
  EXPECT_FALSE(reload_ec);

  const auto& data = config.GetData();
  ASSERT_TRUE(data.is_table());
  const auto& test_table = data.as_table().at("test").as_table();
  EXPECT_EQ(test_table.at("port").as_integer(), 2000);
}

TEST_F(ConfigTest, ReloadInvokesRegisteredListeners) {
  CreateTestConfig("[test]\nvalue = 42\n");

  Config& config = Config::Instance();
  std::error_code load_ec = config.Load(test_config_path_);
  EXPECT_FALSE(load_ec);

  int call_count = 0;
  config.RegisterReloadListener([&call_count]() { ++call_count; });

  std::error_code reload_ec = config.Reload();
  EXPECT_FALSE(reload_ec);
  EXPECT_EQ(call_count, 1);
}

TEST_F(ConfigTest, ReloadDoesNotNotifyOnFailure) {
  Config& config = Config::Instance();

  // Use isolated XDG config path for this test
  const std::string xdg_dir = test_dir_ + "/xdg";
  const std::string app_dir = xdg_dir + "/test-app";
  const std::string config_path = app_dir + "/config.toml";
  std::filesystem::create_directories(app_dir);
  setenv("XDG_CONFIG_HOME", xdg_dir.c_str(), 1);

  // First, write valid config and initialize successfully
  {
    std::ofstream file(config_path);
    file << "[test]\nvalue = 1\n";
  }

  std::error_code init_ec = config.Initialize("test-app");
  EXPECT_FALSE(init_ec);

  int call_count = 0;
  config.RegisterReloadListener([&call_count]() { ++call_count; });

  // Now make config invalid so reload fails
  {
    std::ofstream file(config_path);
    file << "this is not valid TOML {]]}";
  }

  std::error_code reload_ec = config.Reload();
  EXPECT_TRUE(reload_ec);
  EXPECT_EQ(call_count, 0);
}

TEST_F(ConfigTest, SetOverrideAcceptsIntValue) {
  Config& config = Config::Instance();
  
  config.SetOverride("test.port", "8080");
  
  const auto& data = config.GetData();
  ASSERT_TRUE(data.is_table());
  ASSERT_TRUE(data.as_table().contains("test"));
  ASSERT_TRUE(data.as_table().at("test").is_table());
  ASSERT_TRUE(data.as_table().at("test").as_table().contains("port"));
  
  auto port_value = data.as_table().at("test").as_table().at("port");
  EXPECT_TRUE(port_value.is_integer());
  EXPECT_EQ(port_value.as_integer(), 8080);
}

TEST_F(ConfigTest, SetOverrideAcceptsBoolValue) {
  Config& config = Config::Instance();
  
  config.SetOverride("test.enabled", "true");
  
  const auto& data = config.GetData();
  auto enabled_value = data.as_table().at("test").as_table().at("enabled");
  EXPECT_TRUE(enabled_value.is_boolean());
  EXPECT_TRUE(enabled_value.as_boolean());
}

TEST_F(ConfigTest, SetOverrideAcceptsFloatValue) {
  Config& config = Config::Instance();
  
  config.SetOverride("test.timeout", "3.14");
  
  const auto& data = config.GetData();
  auto timeout_value = data.as_table().at("test").as_table().at("timeout");
  EXPECT_TRUE(timeout_value.is_floating());
  EXPECT_DOUBLE_EQ(timeout_value.as_floating(), 3.14);
}

TEST_F(ConfigTest, SetOverrideAcceptsStringValue) {
  Config& config = Config::Instance();
  
  config.SetOverride("test.name", "test-device");
  
  const auto& data = config.GetData();
  auto name_value = data.as_table().at("test").as_table().at("name");
  EXPECT_TRUE(name_value.is_string());
  EXPECT_EQ(name_value.as_string(), "test-device");
}

TEST_F(ConfigTest, SetOverrideCreatesNestedTables) {
  Config& config = Config::Instance();
  
  config.SetOverride("level1.level2.level3.value", "42");
  
  const auto& data = config.GetData();
  ASSERT_TRUE(data.as_table().contains("level1"));
  ASSERT_TRUE(data.as_table().at("level1").as_table().contains("level2"));
  ASSERT_TRUE(data.as_table().at("level1").as_table().at("level2").as_table().contains("level3"));
  
  auto value = data.as_table().at("level1").as_table().at("level2").as_table().at("level3").as_table().at("value");
  EXPECT_EQ(value.as_integer(), 42);
}

TEST_F(ConfigTest, SetOverrideOverwritesExistingValue) {
  CreateTestConfig("[test]\nport = 8080\n");
  
  Config& config = Config::Instance();
  config.Load(test_config_path_);
  
  config.SetOverride("test.port", "9000");
  
  const auto& data = config.GetData();
  auto port_value = data.as_table().at("test").as_table().at("port");
  EXPECT_EQ(port_value.as_integer(), 9000);
}

TEST_F(ConfigTest, MultipleOverridesWork) {
  Config& config = Config::Instance();
  
  config.SetOverride("server.port", "8080");
  config.SetOverride("server.host", "localhost");
  config.SetOverride("server.debug", "true");
  
  const auto& data = config.GetData();
  const auto& server = data.as_table().at("server").as_table();
  
  EXPECT_EQ(server.at("port").as_integer(), 8080);
  EXPECT_EQ(server.at("host").as_string(), "localhost");
  EXPECT_TRUE(server.at("debug").as_boolean());
}

TEST_F(ConfigTest, GetDataReturnsEmptyTableBeforeInit) {
  Config& config = Config::Instance();
  
  const auto& data = config.GetData();
  
  EXPECT_TRUE(data.is_table());
}

TEST_F(ConfigTest, MakeErrorCodeReturnsValidErrorCode) {
  std::error_code ec = make_error_code(ConfigError::FileNotFound);
  
  EXPECT_TRUE(ec);
  EXPECT_EQ(ec.value(), static_cast<int>(ConfigError::FileNotFound));
  EXPECT_STREQ(ec.category().name(), "config");
}

TEST_F(ConfigTest, ErrorCategoryReturnsCorrectMessages) {
  const std::error_category& category = get_config_error_category();
  
  EXPECT_STREQ(category.message(static_cast<int>(ConfigError::Success)).c_str(), "Success");
  EXPECT_STREQ(category.message(static_cast<int>(ConfigError::FileNotFound)).c_str(), 
               "Configuration file not found");
  EXPECT_STREQ(category.message(static_cast<int>(ConfigError::ParseError)).c_str(), 
               "Failed to parse configuration file");
}

}  // namespace comm
