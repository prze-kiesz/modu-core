# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

Feature: Application Startup and Initialization
  As a system operator
  I want the application to start correctly
  So that all modules are properly initialized

  Scenario: Successful startup with default config
    Given a valid default configuration file exists
    When I start the application
    Then the application should initialize all modules
    And InfrConfig should be initialized
    And comm::Config should be initialized
    And comm::Terminate should be initialized
    And the application should be ready to accept signals

  Scenario: Startup with custom config file
    Given a custom config file with:
      """
      [infr_main]
      device_name = "custom_device"
      port = 7777
      """
    When I start the application with config file "custom_config.toml"
    Then the application should load the custom config
    And the device_name should be "custom_device"
    And the port should be 7777

  Scenario: Startup with missing config file
    Given no config file exists
    When I start the application
    Then the application should use default values
    And InfrConfig should be initialized with defaults
    And the application should continue running

  Scenario: Module initialization order
    When I start the application
    Then modules should initialize in correct order:
      | order | module         |
      | 1     | comm::Config   |
      | 2     | comm::Terminate|
      | 3     | InfrConfig     |
    And all initialization hooks should fire
