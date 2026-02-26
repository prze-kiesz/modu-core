# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

Feature: Configuration Reload via SIGHUP
  As a system operator
  I want to reload configuration without restarting the application
  So that I can apply config changes without downtime

  Background:
    Given the application is initialized with default config
    And the config file path is "test_configs/infr_main.toml"

  Scenario: Successfully reload configuration
    Given the config file contains:
      """
      [infr_main]
      device_name = "device_v1"
      port = 8080
      enable_logging = true
      timeout_seconds = 30.0
      """
    When I send SIGHUP signal to the application
    Then the config reload should complete successfully
    And the infr_main config should be reloaded

  Scenario: Reload with changed port
    Given the current port is 8080
    When I update the config file to set port to 9090
    And I send SIGHUP signal to the application
    Then the config reload should complete successfully
    And the new port should be 9090
    And InfrConfig listeners should be notified

  Scenario: Reload with invalid configuration
    Given the application is running
    When I update the config file with invalid TOML syntax
    And I send SIGHUP signal to the application
    Then the config reload should fail
    And the application should continue running with old config
    And an error should be logged

  Scenario: Multiple rapid reloads
    Given the application is running
    When I send SIGHUP signal 3 times with 100ms delay
    Then all 3 config reloads should complete
    And the application should remain stable
    And no reload events should be dropped

  Scenario: Reload triggers listener chain
    Given InfrConfig has 2 registered listeners
    When I send SIGHUP signal to the application
    Then both listeners should be invoked
    And both listeners should receive the new config
