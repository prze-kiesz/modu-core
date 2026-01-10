# comm_terminate Integration Tests

## Overview

Integration tests for the comm_terminate module's SIGHUP configuration reload functionality.

## Quick Start

Run all integration tests:

```bash
./run_test.sh
```

## What Gets Tested

The integration test validates:

1. **SIGHUP Signal Handling**: Process receives and handles SIGHUP without terminating
2. **Multiple Config Reloads**: Multiple SIGHUP signals can be sent and processed
3. **Listener Invocation**: Registered listeners are called in event processor thread
4. **Thread Safety**: Event queue and listeners work correctly across threads
5. **Graceful Shutdown**: SIGTERM triggers proper cleanup and termination
6. **systemd Integration**: READY/RELOADING status notifications work correctly

## Test Workflow

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
