#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

# Integration test for double SIGINT (Ctrl-C) handling
# Tests that first Ctrl-C starts graceful shutdown, second Ctrl-C forces immediate termination

# Note: NOT using 'set -e' because we expect non-zero exit codes (130 from SIGINT)

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test configuration
TEST_NAME="Double SIGINT Integration Test"
LOG_FILE="/tmp/comm_terminate_double_sigint_test.log"
# Use absolute path to the test executable
TEST_EXECUTABLE="/workspaces/modu-core/build/main/comm_terminate/integration_test/modu-core-comm_terminate_integration_test_double_sigint"

# Print header
echo "========================================"
echo "$TEST_NAME"
echo "========================================"
echo ""

# Check if executable exists
if [ ! -f "$TEST_EXECUTABLE" ]; then
    echo -e "${RED}✗ Test executable not found: $TEST_EXECUTABLE${NC}"
    echo "Please build the project first:"
    echo "  cd ../../../../build"
    echo "  make modu-core-comm_terminate_integration_test_double_sigint"
    exit 1
fi

# Clean up old log file
rm -f "$LOG_FILE"

echo "Starting test..."
echo "Log file: $LOG_FILE"
echo ""

# Start the test process in background
$TEST_EXECUTABLE > "$LOG_FILE" 2>&1 &
TEST_PID=$!

echo "Test process started with PID: $TEST_PID"
echo ""

# Wait for initialization
sleep 2

# Test 1: Send first SIGINT (graceful shutdown)
echo "Test 1: Sending first SIGINT (graceful shutdown)"
if kill -SIGINT $TEST_PID 2>/dev/null; then
    echo -e "${GREEN}✓${NC} First SIGINT sent successfully"
else
    echo -e "${RED}✗${NC} Failed to send first SIGINT"
    exit 1
fi
echo ""

# Wait BRIEFLY to ensure graceful shutdown started (not too long!)
sleep 0.5

# Verify process is still running (graceful shutdown with delay)
if kill -0 $TEST_PID 2>/dev/null; then
    echo -e "${GREEN}✓${NC} Process still running (graceful shutdown in progress)"
else
    echo -e "${RED}✗${NC} Process terminated too quickly after first SIGINT"
    echo "Expected: Process should continue running during graceful shutdown"
    exit 1
fi
echo ""

# Test 2: Send second SIGINT (immediate termination)
echo "Test 2: Sending second SIGINT (force immediate termination)"
BEFORE_TIME=$(date +%s%N)  # Nanoseconds for more precision

if kill -SIGINT $TEST_PID 2>/dev/null; then
    echo -e "${GREEN}✓${NC} Second SIGINT sent successfully"
else
    echo -e "${YELLOW}⚠${NC} Process already terminated (race condition)"
fi

# Wait for process to actually terminate and get exit code
wait $TEST_PID 2>/dev/null
EXIT_CODE=$?

# Handle special case: exit code 130 from wait means process was killed by SIGINT
# This is actually what we want!
if [ $EXIT_CODE -eq 130 ]; then
    # Process was terminated by SIGINT - this is success for our test
    true  # Continue with analysis
fi

AFTER_TIME=$(date +%s%N)
ELAPSED_NS=$((AFTER_TIME - BEFORE_TIME))
ELAPSED_MS=$((ELAPSED_NS / 1000000))

# Exit code 130 means terminated by SIGINT (128 + 2)
if [ $EXIT_CODE -eq 130 ] || [ $EXIT_CODE -eq 2 ]; then
    echo -e "${GREEN}✓${NC} Process terminated by SIGINT immediately (${ELAPSED_MS}ms, exit code: $EXIT_CODE)"
elif [ $EXIT_CODE -eq 0 ]; then
    echo -e "${RED}✗${NC} Process exited normally (graceful shutdown completed)"
    echo "Expected: Immediate termination by second SIGINT"
    # Don't exit yet - let's check the logs
