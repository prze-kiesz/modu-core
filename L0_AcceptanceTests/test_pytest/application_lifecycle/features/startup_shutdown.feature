Feature: Application Lifecycle

  Scenario: Application logs its version on startup
    Given the application is started
    Then the startup log contains a version tag matching "modu-core-v\d+\.\d+\.\d+"

  Scenario: Application starts and becomes ready
    Given the application is started
    Then the log contains "Waiting for application termination"
    And there are no ERROR lines in the startup logs

  Scenario: SIGTERM triggers graceful shutdown
    Given the application is running
    When SIGTERM is sent
    Then the log contains "Application daemon shutting down."
    And the exit code is 0

  Scenario: SIGINT triggers graceful shutdown
    Given the application is running
    When SIGINT is sent
    Then the log contains "First SIGINT received - starting graceful shutdown"
    And the log contains "Application daemon shutting down."

  Scenario: Double SIGINT forces immediate exit with code 130
    Given the application is running
    When SIGINT is sent
    And SIGINT is sent again
    Then the log contains "Second SIGINT received - forcing immediate termination"
    And the exit code is 130
