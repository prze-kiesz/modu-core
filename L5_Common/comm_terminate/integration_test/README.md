# comm_terminate Integration Tests

## Overview

Integration tests for the comm_terminate module, covering:
1. **SIGHUP Configuration Reload** - Reload config without restart
2. **Double SIGINT (Ctrl-C)** - Graceful shutdown with force-exit option

## Quick Start

Run all integration tests:

```bash
# SIGHUP config reload test
./run_test.sh

# Double Ctrl-C test
./run_test_double_sigint.sh
```

## Test Suites

### 1. SIGHUP Configuration Reload Test

Tests the listener pattern for configuration reload without process restart.

**What Gets Tested:**

1. **SIGHUP Signal Handling**: Process receives and handles SIGHUP without terminating
2. **Multiple Config Reloads**: Multiple SIGHUP signals can be sent and processed
3. **Listener Invocation**: Registered listeners are called in event processor thread
4. **Thread Safety**: Event queue and listeners work correctly across threads
5. **Graceful Shutdown**: SIGTERM triggers proper cleanup and termination
6. **systemd Integration**: READY/RELOADING status notifications work correctly

**See:** [Full SIGHUP test documentation](#sighup-test-details) below

### 2. Double SIGINT (Ctrl-C) Test

Tests that first Ctrl-C initiates graceful shutdown, second Ctrl-C forces immediate exit.

**What Gets Tested:**
1. **First SIGINT**: Initiates graceful shutdown, cleanup continues
2. **Second SIGINT**: Forces immediate termination via `std::_Exit(130)`
3. **Thread Behavior**: Signal handler continues waiting after first SIGINT
4. **Exit Code**: Process exits with code 130 (128 + SIGINT)
5. **Interrupted Cleanup**: Cleanup does NOT complete (interrupted by second SIGINT)

**See:** [README_DOUBLE_SIGINT.md](README_DOUBLE_SIGINT.md) for full documentation

---

## SIGHUP Test Details

### Test Workflow

The `run_test.sh` script:

1. Builds the test executable if needed
2. Starts `modu-core-comm_terminate_integration_test` in background
3. Waits 2 seconds for initialization
4. Sends SIGHUP signal (first config reload)
5. Waits 2 seconds
6. Sends SIGHUP signal (second config reload)
7. Waits 2 seconds
8. Sends SIGTERM signal (graceful termination)
9. Validates results:
   - At least 2 config reload invocations
   - No ERROR messages in log
   - Process exits with code 0
   - Event processor handled events
   - Graceful shutdown executed

## Expected Output

```
========================================
comm_terminate Integration Test Runner
========================================

Starting test...
Log file: /tmp/comm_terminate_integration_test.log

Test process started with PID: 12345

Test 1: Sending SIGHUP (config reload)
✓ SIGHUP sent successfully

Test 2: Sending second SIGHUP
✓ Second SIGHUP sent successfully

Test 3: Sending SIGTERM (graceful shutdown)
✓ SIGTERM sent successfully

Process exited with code: 0

========================================
Test Results
========================================
Config reload invocations: 2
Errors in log: 0
Warnings in log: 1
✓ Event processor handled config reload
✓ Graceful shutdown executed

========================================
Log Output (last 30 lines)
========================================
[...log output...]

========================================
✓ ALL TESTS PASSED
========================================
```

## Manual Testing

To manually test the SIGHUP handler:

```bash
# Build the test
cd ../../../../build
make modu-core-comm_terminate_integration_test

# Run it
cd main/comm_terminate/integration_test
./modu-core-comm_terminate_integration_test &
PID=$!

# In another terminal, send signals
kill -SIGHUP $PID    # Trigger config reload
kill -SIGHUP $PID    # Trigger another reload
kill -SIGTERM $PID   # Graceful shutdown
```

## Files

- `test_sighup.cpp` - Integration test executable source
- `run_test.sh` - Automated test runner script
- `CMakeLists.txt` - Build configuration

## Troubleshooting

### Test Hangs

If the test hangs, kill it:
```bash
pkill -9 modu-core-comm_terminate_integration_test
```

### Build Errors

Ensure dependencies are installed:
```bash
sudo apt-get install libsystemd-dev libgoogle-glog-dev
```

### Permission Denied

Make script executable:
```bash
chmod +x run_test.sh
```

## Integration with CI/CD

The test can be run in CI/CD pipelines:

```yaml
# Example GitHub Actions
- name: Run Integration Tests
  run: |
    cd L5_Common/comm_terminate/integration_test
    ./run_test.sh
```

Exit code 0 indicates success, non-zero indicates failure.