else
    echo -e "${GREEN}✓${NC} Process terminated (${ELAPSED_MS}ms, exit code: $EXIT_CODE)"
fi
echo ""

# Don't need to wait again at the end - process already terminated
PROCESS_TERMINATED=1

# Wait for process to fully exit and get exit code (if not already done)
if [ -z "$PROCESS_TERMINATED" ]; then
    wait $TEST_PID 2>/dev/null
    EXIT_CODE=$?
    echo "Process exited with code: $EXIT_CODE"
else
    echo "Process was terminated by second SIGINT"
fi
echo ""

# Analyze log file
echo "========================================"
echo "Test Results"
echo "========================================"

# Check for key log messages
GRACEFUL_SHUTDOWN=$(grep -c "Graceful shutdown initiated" "$LOG_FILE" || true)
FIRST_SIGINT=$(grep -c "First SIGINT received" "$LOG_FILE" || true)
SECOND_SIGINT=$(grep -c "Second SIGINT received" "$LOG_FILE" || true)
CLEANUP_STARTED=$(grep -c "Simulating cleanup work" "$LOG_FILE" || true)
CLEANUP_COMPLETED=$(grep -c "Graceful shutdown completed normally" "$LOG_FILE" || true)
ERRORS=$(grep -c "ERROR" "$LOG_FILE" || true)

echo "Graceful shutdown initiated: $GRACEFUL_SHUTDOWN"
echo "First SIGINT received: $FIRST_SIGINT"
echo "Second SIGINT received: $SECOND_SIGINT"
echo "Cleanup started: $CLEANUP_STARTED"
echo "Cleanup completed normally: $CLEANUP_COMPLETED"
echo "Errors in log: $ERRORS"
echo ""

# Verify results
TESTS_PASSED=0
TESTS_FAILED=0

# Test: Graceful shutdown was initiated
if [ "$GRACEFUL_SHUTDOWN" -ge 1 ]; then
    echo -e "${GREEN}✓${NC} Graceful shutdown was initiated"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗${NC} Graceful shutdown was NOT initiated"
    ((TESTS_FAILED++))
fi

# Test: First SIGINT was received and handled
if [ "$FIRST_SIGINT" -ge 1 ]; then
    echo -e "${GREEN}✓${NC} First SIGINT was received and handled"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗${NC} First SIGINT was NOT received"
    ((TESTS_FAILED++))
fi

# Test: Second SIGINT was received and forced termination
if [ "$SECOND_SIGINT" -ge 1 ]; then
    echo -e "${GREEN}✓${NC} Second SIGINT received and forced termination"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗${NC} Second SIGINT was NOT received"
    ((TESTS_FAILED++))
fi

# Test: Cleanup was started but NOT completed (interrupted by second SIGINT)
if [ "$CLEANUP_STARTED" -ge 1 ] && [ "$CLEANUP_COMPLETED" -eq 0 ]; then
    echo -e "${GREEN}✓${NC} Cleanup was interrupted by second SIGINT"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗${NC} Expected cleanup to start but not complete"
    echo "   Cleanup started: $CLEANUP_STARTED, Cleanup completed: $CLEANUP_COMPLETED"
    ((TESTS_FAILED++))
fi

# Test: No errors in log
if [ "$ERRORS" -eq 0 ]; then
    echo -e "${GREEN}✓${NC} No errors in log"
    ((TESTS_PASSED++))
else
    echo -e "${YELLOW}⚠${NC} Found $ERRORS error(s) in log"
fi

echo ""
echo "========================================"
echo "Log Output (last 40 lines)"
echo "========================================"
tail -n 40 "$LOG_FILE"
echo ""

# Final result
echo "========================================"
if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED${NC} ($TESTS_PASSED/$((TESTS_PASSED + TESTS_FAILED)))"
    echo "========================================"
    exit 0
else
    echo -e "${RED}✗ SOME TESTS FAILED${NC} (Passed: $TESTS_PASSED, Failed: $TESTS_FAILED)"
    echo "========================================"
    exit 1
fi
