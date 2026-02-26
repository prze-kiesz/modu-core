# BDD Acceptance Tests with Cucumber-cpp

## Overview

This directory contains BDD (Behavior-Driven Development) acceptance tests for the entire modu-core application using Cucumber-cpp with Gherkin syntax.

## Architecture

The test system uses **Opcja B** - Test Mode with hooks compiled only in test builds:

```
acceptance_test/
├── features/                  # Gherkin feature files
│   ├── config_reload.feature
│   ├── graceful_shutdown.feature
│   └── application_startup.feature
├── step_definitions/          # C++ implementations of Gherkin steps
│   ├── config_steps.cpp
│   ├── terminate_steps.cpp
│   └── startup_steps.cpp
├── support/                   # Test infrastructure
│   ├── test_hooks.h/cpp      # Hook registry for observing app events
│   ├── test_fixture.h/cpp    # Application lifecycle control
│   └── cucumber_hooks.cpp    # Before/After scenario hooks
└── CMakeLists.txt
```

## How It Works

### Test Hooks

When built with `-DENABLE_TEST_HOOKS=ON`, the production code includes test hook points that allow tests to:
- Observe internal events (config reload, shutdown stages, etc.)
- Inspect application state
- Verify behavior without modifying production logic

Example hook in production code:
```cpp
#ifdef ENABLE_TEST_HOOKS
  TEST_HOOK(OnConfigReloadCompleted("infr_main"));
#endif
```

**Important**: Hooks are completely compiled out in production builds (when `ENABLE_TEST_HOOKS` is not defined).

### Test Fixture

`TestFixture` manages the application lifecycle during tests:
- Starts/stops the application
- Sends signals (SIGHUP, SIGTERM, etc.)
- Provides access to application state
- Waits for events to complete

### Gherkin Features

Feature files describe application behavior in plain English:

```gherkin
Feature: Configuration Reload via SIGHUP
  Scenario: Successfully reload configuration
    Given the application is running
    When I send SIGHUP signal to the application
    Then the config reload should complete successfully
    And InfrConfig listeners should be notified
```

## Building

### Prerequisites

```bash
# Install cucumber-cpp (static libraries with GTest support)
git clone --branch v0.8.0 https://github.com/cucumber/cucumber-cpp.git /tmp/cucumber-cpp
cd /tmp/cucumber-cpp
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCUKE_ENABLE_EXAMPLES=OFF \
  -DCUKE_ENABLE_GTEST=ON
cmake --build build -j$(nproc)
sudo cmake --install build
rm -rf /tmp/cucumber-cpp

# Install Ruby and Cucumber gems
# IMPORTANT: Version compatibility is critical
sudo apt-get install ruby ruby-dev

# Install compatible Cucumber versions:
# - Cucumber 9.2.0 (NOT 10.x - has incompatible cucumber-core)
# - cucumber-wire 8.0.0 requires cucumber-core < 16
# - Cucumber 10.x ships with cucumber-core 16.x which breaks compatibility
sudo gem install cucumber -v 9.2.0
sudo gem install cucumber-wire -v 8.0.0
```

**Version Compatibility Note:**
- ✓ Cucumber 9.2.0 + cucumber-wire 8.0.0 (recommended)
- ✗ Cucumber 10.x + cucumber-wire 8.0.0 (incompatible - cucumber-core version conflict)

The development container (`.devcontainer/Dockerfile`) includes all prerequisites.

### Build with Test Hooks

```bash
# Configure with acceptance tests enabled
cmake -S . -B build \
  -DENABLE_ACCEPTANCE_TESTS=ON \
  -DENABLE_TEST_HOOKS=ON

# Build everything
cmake --build build

# Build just the test binary
cmake --build build --target cucumber_tests
```

## Running Tests

### Method 1: Using CMake target

```bash
cmake --build build --target run_acceptance_tests
```

### Method 2: Direct execution

```bash
cd build/acceptance_test
./cucumber_tests --format pretty
```

### Method 3: Using the test script

```bash
./build/acceptance_test/run_tests.sh
```

## Wire Protocol Configuration

Cucumber (Ruby) communicates with the C++ wireserver using the **wire protocol** over TCP.

