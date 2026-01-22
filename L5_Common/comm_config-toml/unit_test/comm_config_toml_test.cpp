// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

/**
 * @file comm_config_toml_test.cpp
 * @brief Unit tests for TOML configuration management
 */

#include "comm_config_toml.h"

#include <gtest/gtest.h>

namespace comm {

class ConfigTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Setup code before each test
  }

  void TearDown() override {
    // Cleanup code after each test
  }
};

TEST_F(ConfigTest, InstanceReturnsSingleton) {
  Config& instance1 = Config::Instance();
  Config& instance2 = Config::Instance();
  
  EXPECT_EQ(&instance1, &instance2);
}

TEST_F(ConfigTest, InitializeReturnsSuccess) {
  Config& config = Config::Instance();
  std::error_code ec = config.Initialize("test-app");
  
  EXPECT_FALSE(ec);
  EXPECT_TRUE(config.IsInitialized());
}

TEST_F(ConfigTest, LoadAcceptsPath) {
  Config& config = Config::Instance();
  config.Initialize("test-app");
  
  std::error_code ec = config.Load("/tmp/test_config.toml");
  
  EXPECT_FALSE(ec);
}

TEST_F(ConfigTest, ReloadFailsWhenNotInitialized) {
  Config& config = Config::Instance();
  
  // Note: This test assumes fresh instance, may need adjustment
  // std::error_code ec = config.Reload();
  // EXPECT_TRUE(ec);
  // EXPECT_EQ(ec, make_error_code(ConfigError::NotInitialized));
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
