# Quick Start - BDD Acceptance Tests

## 1. Prerequisites

Ensure you have:
- cucumber-cpp v0.8.0 (static, with GTest)
- Ruby 3.x
- **Cucumber 9.2.0** (NOT 10.x)
- **cucumber-wire 8.0.0**

```bash
# If missing, install compatible versions:
sudo gem install cucumber -v 9.2.0
sudo gem install cucumber-wire -v 8.0.0
```

**Note:** The development container includes all prerequisites.

## 2. Build with Test Hooks

```bash
cmake -S . -B build-test -DENABLE_ACCEPTANCE_TESTS=ON -DENABLE_TEST_HOOKS=ON

cmake --build build-test
```

## 3. Run Tests

```bash
# Run all acceptance tests
cmake --build build-test --target run_acceptance_tests

# Or run directly
./build-test/acceptance_test/cucumber_tests --format pretty
```

## 4. View Results

- Console output shows pass/fail for each scenario
- `build-test/acceptance_test/cucumber_results.json` contains detailed results

## Example Output

```
Feature: Configuration Reload via SIGHUP

  Scenario: Successfully reload configuration
    Given the application is initialized with default config  ✓
    When I send SIGHUP signal to the application             ✓
    Then the config reload should complete successfully       ✓

  Scenario: Reload with changed port
    Given the current port is 8080                           ✓
    When I update the config file to set port to 9090        ✓
    And I send SIGHUP signal to the application              ✓
    Then the new port should be 9090                         ✓

2 scenarios (2 passed)
7 steps (7 passed)
```

## Adding a New Test

1. **Write feature** in `acceptance_test/features/my_test.feature`:
```gherkin
Scenario: My test scenario
  Given some initial state
  When I perform an action
  Then something should happen
```

2. **Implement steps** in `acceptance_test/step_definitions/my_steps.cpp`:
```cpp
GIVEN("^some initial state$") {
  test::TestFixture::Instance().StartApplication();
}

WHEN("^I perform an action$") {
  // Do something
}

THEN("^something should happen$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("my_event"));
}
```

3. **Add hook** in production code:
```cpp
#ifdef ENABLE_TEST_HOOKS
  TEST_HOOK(OnMyEvent());
#endif
```

4. **Rebuild and test**:
```bash
cmake --build build-test --target cucumber_tests
./build-test/acceptance_test/cucumber_tests
```

See [README.md](README.md) for full documentation.