### Required Files

1. **features/support/cucumber.wire** - Wire protocol configuration:
```yaml
---
host: localhost
port: 3902
timeout:
  connect: 5
  invoke: 60
```

2. **features/support/env.rb** - Load cucumber-wire plugin:
```ruby
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

# Load cucumber-wire plugin to enable wire protocol support
require 'cucumber/wire'
```

**Important:** Without `env.rb` loading the `cucumber/wire` gem, Cucumber will not connect to the wireserver even if `cucumber.wire` is present.

### How It Works

1. C++ wireserver (`modu-core-test_bdd_cucumber`) listens on port 3902
2. Cucumber reads `.wire` file and connects via TCP
3. Cucumber queries available step definitions
4. When a step matches, Cucumber invokes it via wire protocol
5. C++ executes the step and returns results (pass/fail)

### Manual Testing

```bash
# Start wireserver in background
cd build-test/test_bdd_cucumber
./modu-core-test_bdd_cucumber -p 3902 &

# Run Cucumber tests from source directory
cd /workspaces/modu-core/L0_AcceptanceTests/test_bdd_cucumber
cucumber features/ --format pretty

# Check wire connection
echo '["step_matches",{"name_to_match":"test"}]' | nc localhost 3902
```

## Test Output

Tests generate two types of output:

1. **Console output** (pretty format) - Human-readable test results
2. **JSON output** (`cucumber_results.json`) - Machine-readable for CI/CD

Example console output:
```
Feature: Configuration Reload via SIGHUP

  Scenario: Successfully reload configuration
    Given the application is running                      ✓
    When I send SIGHUP signal to the application          ✓
    Then the config reload should complete successfully   ✓
    And InfrConfig listeners should be notified           ✓

1 scenario (1 passed)
4 steps (4 passed)
```

## Writing New Tests

### 1. Add a Feature File

Create `features/my_feature.feature`:
```gherkin
Feature: My Feature
  Scenario: Test something
    Given some precondition
    When I do something
    Then something should happen
```

### 2. Implement Step Definitions

Create `step_definitions/my_steps.cpp`:
```cpp
#include <cucumber-cpp/autodetect.hpp>
#include <gtest/gtest.h>
#include "test_hooks.h"
#include "test_fixture.h"

GIVEN("^some precondition$") {
  test::TestFixture::Instance().StartApplication();
}

WHEN("^I do something$") {
  // Trigger the action
}

THEN("^something should happen$") {
  EXPECT_TRUE(test::TestHooks::EventOccurred("my_event"));
}
```

### 3. Add Hooks to Production Code

In your production code, add test hooks at key points:
```cpp
#ifdef ENABLE_TEST_HOOKS
#include "test_hooks.h"
#endif

void MyFunction() {
  // ... do work ...
  
#ifdef ENABLE_TEST_HOOKS
  TEST_HOOK(OnMyEventHappened("details"));
#endif
}
```

### 4. Rebuild and Run

```bash
cmake --build build --target cucumber_tests
./build/acceptance_test/cucumber_tests
```

## Available Test Hooks

See [test_hooks.h](support/test_hooks.h) for the complete API. Key hooks include:

### Event Hooks
- `OnConfigReloadStarted()` - Config reload beginning
- `OnConfigReloadCompleted(section)` - Config reload succeeded
- `OnConfigReloadFailed(error)` - Config reload failed
- `OnTerminationSignalReceived(signal)` - Termination signal received
- `OnShutdownStage(stage)` - Shutdown stage completed
- `OnInfrConfigInitialized()` - InfrConfig initialized
- `OnModuleInitialized(module_name)` - Module initialized

### State Access
- `SetTestData(key, value)` - Store test data
- `GetTestData(key)` - Retrieve test data
- `EventOccurred(event_name)` - Check if event fired
- `GetEventCount(event_name)` - Count event occurrences
- `ClearEvents()` - Reset event tracking
- `Reset()` - Reset all test state

## Example Test Scenarios

### Config Reload Test
```gherkin
Scenario: Reload with changed port
  Given the current port is 8080
  When I update the config file to set port to 9090
  And I send SIGHUP signal to the application
  Then the config reload should complete successfully
  And the new port should be 9090
```

