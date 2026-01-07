// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

#include "comm_config.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace {

class ConfigTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create temporary directory for test files
    test_dir = std::filesystem::temp_directory_path() / "modu_core_config_test";
    std::filesystem::create_directories(test_dir);
  }

  void TearDown() override {
    // Clean up test directory
    if (std::filesystem::exists(test_dir)) {
      std::filesystem::remove_all(test_dir);
    }
  }

  void create_test_config(const std::string& filename, const std::string& content) {
    std::ofstream file(test_dir / filename);
    file << content;
  }

  std::filesystem::path test_dir;
};

TEST_F(ConfigTest, EmptyConfigCreation) {
  comm::Config config;
  EXPECT_EQ(config.get_all_keys().size(), 0);
}

TEST_F(ConfigTest, LoadValidTOML) {
  create_test_config("test.toml", R"(
    [server]
    host = "localhost"
    port = 8080
    debug = true
    
    [database]
    connection_string = "postgresql://localhost:5432/mydb"
    max_connections = 100
    timeout = 30.5
    
    [features]
    enabled = ["auth", "logging", "metrics"]
  )");

  comm::Config config;
  auto err = config.load_from_file(test_dir / "test.toml");
  
  ASSERT_EQ(err.value(), 0);
  
  EXPECT_EQ(config.get_string("server.host"), "localhost");
  EXPECT_EQ(config.get_int("server.port"), 8080);
  EXPECT_EQ(config.get_bool("server.debug"), true);
  
  EXPECT_EQ(config.get_string("database.connection_string"),
            "postgresql://localhost:5432/mydb");
  EXPECT_EQ(config.get_int("database.max_connections"), 100);
  EXPECT_DOUBLE_EQ(config.get_double("database.timeout").value(), 30.5);
  
  auto features = config.get_string_array("features.enabled");
  ASSERT_TRUE(features.has_value());
  EXPECT_EQ(features->size(), 3);
  EXPECT_EQ((*features)[0], "auth");
  EXPECT_EQ((*features)[1], "logging");
  EXPECT_EQ((*features)[2], "metrics");
}

TEST_F(ConfigTest, LoadNonExistentFile) {
  comm::Config config;
  auto err = config.load_from_file(test_dir / "nonexistent.toml");
  
  EXPECT_EQ(err, comm::make_error_code(comm::ConfigError::FileNotFound));
}

