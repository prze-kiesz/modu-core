<!--
Copyright (c) 2026 Przemek Kieszkowski
SPDX-License-Identifier: BSD-2-Clause
-->

# AI Quick Reference - modu-core Development

## 🚀 Essential Commands

### Build System
```bash
# Initial configuration (from project root)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build entire project (parallel)
cd build && make -j$(nproc)

# Build specific target
cd build && make modu-core-<module_name>

# Run all tests
cd build && ctest --output-on-failure

# Run specific test
cd build && ctest -R <test_name> --output-on-failure

# Clean and rebuild
cd build && make clean && make -j$(nproc)
```

### Git Workflow
```bash
# Branch naming conventions
feature/<name>   # New features
fix/<name>       # Bug fixes  
docs/<name>      # Documentation
refactor/<name>  # Code refactoring
test/<name>      # Test additions

# Check branch status before work
git fetch origin
git status

# Commit conventions (Conventional Commits)
git commit -m "feat(<scope>): add new feature"
git commit -m "fix(<scope>): resolve issue"
git commit -m "refactor(<scope>): improve code structure"
git commit -m "docs(<scope>): update documentation"
git commit -m "test(<scope>): add unit tests"

# Check CI status after push
git push origin <branch>
# Then check: https://github.com/prze-kiesz/modu-core/actions
```

## 📁 Project Structure Patterns

### New Module Skeleton
```
L[X]_<Layer>/
└── <module_name>/
    ├── CMakeLists.txt              # Build configuration
    ├── <module_name>-config.cmake  # Module discovery
    ├── interface/
    │   └── <module_name>.h         # Public API
    ├── src/
    │   └── <module_name>.cpp       # Implementation
    ├── unit_test/
    │   ├── CMakeLists.txt
    │   └── <module_name>_test.cpp
    └── README.md                   # Optional documentation
```

### CMakeLists.txt Template
```cmake
set(MODULE_NAME "module_name")
set(MODULE_TARGET "${PROJECT_NAME}-${MODULE_NAME}")

set(MODULE_SOURCES src/module_name.cpp)
set(MODULE_HEADERS interface/module_name.h)

add_library(${MODULE_TARGET} OBJECT ${MODULE_SOURCES} ${MODULE_HEADERS})
target_compile_features(${MODULE_TARGET} PUBLIC cxx_std_20)

target_include_directories(${MODULE_TARGET}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(BUILD_TESTING)
  add_subdirectory(unit_test)
endif()
```

## 🧪 Testing Patterns

### Unit Test Structure
```cpp
#include "<module_name>.h"
#include <gtest/gtest.h>

namespace <namespace> {

class ModuleTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup before each test
  }
  void TearDown() override {
    // Cleanup after each test
  }
};

TEST_F(ModuleTest, DescriptiveTestName) {
  // Arrange
  auto& instance = Module::Instance();
  
  // Act
  auto result = instance.DoSomething();
  
  // Assert
  EXPECT_TRUE(result);
}

} // namespace
```

### Running Tests Efficiently
```bash
# Run only failed tests
cd build && ctest --rerun-failed --output-on-failure

# Run with verbose output
cd build && ctest -V -R <test_pattern>

# Generate test coverage (if enabled)
cd build && make coverage
```

### Unit Test Custom main() (required for glog)
```cpp
// unit_test/<module>_test.cpp
#include <glog/logging.h>
#include <gtest/gtest.h>

// ... test fixtures and TEST_F() ...

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  google::ShutdownGoogleLogging();
  return result;
}
```
```cmake
# unit_test/CMakeLists.txt — use GTest::gtest, NOT GTest::gtest_main
target_link_libraries(${TEST_TARGET} GTest::gtest glog::glog ...)
```

## 🎭 Acceptance Tests (pytest-bdd)

Acceptance tests live in `L0_AcceptanceTests/test_pytest/` and are **black-box**
tests that run the compiled `modu-core` binary.

### Directory Structure
```
L0_AcceptanceTests/test_pytest/
├── pytest.ini                    # log_cli = true, log_cli_level = INFO
├── requirements.txt              # pytest-bdd, pytest-timeout
├── conftest.py                   # shared BDD step definitions
├── helpers/
│   ├── __init__.py
│   └── log_watcher.py           # LogWatcher class
├── application_lifecycle/
│   ├── features/startup_shutdown.feature
│   └── test_startup_shutdown.py
└── configuration/
    ├── features/config_reload.feature
    └── test_config_reload.py
```

### Running Acceptance Tests
```bash
# Requires binary — use build-test output or build output
MODU_CORE_BINARY=./build-test/main/modu-core \
  pytest L0_AcceptanceTests/test_pytest/ -v

# Run single functional area
pytest L0_AcceptanceTests/test_pytest/configuration/ -v

# Run matching test name
pytest L0_AcceptanceTests/test_pytest/ -v -k "reload"

# Show live logs (already on by default via pytest.ini)
pytest L0_AcceptanceTests/test_pytest/ -v -s
```

