# Double SIGINT (Ctrl-C) Integration Test

## Overview

This integration test validates the double Ctrl-C (SIGINT) functionality in the comm_terminate module, which allows users to:
1. **First Ctrl-C**: Initiate graceful shutdown with time for cleanup
2. **Second Ctrl-C**: Force immediate termination if cleanup takes too long

## Quick Start

```bash
./run_test_double_sigint.sh
```

## What Gets Tested

The integration test validates:

1. **First SIGINT Handling**: Process receives SIGINT and starts graceful shutdown
2. **Continued Execution**: After first SIGINT, cleanup code continues running
3. **Second SIGINT Detection**: Signal handler detects the second SIGINT
4. **Immediate Termination**: Process terminates immediately via `std::_Exit(130)`
5. **Exit Code**: Process exits with code 130 (128 + SIGINT)
6. **No Complete Cleanup**: Cleanup loop does NOT finish (interrupted mid-way)

## Test Workflow

The `run_test_double_sigint.sh` script:

1. Builds the test executable if needed
2. Starts `modu-core-comm_terminate_integration_test_double_sigint` in background
3. Waits 2 seconds for initialization
4. Sends first SIGINT signal (graceful shutdown initiated)
5. Waits 0.5 seconds and verifies process is still running
6. Sends second SIGINT signal (force termination)
7. Validates results:
   - Process terminates immediately (within milliseconds)
   - Exit code is 130 (terminated by SIGINT)
   - First SIGINT was received and logged
   - Second SIGINT was received and logged
   - Cleanup did NOT complete (was interrupted)
   - No ERROR messages in log

## Expected Output

```
========================================
Double SIGINT Integration Test
========================================

Starting test...
Log file: /tmp/comm_terminate_double_sigint_test.log

Test process started with PID: 12345

Test 1: Sending first SIGINT (graceful shutdown)
✓ First SIGINT sent successfully

✓ Process still running (graceful shutdown in progress)

Test 2: Sending second SIGINT (force immediate termination)
✓ Second SIGINT sent successfully
✓ Process terminated by SIGINT immediately (3ms, exit code: 130)

Process was terminated by second SIGINT

========================================
Test Results
========================================
Graceful shutdown initiated: 1
First SIGINT received: 1
Second SIGINT received: 1
Cleanup started: 1
Cleanup completed normally: 0
Errors in log: 0

✓ Graceful shutdown was initiated
✓ First SIGINT was received and handled
✓ Second SIGINT received and forced termination
✓ Cleanup was interrupted by second SIGINT
✓ No errors in log

========================================
Log Output (last 40 lines)
========================================
[...log output showing both SIGINTs...]

========================================
✓ ALL TESTS PASSED (5/5)
========================================
```

## Manual Testing

To manually test the double SIGINT handler:

```bash
# Build the test
cd ../../../../build
make modu-core-comm_terminate_integration_test_double_sigint

# Run it
cd main/comm_terminate/integration_test
./modu-core-comm_terminate_integration_test_double_sigint &
PID=$!

# Wait a moment for startup
sleep 2

# Send first SIGINT (graceful shutdown)
kill -SIGINT $PID
# You should see: "First SIGINT received - starting graceful shutdown"
# Process continues running, cleanup in progress...

# Send second SIGINT (force exit)
kill -SIGINT $PID
# You should see: "Second SIGINT received - forcing immediate termination!"
# Process terminates immediately, exit code 130
```

## Implementation Details

### Signal Handler Thread Behavior

The signal handler thread uses an atomic flag to distinguish between first and second SIGINT:

1. **First SIGINT**:
   - Sets `m_first_sigint_received` atomic flag to `true`
   - Releases semaphore to unblock `WaitForTermination()`
   - **Continues waiting** for more signals (doesn't exit the loop)
   - Main thread begins graceful cleanup

2. **Second SIGINT**:
   - Detects `m_first_sigint_received` is already `true`
   - Logs warning about forced termination
   - Calls `std::_Exit(130)` for immediate process termination
   - **No destructors run** - instant exit

### Why std::_Exit()?

- `std::_Exit()` terminates immediately without calling destructors or atexit handlers
- This is intentional - user wants to force exit NOW
- Exit code 130 follows shell convention (128 + signal number)

### Thread Synchronization

- Signal handler thread continues running after first SIGINT
- Uses `sigwait()` to block and wait for second SIGINT
- No race conditions - atomic flag ensures proper detection
- Main thread is NOT joined in `WaitForTermination()` to allow continued signal handling

## Related Documentation

- [../../docs/SIGHUP_CONFIG_RELOAD.md](../../../../docs/SIGHUP_CONFIG_RELOAD.md) - Configuration reload pattern
- [README.md](README.md) - SIGHUP integration tests
- [../interface/comm_terminate.h](../interface/comm_terminate.h) - API documentation

## Common Issues

### Test Fails with "Process still running after second SIGINT"

- The signal handler thread may have exited prematurely
- Check that `continue` is called after first SIGINT (not setting `should_terminate`)
- Verify atomic flag logic is correct

### Test Fails with "Graceful shutdown completed normally"

- Second SIGINT was not sent quickly enough
- Process completed cleanup before second SIGINT arrived
- Adjust timing in test script (reduce delay before second SIGINT)

### Exit Code is 0 Instead of 130

- Second SIGINT was not received by signal handler
- Check that signal handler thread is still running after first SIGINT
- Verify `std::_Exit(130)` is called on second SIGINT
