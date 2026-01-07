#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

###############################################################################
# Integration test runner for comm_terminate SIGHUP handler
###############################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Navigate up from L5_Common/comm_terminate/integration_test to project root
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
TEST_EXECUTABLE="${BUILD_DIR}/main/comm_terminate/integration_test/modu-core-comm_terminate_integration_test"
LOG_FILE="/tmp/comm_terminate_integration_test.log"
TIMEOUT=15

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}comm_terminate Integration Test Runner${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if test executable exists, if not build it
if [ ! -f "$TEST_EXECUTABLE" ]; then
    echo -e "${YELLOW}Test executable not found. Building...${NC}"
    cd "$BUILD_DIR"
    make modu-core-comm_terminate_integration_test
    if [ $? -ne 0 ]; then
        echo -e "${RED}✗ Build failed${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Build successful${NC}"
    echo ""
fi

# Clean up old log
rm -f "$LOG_FILE"

echo -e "${BLUE}Starting test...${NC}"
echo "Log file: $LOG_FILE"
echo ""

# Run test in background
"$TEST_EXECUTABLE" > "$LOG_FILE" 2>&1 &
TEST_PID=$!

echo -e "Test process started with PID: ${YELLOW}$TEST_PID${NC}"

# Give the test time to initialize
sleep 2

# Test 1: Send SIGHUP to trigger config reload
echo -e "\n${BLUE}Test 1: Sending SIGHUP (config reload)${NC}"
kill -SIGHUP $TEST_PID
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ SIGHUP sent successfully${NC}"
else
    echo -e "${RED}✗ Failed to send SIGHUP${NC}"
    kill -9 $TEST_PID 2>/dev/null
    exit 1
fi

sleep 2

# Test 2: Send another SIGHUP to test multiple reloads
echo -e "\n${BLUE}Test 2: Sending second SIGHUP${NC}"
kill -SIGHUP $TEST_PID
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Second SIGHUP sent successfully${NC}"
else
    echo -e "${RED}✗ Failed to send second SIGHUP${NC}"
    kill -9 $TEST_PID 2>/dev/null
    exit 1
fi

sleep 2

# Test 3: Graceful termination with SIGTERM
echo -e "\n${BLUE}Test 3: Sending SIGTERM (graceful shutdown)${NC}"
kill -SIGTERM $TEST_PID
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ SIGTERM sent successfully${NC}"
else
    echo -e "${RED}✗ Failed to send SIGTERM${NC}"
    kill -9 $TEST_PID 2>/dev/null
    exit 1
fi

# Wait for process to terminate
wait $TEST_PID 2>/dev/null
EXIT_CODE=$?

echo -e "\nProcess exited with code: ${YELLOW}$EXIT_CODE${NC}"

# Analyze results
echo -e "\n${BLUE}========================================${NC}"
echo -e "${BLUE}Test Results${NC}"
echo -e "${BLUE}========================================${NC}"

# Count config reloads
RELOAD_COUNT=$(grep -c "Config reload listener invoked" "$LOG_FILE" || echo "0")
echo -e "Config reload invocations: ${YELLOW}$RELOAD_COUNT${NC}"

# Check for expected patterns
ERRORS=$(grep -c "ERROR" "$LOG_FILE" 2>/dev/null || echo "0")
WARNINGS=$(grep -c "WARNING" "$LOG_FILE" 2>/dev/null || echo "0")

echo -e "Errors in log: ${YELLOW}$ERRORS${NC}"
echo -e "Warnings in log: ${YELLOW}$WARNINGS${NC}"

# Validate test results
SUCCESS=true

if [ "$RELOAD_COUNT" -lt 2 ]; then
    echo -e "${RED}✗ Expected at least 2 config reloads, got $RELOAD_COUNT${NC}"
    SUCCESS=false
fi

if [ "$ERRORS" -gt 0 ]; then
    echo -e "${RED}✗ Unexpected errors in log${NC}"
    SUCCESS=false
fi

if [ "$EXIT_CODE" -ne 0 ]; then
    echo -e "${RED}✗ Test process exited with non-zero code: $EXIT_CODE${NC}"
    SUCCESS=false
fi

# Check for critical log entries
if grep -q "Config reload event processed" "$LOG_FILE"; then
    echo -e "${GREEN}✓ Event processor handled config reload${NC}"
else
    echo -e "${RED}✗ Event processor did not process config reload${NC}"
    SUCCESS=false
fi

if grep -q "Application daemon shutting down" "$LOG_FILE"; then
    echo -e "${GREEN}✓ Graceful shutdown executed${NC}"
else
    echo -e "${RED}✗ Graceful shutdown not detected${NC}"
    SUCCESS=false
fi

# Display log summary
echo -e "\n${BLUE}========================================${NC}"
echo -e "${BLUE}Log Output (last 30 lines)${NC}"
echo -e "${BLUE}========================================${NC}"
tail -30 "$LOG_FILE"

# Final result
echo -e "\n${BLUE}========================================${NC}"
if [ "$SUCCESS" = true ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED${NC}"
    echo -e "${BLUE}========================================${NC}"
    exit 0
else
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    echo "Full log available at: $LOG_FILE"
    exit 1
fi