TEST_F(ConfigTest, LoadInvalidTOML) {
  create_test_config("invalid.toml", R"(
    [server
    host = "localhost"
  )");

  comm::Config config;
  auto err = config.load_from_file(test_dir / "invalid.toml");
  
  EXPECT_EQ(err, comm::make_error_code(comm::ConfigError::ParseError));
}

TEST_F(ConfigTest, SetAndGetValues) {
  comm::Config config;
  
  config.set_string("app.name", "test_app");
  config.set_int("app.version", 42);
  config.set_double("app.ratio", 3.14);
  config.set_bool("app.enabled", true);
  config.set_string_array("app.tags", {"production", "critical"});
  
  EXPECT_EQ(config.get_string("app.name"), "test_app");
  EXPECT_EQ(config.get_int("app.version"), 42);
  EXPECT_DOUBLE_EQ(config.get_double("app.ratio").value(), 3.14);
  EXPECT_EQ(config.get_bool("app.enabled"), true);
  
  auto tags = config.get_string_array("app.tags");
  ASSERT_TRUE(tags.has_value());
  EXPECT_EQ(tags->size(), 2);
  EXPECT_EQ((*tags)[0], "production");
  EXPECT_EQ((*tags)[1], "critical");
}

TEST_F(ConfigTest, HasKeyAndRemove) {
  comm::Config config;
  config.set_string("test.key", "value");
  
  EXPECT_TRUE(config.has_key("test.key"));
  EXPECT_FALSE(config.has_key("test.nonexistent"));
  
  EXPECT_TRUE(config.remove_key("test.key"));
  EXPECT_FALSE(config.has_key("test.key"));
  EXPECT_FALSE(config.remove_key("test.key"));  // Already removed
}

TEST_F(ConfigTest, SaveAndLoadRoundtrip) {
  comm::Config config1;
  config1.set_string("app.name", "roundtrip_test");
  config1.set_int("app.count", 123);
  config1.set_bool("app.active", false);
  
  auto save_path = test_dir / "roundtrip.toml";
  auto save_err = config1.save_to_file(save_path);
  ASSERT_EQ(save_err.value(), 0);
  ASSERT_TRUE(std::filesystem::exists(save_path));
  
  comm::Config config2;
  auto load_err = config2.load_from_file(save_path);
  ASSERT_EQ(load_err.value(), 0);
  
  EXPECT_EQ(config2.get_string("app.name"), "roundtrip_test");
  EXPECT_EQ(config2.get_int("app.count"), 123);
  EXPECT_EQ(config2.get_bool("app.active"), false);
}

TEST_F(ConfigTest, MergeConfigurations) {
  create_test_config("base.toml", R"(
    [server]
    host = "localhost"
    port = 8080
    
    [database]
    host = "db.local"
  )");

  create_test_config("override.toml", R"(
    [server]
    port = 9090
    debug = true
    
    [cache]
    enabled = true
  )");

  comm::Config config;
  [[maybe_unused]] auto err1 = config.load_from_file(test_dir / "base.toml");
  [[maybe_unused]] auto err2 = config.merge_from_file(test_dir / "override.toml");
  
  // Original values
  EXPECT_EQ(config.get_string("server.host"), "localhost");
  EXPECT_EQ(config.get_string("database.host"), "db.local");
  
  // Overridden value
  EXPECT_EQ(config.get_int("server.port"), 9090);
  
  // New values from override
  EXPECT_EQ(config.get_bool("server.debug"), true);
  EXPECT_EQ(config.get_bool("cache.enabled"), true);
}

TEST_F(ConfigTest, GetAllKeys) {
  comm::Config config;
  config.set_string("a.b.c", "value1");
  config.set_int("a.b.d", 42);
  config.set_string("x.y", "value2");
  
  auto keys = config.get_all_keys();
  EXPECT_EQ(keys.size(), 3);
  
  EXPECT_NE(std::find(keys.begin(), keys.end(), "a.b.c"), keys.end());
  EXPECT_NE(std::find(keys.begin(), keys.end(), "a.b.d"), keys.end());
  EXPECT_NE(std::find(keys.begin(), keys.end(), "x.y"), keys.end());
}

TEST_F(ConfigTest, ClearConfiguration) {
  comm::Config config;
  config.set_string("key1", "value1");
  config.set_int("key2", 123);
  
  EXPECT_EQ(config.get_all_keys().size(), 2);
  
  config.clear();
  EXPECT_EQ(config.get_all_keys().size(), 0);
  EXPECT_FALSE(config.has_key("key1"));
  EXPECT_FALSE(config.has_key("key2"));
}

TEST_F(ConfigTest, TypeMismatch) {
  comm::Config config;
  config.set_string("value", "not_a_number");
  
  EXPECT_FALSE(config.get_int("value").has_value());
  EXPECT_FALSE(config.get_bool("value").has_value());
  EXPECT_FALSE(config.get_double("value").has_value());
  
  EXPECT_TRUE(config.get_string("value").has_value());
}

TEST_F(ConfigTest, IntegerToDoubleConversion) {
  comm::Config config;
  config.set_int("number", 42);
  
  // Should be able to get integer as double
  auto as_double = config.get_double("number");
  ASSERT_TRUE(as_double.has_value());
  EXPECT_DOUBLE_EQ(as_double.value(), 42.0);
}

TEST_F(ConfigTest, XDGHierarchyLoading) {
  // Create system config
  auto system_dir = test_dir / "system";
  std::filesystem::create_directories(system_dir);
  std::ofstream sys_file(system_dir / "config.toml");
  sys_file << "[app]\nname = \"system_app\"\nversion = 1\n";
  sys_file.close();

  // Create user config
  auto user_dir = test_dir / "user" / ".config" / "myapp";
  std::filesystem::create_directories(user_dir);
  std::ofstream user_file(user_dir / "config.toml");
  user_file << "[app]\nversion = 2\ndebug = true\n";
  user_file.close();

  // Set XDG_CONFIG_HOME temporarily
  setenv("XDG_CONFIG_HOME", (test_dir / "user" / ".config").c_str(), 1);

  comm::Config config;
  auto err = config.load_xdg_hierarchy("myapp", system_dir / "config.toml");
  
  ASSERT_EQ(err.value(), 0);
  
  // System value should be present
  EXPECT_EQ(config.get_string("app.name"), "system_app");
  
  // User value should override system value
  EXPECT_EQ(config.get_int("app.version"), 2);
  
  // User-only value should be present
  EXPECT_EQ(config.get_bool("app.debug"), true);
  
  unsetenv("XDG_CONFIG_HOME");
}

}  // namespace