### Graceful Shutdown Test
```gherkin
Scenario: Shutdown via SIGTERM
  Given the application is running
  When I send SIGTERM signal to the application
  Then the application should receive termination signal
  And all resources should be cleaned up
  And the application should exit with code 0
```

## CI/CD Integration

### GitHub Actions Example

```yaml
- name: Build with test hooks
  run: |
    cmake -S . -B build \
      -DENABLE_ACCEPTANCE_TESTS=ON \
      -DENABLE_TEST_HOOKS=ON
    cmake --build build

- name: Run acceptance tests
  run: cmake --build build --target run_acceptance_tests

- name: Upload test results
  uses: actions/upload-artifact@v3
  with:
    name: cucumber-results
    path: build/acceptance_test/cucumber_results.json
```

## Troubleshooting

### Steps show as "undefined" - Cucumber not connecting to wireserver

**Symptoms:**
```
UUUUUUUUUUUUUUUUUUUU
4 scenarios (4 undefined)
20 steps (20 undefined)
```

**Causes:**
1. Missing `features/support/env.rb` with `require 'cucumber/wire'`
2. Wrong Cucumber/cucumber-wire versions (version mismatch)
3. Wireserver not running on port 3902

**Solutions:**
```bash
# 1. Ensure env.rb exists with correct require
cat features/support/env.rb
# Should contain: require 'cucumber/wire'

# 2. Check gem versions (must be compatible)
gem list | grep cucumber
# Should show: cucumber (9.2.0) and cucumber-wire (8.0.0)
# NOT cucumber (10.x) - incompatible with cucumber-wire 8.0.0

# 3. Check wireserver is running
netstat -tulpn | grep 3902
# Should show: tcp 0 0 127.0.0.1:3902 ... LISTEN

# 4. Test wire connection manually
echo '["step_matches",{"name_to_match":"test"}]' | nc localhost 3902
# Should return: ["success"]
```

### Wire protocol version conflict

**Error:**
```
Unable to activate cucumber-wire-8.0.0, because cucumber-core-16.2.0 conflicts with cucumber-core (> 11, < 16)
```

**Solution:** Downgrade Cucumber to 9.x:
```bash
sudo gem uninstall cucumber -x
sudo gem install cucumber -v 9.2.0
sudo gem install cucumber-wire -v 8.0.0
```

### Wireserver crashes with "InitGoogleLogging() twice"

**Cause:** Multiple glog initializations in test code.

**Solution:** Ensure glog is initialized only once (in `cucumber_hooks.cpp`), not in `TestFixture::RunApplication()`.

### cucumber-cpp not found

```bash
# Install development package
sudo apt-get install libcucumber-cpp-dev

# Or set CMAKE_PREFIX_PATH
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/cucumber-cpp/install
```

### Test hooks not firing

Ensure you built with `-DENABLE_TEST_HOOKS=ON`:
```bash
# Check if hooks are enabled
grep "ENABLE_TEST_HOOKS" build/CMakeCache.txt
```

### Application not starting in tests

Check logs in the test output:
```bash
./build/acceptance_test/cucumber_tests --format pretty 2>&1 | grep TEST_HOOK
```

## Performance Considerations

- Test hooks have **zero runtime overhead** in production builds
- Test execution is slower due to thread synchronization and event tracking
- Each scenario runs with a fresh application instance for isolation

## Best Practices

1. **Keep scenarios independent** - Each scenario should set up its own state
2. **Use descriptive names** - Feature/scenario names should clearly indicate what's being tested
3. **Avoid implementation details** - Focus on behavior, not internal structure
4. **Add hooks sparingly** - Only add hooks for genuinely observable events
5. **Clean up in After hooks** - Ensure application stops cleanly between scenarios

## Related Documentation

- [SIGHUP Config Reload](../docs/SIGHUP_CONFIG_RELOAD.md)
- [Module Template](../docs/MODULE_TEMPLATE.md)
- [Cucumber-cpp Documentation](https://github.com/cucumber/cucumber-cpp)
- [Gherkin Syntax Reference](https://cucumber.io/docs/gherkin/reference/)