### Writing a New Acceptance Test

1. **Add a scenario to a `.feature` file** (or create a new one):
```gherkin
# L0_AcceptanceTests/test_pytest/configuration/features/config_reload.feature
Scenario: My new scenario
  Given the application is running
  When I do something
  Then the log contains "Expected message"
```

2. **Implement steps in `test_<area>.py`**:
```python
from pytest_bdd import scenario, given, when, then
from pathlib import Path
import logging

_log = logging.getLogger(__name__)

@scenario("features/config_reload.feature", "My new scenario")
def test_my_new_scenario():
    pass

@when("I do something")
def do_something(app):
    # app is the RunningApp fixture from conftest.py
    mark = app.logs.mark()         # snapshot current log position
    app.do_action()
    app.logs.wait_for("Expected message", from_mark=mark, timeout=5.0)
```

3. **Use `LogWatcher` correctly** — the `from_mark` parameter is critical
   when checking logs that must appear AFTER an event (e.g., SIGHUP reload):
```python
# WRONG — may match startup logs
app.logs.wait_for("Configuration initialized", timeout=5.0)

# CORRECT — only matches lines after the reload signal
mark = app.logs.mark()
os.kill(app.pid, signal.SIGHUP)
app.logs.wait_for("Configuration initialized", from_mark=mark, timeout=5.0)
```

4. **Shared steps** (app is running, log contains, etc.) are in `conftest.py` —
   don't duplicate them.

### LogWatcher API
```python
from helpers.log_watcher import LogWatcher

watcher = LogWatcher(process)        # wraps a subprocess.Popen object
watcher.wait_for("pattern", timeout=5.0)            # searches from start
watcher.wait_for("pattern", from_mark=mark, timeout=5.0)  # after mark
mark = watcher.mark()               # returns current line index (snapshot)
watcher.get_all_lines()             # returns list of all lines seen so far
```

### Logging in Step Definitions
Add `_log.info()` calls so pytest live log output (`log_cli = true`) shows
what was matched:
```python
@then(parsers.parse('the log contains "{text}"'))
def log_contains(app, text):
    matched = app.logs.wait_for(text, timeout=5.0)
    _log.info("log match  | %s", matched)
```

## 🔍 Code Quality Checks

### Static Analysis
```bash
# Run clang-tidy on all files
./scripts/run-clang-tidy.sh

# Check specific file
clang-tidy <file>.cpp -- -I./interface -std=c++20

# Format code (auto-fix)
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

### Pre-commit Checklist
- [ ] Code builds without warnings: `make -j$(nproc)`
- [ ] Unit tests pass: `ctest --output-on-failure`
- [ ] Acceptance tests pass: `MODU_CORE_BINARY=./build-test/main/modu-core pytest L0_AcceptanceTests/test_pytest/ -v`
- [ ] Code formatted: `clang-format` applied
- [ ] No clang-tidy issues: `./scripts/run-clang-tidy.sh`
- [ ] Commit message follows conventions
- [ ] Branch up-to-date with main

## 🏗️ Architecture Rules

### Layer Dependencies
```
L0_AcceptanceTests (black-box — depends on compiled binary only)
     ↑ tests the binary from
L1_Presentation
  ↓ can depend on
L2_Services
  ↓ can depend on
L3_Storage
  ↓ can depend on
L4_Infrastructure
  ↓ can depend on
L5_Common
```

**NEVER**: Lower layers depending on higher layers  
**NEVER**: Circular dependencies between modules  
**NOTE**: L0 tests the compiled binary only — no source-level dependency on any layer

### Configuration System (XDG Hierarchy)
```cpp
// 1. Initialize in comm_main (automatic)
Config::Instance().Initialize("app-name");

// 2. Config files loaded in order (later overrides earlier):
//    - /etc/<app-name>/config.toml        (system defaults)
//    - ~/.config/<app-name>/config.toml   (user overrides)

// 3. CLI overrides (highest priority)
./modu-core --set key.subkey=value

// 4. Access config in any module
auto config = Config::Instance().Get<MyConfig>("section_name");

// 5. Reload on SIGHUP (automatic)
kill -HUP <pid>
```

### Error Handling Pattern
```cpp
// Define error enum
enum class ModuleError { Success = 0, FailedOperation, InvalidInput };

// Create error category
class ModuleErrorCategory : public std::error_category {
  const char* name() const noexcept override { return "module"; }
  std::string message(int ev) const override { /* ... */ }
};

// Use in functions
std::error_code DoOperation() {
  if (error_condition) {
    LOG(ERROR) << "Operation failed: reason";
    return make_error_code(ModuleError::FailedOperation);
  }
  return {}; // Success
}
```

## 🐛 Debugging Tips

### Common Issues
```bash
# 1. Build fails after git pull
cd build && cmake .. && make clean && make -j$(nproc)

