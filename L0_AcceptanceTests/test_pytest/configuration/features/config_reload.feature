Feature: Configuration Reload via SIGHUP

  Scenario: SIGHUP triggers config reload event
    Given the application is running
    When SIGHUP is sent
    Then the log contains "Received SIGHUP, queuing config reload event"

  Scenario: Application keeps running after SIGHUP
    Given the application is running
    When SIGHUP is sent
    Then the log contains "Received SIGHUP, queuing config reload event"
    And the application is still running

  Scenario: Config file update is picked up on SIGHUP
    Given the application is running
    And the config file is updated on disk
    When SIGHUP is sent
    Then the log contains "Received SIGHUP, queuing config reload event"
    And the log contains "Configuration initialized with"
    And the application is still running
