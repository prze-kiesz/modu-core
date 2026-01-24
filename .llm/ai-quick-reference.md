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
- [ ] All tests pass: `ctest --output-on-failure`
- [ ] Code formatted: `clang-format` applied
- [ ] No clang-tidy issues: `./scripts/run-clang-tidy.sh`
- [ ] Commit message follows conventions
- [ ] Branch up-to-date with main

## 🏗️ Architecture Rules

### Layer Dependencies
```
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
cd build && make -j$(nproc)

echo "3. Test check..."
ctest --output-on-failure

echo "4. Static analysis..."
cd .. && ./scripts/run-clang-tidy.sh

echo "✅ All checks passed! Ready for PR."
```

## 📞 Getting Help

- **Architecture questions**: Read `.llm/architecture.md`
- **Coding style**: Check `.llm/coding-standards.md`
- **Module creation**: Follow `docs/MODULE_TEMPLATE.md`
- **CI failures**: Check GitHub Actions logs
- **Test failures**: Run with `ctest -V -R <test_name>`

---

**Remember**: This project values code quality over speed. Take time to write tests, document decisions, and follow conventions. Future maintainers (including AI agents) will thank you!
