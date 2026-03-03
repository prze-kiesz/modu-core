<!--
Copyright (c) 2026 Przemek Kieszkowski
SPDX-License-Identifier: BSD-2-Clause
-->

# Project Context - modu-core

## About the Project
`modu-core` - Core/kernel project

## General Information
- **Created:** January 2026
- **License:** See LICENSE file
- **Status:** In development

## Directory Structure
```
modu-core/
├── L0_AcceptanceTests/          # Acceptance tests (black-box, BDD)
│   └── test_pytest/             # pytest-bdd test suite
│       ├── application_lifecycle/  # Startup/shutdown scenarios
│       ├── configuration/          # Config reload scenarios
│       ├── helpers/                # LogWatcher, shared utilities
│       ├── conftest.py             # Shared BDD step definitions
│       └── pytest.ini              # pytest configuration
├── L1_Presentation/             # Layer 1: UI/APIs (planned)
├── L2_Services/                 # Layer 2: Business logic (planned)
├── L3_Storage/                  # Layer 3: Data access (planned)
├── L4_Infrastructure/           # Layer 4: Cross-cutting concerns
│   └── infr_main/               # Application entry infrastructure
├── L5_Common/                   # Layer 5: Shared utilities
│   ├── comm_config-toml/        # TOML config (XDG + SIGHUP reload)
│   ├── comm_main/               # Main loop / signal handling
│   └── comm_terminate/          # Graceful shutdown logic
├── main/                        # Final binary assembly (main.cpp)
├── build/                       # Release/Debug build artifacts
├── build-test/                  # Test build artifacts
├── .llm/                        # AI guidelines and context
├── .github/workflows/           # GitHub Actions CI
└── scripts/                     # clang-tidy, helpers
```

## Technical Stack

- **Language**: C++20
- **Build system**: CMake 3.28+, modular `-config.cmake` pattern
- **Testing (unit)**: GoogleTest + glog (`google::InitGoogleLogging`)
- **Testing (acceptance)**: pytest 9.0.2 + pytest-bdd 8.1.0 (Python 3.12)
- **Logging**: Google glog (`LOG(INFO)`, `LOG(WARNING)`, `LOG(ERROR)`)
- **Config**: TOML via `toml++`, loaded via XDG hierarchy
- **CI**: GitHub Actions, matrix `[Debug, Release]`
- **Devcontainer**: Ubuntu 24.04, image `ghcr.io/prze-kiesz/modu-core:latest`
- **Static analysis**: clang-tidy, clang-format

## Key Features

- **Layered modular architecture** — L0 (tests) through L5 (common), strict one-direction dependency rule
- **TOML config with XDG hierarchy** — `/etc/<app>/config.toml` overridden by `~/.config/<app>/config.toml`
- **SIGHUP hot-reload** — send `kill -HUP <pid>` to reload config without restart
- **CLI overrides** — `--set key.subkey=value` at highest priority
- **Graceful shutdown** — SIGTERM/SIGINT handled, modules tear down cleanly
- **BDD acceptance tests** — human-readable `.feature` files drive black-box tests

## Development Setup

```bash
# Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Unit tests
ctest --test-dir build --output-on-failure

# Acceptance tests (requires built binary)
MODU_CORE_BINARY=./build/main/modu-core pytest L0_AcceptanceTests/test_pytest/ -v

# Full test build (separate CMake preset)
cmake -S . -B build-test -DBUILD_TESTS=ON
cmake --build build-test -j$(nproc)
ctest --test-dir build-test --output-on-failure
```

## Known Issues

- L1–L3 layers are planned but currently empty (framework skeleton only)
- `build/` (non-test) does not include `L0_AcceptanceTests` ctest target yet

## Attribution and Copyright
**Copyright (c) 2026 Przemek Kieszkowski**

This project is the intellectual property of Przemek Kieszkowski. All knowledge, patterns, and design decisions represent original work and research.

For detailed attribution requirements for AI models and derivative works, see `ATTRIBUTION.md` in the repository root.

**License:** BSD-2-Clause - See LICENSE file
