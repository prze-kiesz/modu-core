#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

set -e

# Script to run BDD acceptance tests with Cucumber-cpp wireserver

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WIRESERVER="$SCRIPT_DIR/../../build-test/test_bdd_cucumber/modu-core-test_bdd_cucumber"
WIRESERVER_PORT=3902
FEATURES_DIR="$SCRIPT_DIR/features"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "Starting Cucumber-cpp wireserver on port $WIRESERVER_PORT..."

# Start wireserver in background
$WIRESERVER -p $WIRESERVER_PORT &
WIRESERVER_PID=$!

# Wait for wireserver to start
sleep 2

# Check if wireserver is running
if ! kill -0 $WIRESERVER_PID 2>/dev/null; then
    echo -e "${RED}Failed to start wireserver${NC}"
    exit 1
fi

echo -e "${GREEN}Wireserver started (PID: $WIRESERVER_PID)${NC}"

# Run Cucumber tests
echo "Running Cucumber tests..."
cd "$SCRIPT_DIR"
cucumber features/ --format pretty --publish-quiet

CUCUMBER_EXIT=$?

# Stop wireserver
echo "Stopping wireserver..."
kill $WIRESERVER_PID 2>/dev/null || true
wait $WIRESERVER_PID 2>/dev/null || true

if [ $CUCUMBER_EXIT -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
else
    echo -e "${RED}Tests failed with exit code $CUCUMBER_EXIT${NC}"
fi

exit $CUCUMBER_EXIT
