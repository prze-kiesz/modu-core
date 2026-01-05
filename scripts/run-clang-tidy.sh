#!/bin/bash
# Copyright (c) 2026 Przemek Kieszkowski
# SPDX-License-Identifier: BSD-2-Clause
#
# Run clang-tidy using compile_commands.json from CMake

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

echo -e "${YELLOW}Running clang-tidy on modu-core...${NC}"

# Check if build directory exists
if [ ! -d "${BUILD_DIR}" ]; then
    echo -e "${RED}Build directory not found. Running cmake...${NC}"
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    cmake -DCMAKE_BUILD_TYPE=Debug ..
fi

# Check if compile_commands.json exists
if [ ! -f "${BUILD_DIR}/compile_commands.json" ]; then
    echo -e "${RED}compile_commands.json not found. Running cmake...${NC}"
    cd "${BUILD_DIR}"
    cmake -DCMAKE_BUILD_TYPE=Debug ..
fi

echo -e "${GREEN}Using compile_commands.json from: ${BUILD_DIR}${NC}"
echo ""

# Run clang-tidy using run-clang-tidy tool (comes with LLVM)
# This automatically uses compile_commands.json
run-clang-tidy-19 -p "${BUILD_DIR}" -quiet

echo ""
echo -e "${GREEN}✓ clang-tidy completed${NC}"
echo -e "${GREEN}Found $(echo "${SOURCE_FILES}" | wc -l) files to check${NC}"

# Run clang-tidy
cd "${BUILD_DIR}"
echo "${SOURCE_FILES}" | xargs clang-tidy -p . --quiet

echo -e "${GREEN}clang-tidy completed${NC}"
