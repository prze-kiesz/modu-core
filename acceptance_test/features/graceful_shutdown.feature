# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

Feature: Graceful Application Shutdown
  As a system operator
  I want the application to shutdown gracefully
  So that no data is lost and resources are properly released

  Background:
    Given the application is running

  Scenario: Shutdown via SIGTERM
    When I send SIGTERM signal to the application
    Then the application should receive termination signal
    And the application should complete shutdown stages
    And the application should exit with code 0
    And all resources should be cleaned up

  Scenario: Shutdown via SIGINT
    When I send SIGINT signal to the application
    Then the application should receive termination signal
    And the shutdown stage "cleanup" should complete
    And the application should exit cleanly

  Scenario: Shutdown during config reload
    When I trigger config reload
    And I send SIGTERM before reload completes
    Then the config reload should be interrupted
    And the application should shutdown gracefully
    And the application should exit with code 0

  Scenario: Multiple termination signals
    When I send SIGTERM signal to the application
    And I send SIGTERM again after 50ms
    Then only one shutdown sequence should execute
    And the application should exit cleanly