# 2. Tests fail unexpectedly
# Check singleton state pollution between tests
# Use unique test fixtures/files per test

# 3. Configuration not loading
# Verify XDG_CONFIG_HOME and file paths
echo $XDG_CONFIG_HOME
ls -la ~/.config/<app-name>/

# 4. CI fails but local passes
# Check Docker environment differences
# Verify all dependencies in Dockerfile
```

### Useful Log Analysis
```bash
# Filter logs by level
./modu-core 2>&1 | grep "ERROR"
./modu-core 2>&1 | grep "WARNING"

# Follow config loading
./modu-core 2>&1 | grep "Config::"

# Check SIGHUP reload
kill -HUP $(pidof modu-core) && tail -f /tmp/log
```

## 📚 Key Files to Read First

1. **Architecture**: `.llm/architecture.md` - Layer structure, module patterns
2. **Coding Standards**: `.llm/coding-standards.md` - Naming, formatting rules
3. **Contributing**: `.github/CONTRIBUTING.md` - PR workflow, branch conventions
4. **Module Template**: `docs/MODULE_TEMPLATE.md` - How to create new modules
5. **SIGHUP Reload**: `docs/SIGHUP_CONFIG_RELOAD.md` - Config reload mechanics

## 🔄 Common Workflows

### Adding a New Module
```bash
# 1. Create directory structure
mkdir -p L5_Common/<module_name>/{interface,src,unit_test}

# 2. Create files from template (see docs/MODULE_TEMPLATE.md)
# - CMakeLists.txt
# - <module_name>-config.cmake  
# - interface/<module_name>.h
# - src/<module_name>.cpp
# - unit_test/CMakeLists.txt
# - unit_test/<module_name>_test.cpp

# 3. Register in parent CMakeLists.txt
echo 'find_package("<module_name>" REQUIRED)' >> CMakeLists.txt

# 4. Build and test
cd build && cmake .. && make -j$(nproc) && ctest -R <module_name>

# 5. Commit
git add L5_Common/<module_name>/
git commit -m "feat(<module_name>): add new module"
```

### Fixing a Bug
```bash
# 1. Create branch
git checkout -b fix/issue-description

# 2. Write failing test first
# Edit: <module>/unit_test/<module>_test.cpp

# 3. Run test to confirm failure
cd build && ctest -R <test_name>

# 4. Fix implementation
# Edit: <module>/src/<module>.cpp

# 5. Verify fix
cd build && make && ctest --output-on-failure

# 6. Commit and push
git commit -am "fix(<module>): resolve issue with detailed description"
git push origin fix/issue-description

# 7. Create PR on GitHub
```

### Updating Documentation
```bash
# Module-level docs
vim L5_Common/<module>/README.md

# Architecture docs  
vim .llm/architecture.md

# Contributing guide
vim .github/CONTRIBUTING.md

# Commit
git commit -m "docs(<scope>): update documentation for clarity"
```

## ⚡ Performance Tips

### Parallel Building
```bash
# Use all cores
make -j$(nproc)

# Limit to N cores (if system is slow)
make -j4
```

### Incremental Testing
```bash
# Run only tests for changed modules
cd build && ctest -R <pattern> --output-on-failure

# Skip long integration tests during development
cd build && ctest -E integration --output-on-failure
```

### Ccache (if available)
```bash
# Enable ccache for faster rebuilds
export CMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

## 🎯 Quick Checks Before Submitting PR

```bash
#!/bin/bash
# Save as: scripts/pre-pr-check.sh

set -e
cd "$(git rev-parse --show-toplevel)"

echo "1. Formatting check..."
find L*/ main/ -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run -Werror

echo "2. Build check..."
cd build-test && make -j$(nproc)

echo "3. Unit test check..."
ctest --output-on-failure

echo "4. Acceptance test check..."
cd ..
MODU_CORE_BINARY=./build-test/main/modu-core \
  pytest L0_AcceptanceTests/test_pytest/ -v

echo "5. Static analysis..."
./scripts/run-clang-tidy.sh

echo "✅ All checks passed! Ready for PR."
```

## 📞 Getting Help

- **Architecture questions**: Read `.llm/architecture.md`
- **Coding style**: Check `.llm/coding-standards.md`
- **Module creation**: Follow `docs/MODULE_TEMPLATE.md`
- **Acceptance tests**: Read `L0_AcceptanceTests/test_pytest/README.md`
- **CI failures**: Check GitHub Actions logs
- **Unit test failures**: Run with `ctest -V -R <test_name>`
- **Acceptance test failures**: Run with `pytest -v -s -k <test_name>` and check live logs

---

**Remember**: This project values code quality over speed. Take time to write tests, document decisions, and follow conventions. Future maintainers (including AI agents) will thank you!
