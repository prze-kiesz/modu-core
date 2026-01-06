# Contributing to modu-core

Thank you for your interest in contributing to **modu-core**! This document provides guidelines and instructions for contributing to the project.

## Development Workflow

All changes must go through pull requests (PRs). Direct commits to the `main` branch are not allowed.

### 1. Fork and Clone

```bash
# Fork the repository on GitHub, then clone your fork
git clone https://github.com/YOUR-USERNAME/modu-core.git
cd modu-core

# Add upstream remote
git remote add upstream https://github.com/prze-kiesz/modu-core.git
```

### 2. Create a Feature Branch

```bash
# Update your main branch
git checkout main
git pull upstream main

# Create a new feature branch
git checkout -b feature/your-feature-name
# or for bug fixes
git checkout -b fix/bug-description
```

**Branch Naming Conventions:**
- `feature/` - New features
- `fix/` - Bug fixes
- `docs/` - Documentation changes
- `refactor/` - Code refactoring
- `test/` - Test additions or updates
- `chore/` - Maintenance tasks

### 3. Make Your Changes

Follow the project's coding standards:

#### Architecture Rules
- Respect layer dependencies: Higher layers (L1) → Lower layers (L5) only
- No circular dependencies between modules
- Each module should be self-contained and independently testable

#### Code Style
- Modern C++20 features encouraged
- Use `std::error_code` for error handling
- RAII and smart pointers for resource management
- Clear, descriptive naming (follow existing conventions)
- Comprehensive documentation in header files

#### Testing Requirements
- All new code must have unit tests
- Tests should use Google Test framework
- Aim for high code coverage
- Tests must pass locally before submitting PR

### 4. Run Quality Checks

Before committing, ensure your code passes all checks:

```bash
# Build the project
cd build
cmake ..
make -j$(nproc)

# Run all tests
ctest --output-on-failure

# Run static analysis
cd ..
./scripts/run-clang-tidy.sh

# Check code formatting
find L*_*/ main/ -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
  -exec clang-format --dry-run --Werror {} \+
```

### 5. Commit Your Changes

Write clear, descriptive commit messages:

```bash
git add .
git commit -m "feat: add new module for data serialization

- Implement JSON serialization for configuration
- Add unit tests with 95% coverage
- Update documentation

Closes #123"
```

**Commit Message Format:**
```
<type>: <subject>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks
- `perf`: Performance improvements

### 6. Push and Create Pull Request

```bash
# Push to your fork
git push origin feature/your-feature-name

# Create pull request on GitHub
```

When creating a PR:
- Fill out the pull request template completely
- Link related issues
- Provide clear description of changes
- Add screenshots/logs if applicable
- Request reviews from maintainers

### 7. Code Review Process

**For Contributors:**
- All PRs require at least one approval from a maintainer
- CI/CD checks must pass:
  - ✅ Build succeeds (Debug and Release)
  - ✅ All tests pass
  - ✅ Static analysis clean (clang-tidy, cppcheck)
  - ✅ Code formatting correct (clang-format)
- Address reviewer feedback promptly
- Keep PRs focused and reasonably sized

**For Administrators:**
- Can merge own PRs after CI/CD checks pass
- No approval required (but CI/CD is mandatory)
- Same code quality standards apply

### 8. After Approval

Once approved and all checks pass:
- Maintainer will merge your PR using squash merge
- Your branch will be automatically deleted
- Update your local repository:

```bash
git checkout main
git pull upstream main
git push origin main
```

## Branch Protection Rules

The `main` branch is protected with the following rules:

- ✅ Require pull request before merging
  - Contributors: 1 approval required
  - Administrators: Can self-merge (no approval needed)
- ✅ Require status checks to pass (mandatory for all users):
  - Build and Test (Debug)
  - Build and Test (Release)
  - Static Analysis (clang-tidy)
  - Static Analysis (cppcheck)
  - Static Analysis (clang-format)
- ✅ Require conversation resolution before merging
- ✅ Require linear history (squash merging)
- ❌ Allow force pushes: Disabled
- ❌ Allow deletions: Disabled

## Adding a New Module

When adding a new module, follow this structure:

```bash
# Create module structure
mkdir -p L5_Common/new_module/{interface,src,unit_test}

# Create required files
touch L5_Common/new_module/CMakeLists.txt
touch L5_Common/new_module/new_module-config.cmake
touch L5_Common/new_module/interface/new_module.h
touch L5_Common/new_module/src/new_module.cpp
touch L5_Common/new_module/unit_test/CMakeLists.txt
touch L5_Common/new_module/unit_test/new_module_test.cpp
```

**Module Requirements:**
1. `CMakeLists.txt` - Build configuration
2. `*-config.cmake` - Integration interface
3. `interface/` - Public headers
4. `src/` - Implementation files
5. `unit_test/` - Comprehensive tests
6. Update root `CMakeLists.txt` to register module

## Reporting Issues

When reporting issues:
- Use GitHub Issues
- Provide clear, descriptive title
- Include reproduction steps
- Specify environment (OS, compiler, build type)
- Attach relevant logs/screenshots
- Label appropriately (bug, enhancement, question, etc.)

## Development Container

Use the provided development container for consistent environment:

```bash
# Pull latest dev container
docker pull ghcr.io/prze-kiesz/modu-core:latest

# Or rebuild locally
cd .devcontainer
docker build -t modu-core-dev .
```

## Getting Help

- **Discussions**: https://github.com/prze-kiesz/modu-core/discussions
- **Issues**: https://github.com/prze-kiesz/modu-core/issues
- **Documentation**: See README.md and module-specific docs

## License

By contributing, you agree that your contributions will be licensed under the BSD 2-Clause License.

---

Thank you for contributing to modu-core! 🚀
