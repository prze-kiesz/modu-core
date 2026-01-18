# modu-core

[![Build and Test](https://github.com/prze-kiesz/modu-core/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/prze-kiesz/modu-core/actions/workflows/build-and-test.yml)
[![Static Analysis](https://github.com/prze-kiesz/modu-core/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/prze-kiesz/modu-core/actions/workflows/static-analysis.yml)
[![Docker Build](https://github.com/prze-kiesz/modu-core/actions/workflows/docker-build.yml/badge.svg)](https://github.com/prze-kiesz/modu-core/actions/workflows/docker-build.yml)
[![License](https://img.shields.io/badge/License-BSD_2--Clause-blue.svg)](https://opensource.org/licenses/BSD-2-Clause)

Universal C++ Application Framework with Layered Modular Architecture

## Project Overview

**modu-core** is a modern C++20 framework designed for building robust, maintainable, and scalable applications through a strictly layered modular architecture. The project demonstrates best practices in software engineering, including clean architecture, dependency management, comprehensive testing, and professional CI/CD pipelines.

## Key Features

### 🏗️ Layered Architecture

The framework implements a five-layer architecture where each layer has distinct responsibilities and clear dependencies:

- **L1_Presentation** - User interface and presentation logic
- **L2_Services** - Business logic and application services
- **L3_Storage** - Data persistence and storage abstractions
- **L4_Infrastructure** - System-level infrastructure (networking, hardware interfaces)
- **L5_Common** - Cross-cutting concerns (logging, error handling, lifecycle management)

**Dependency Rule**: Higher layers (L1) can depend on lower layers (L5), but never the reverse. This ensures clean separation of concerns and prevents circular dependencies.

> **Note**: This five-layer architecture is just an example demonstrating the framework's capabilities. End users can freely adapt the architecture to their specific needs - add, remove, or reorganize layers, rename them, or create entirely different structures. The framework is flexible and supports any layered architecture design.

### 🧩 Independent Module Development

Each module within a layer is:
- **Self-contained**: Has its own `CMakeLists.txt` for build configuration
- **Independently testable**: Includes dedicated `unit_test/` directory with comprehensive tests
- **Discoverable**: Provides `*-config.cmake` for integration via CMake's `find_package()`
- **Well-documented**: Clear interfaces defined in `interface/` headers, optional `README.md` for module overview, and `doc/` subdirectory for detailed documentation

This architecture enables:
- Parallel development by multiple teams
- Module-level testing and validation
- Easy addition of new modules without affecting existing code
- Selective linking - only include what you need

### ⚙️ Modern C++ Standards

- **C++20**: Leverages latest language features (concepts, coroutines, ranges)
- **std::error_code**: Standardized error handling across all modules
- **RAII & Smart Pointers**: Memory safety without garbage collection
- **Type Safety**: Strong typing with compile-time checks

### 🧪 Comprehensive Testing

- **Unit Tests**: Each module has isolated unit tests using Google Test framework
- **CTest Integration**: Automatic test discovery and execution
- **Continuous Testing**: All tests run on every commit via CI/CD
- **Code Coverage**: Track test coverage metrics (planned)

### 📦 Professional Build System

- **CMake 3.22+**: Modern CMake with target-based dependency management
- **OBJECT Libraries**: Efficient compilation and linking
- **Generator Expressions**: Flexible build/install interface separation
- **Cross-platform**: Linux (Ubuntu 24.04), with potential for Windows/macOS

### 🔄 Error Handling

- **Custom Error Categories**: Each module defines domain-specific error codes
- **Type-safe**: Compile-time error code validation
- **Informative**: Human-readable error messages
- **Composable**: Errors propagate cleanly through layer boundaries

### 🚀 System Integration

- **systemd Support**: Native integration with Linux system daemon manager
- **Signal Handling**: Graceful shutdown on SIGTERM, SIGINT, SIGQUIT
- **Logging**: Structured logging with Google glog
- **Lifecycle Management**: Proper initialization and deinitialization order

## Project Structure

```
modu-core/
├── CMakeLists.txt              # Root project configuration
├── main/                       # Application entry point
│   ├── CMakeLists.txt
│   └── main.cpp
├── L1_Presentation/            # Layer 1: UI and presentation
├── L2_Services/                # Layer 2: Business logic
├── L3_Storage/                 # Layer 3: Data persistence
├── L4_Infrastructure/          # Layer 4: System infrastructure
│   └── infr_main/              # Infrastructure initialization module
│       ├── CMakeLists.txt
│       ├── infr_main-config.cmake
│       ├── interface/
│       │   └── infr_main.h
│       └── src/
│           └── infr_main.cpp
└── L5_Common/                  # Layer 5: Common utilities
    ├── comm_main/              # Common layer initialization
    │   ├── CMakeLists.txt
    │   ├── comm_main-config.cmake
    │   ├── interface/
    │   │   └── comm_main.h
    │   └── src/
    │       └── comm_main.cpp
    └── comm_terminate/         # Signal handling and graceful shutdown
        ├── CMakeLists.txt
        ├── comm_terminate-config.cmake
        ├── interface/
        │   └── comm_terminate.h
        ├── src/
        │   └── comm_terminate.cpp
        └── unit_test/
            ├── CMakeLists.txt
            └── comm_terminate_test.cpp
```

## Building the Project

### Development Container

The project includes a pre-configured development container with all dependencies installed. 

> **📖 For detailed documentation** including version management, tagging strategy, and multi-architecture support, see [.devcontainer/README.md](.devcontainer/README.md)

**Option 1: Prebuilt Docker Image (Recommended)**
```bash
# Pull the latest development container image
docker pull ghcr.io/prze-kiesz/modu-core:latest

# Or use it directly in VS Code by uncommenting the "image" line in .devcontainer/devcontainer.json
```

**Option 2: Build Locally**
```bash
# Build the development container from Dockerfile
cd .devcontainer
docker build -t modu-core-dev .
```

The container includes:
- Ubuntu 24.04 LTS
- CMake, Make, GCC, Clang-19
- Google Test (gtest + gmock) 1.16.0
- Google glog 0.6.0
- systemd development libraries
- Cross-compilation tools (ARM64)
- Development tools (ccache, clang-tidy, clang-format, clangd)

### Prerequisites

```bash
# Ubuntu 24.04
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libgtest-dev \
    libgmock-dev \
    libgoogle-glog-dev \
    libsystemd-dev \
    pkg-config
```

### Build Instructions

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run application
./main/modu-core

# Run tests
ctest --output-on-failure
```

### Build Options

- `BUILD_TESTING=ON` - Enable unit tests (default: ON)
- `CMAKE_BUILD_TYPE=Release|Debug` - Build configuration

## Running Tests

```bash
# Run all tests
cd build
ctest --output-on-failure

# Run specific test
./main/comm_terminate/unit_test/modu-core-comm_terminate_unittest

# Verbose test output
ctest -V
```

## CI/CD Integration

The project uses **GitHub Actions** for continuous integration and deployment:

### Automated Workflows

- ✅ **Build and Test**: Compile and test on every push and pull request
  - Builds: Debug and Release configurations with GCC
  - Runs all unit tests with CTest
  - Test results uploaded as artifacts
  - Uses prebuilt Docker container for fast builds
- ✅ **Static Analysis**: Automated code quality checks
  - **clang-tidy**: Modern C++ linting and best practices
  - **cppcheck**: Static analysis for bugs and undefined behavior
  - **clang-format**: Code formatting verification
  - Reports uploaded as artifacts for review
- ✅ **Docker Image Build**: Automatically build and publish development container to GitHub Container Registry
  - Triggered on changes to `.devcontainer/Dockerfile`
  - Tagged with `latest`, branch name, PR number, and git SHA
  - Published to `ghcr.io/prze-kiesz/modu-core`
  - Semantic versioning on releases
- 🔄 **Code Quality**: Static analysis and linting (planned)
- 🔄 **Coverage Reports**: Track test coverage trends (planned)
- 📦 **Package Creation**: Build `.deb` and `.rpm` packages (planned)
- 🚀 **Release Automation**: Automatic versioning and release creation (planned)

### Using the Prebuilt Development Container

```bash
# Pull the latest image
docker pull ghcr.io/prze-kiesz/modu-core:latest

# Run with VS Code Remote Containers extension
# Or update .devcontainer/devcontainer.json to use prebuilt image
```

See `.github/workflows/` for workflow definitions.

## Versioning

The project follows [Semantic Versioning 2.0.0](https://semver.org/):

- **MAJOR**: Incompatible API changes
- **MINOR**: Backward-compatible functionality additions
- **PATCH**: Backward-compatible bug fixes

Current version: `1.0.0` (defined in root `CMakeLists.txt`)

Version information is embedded in:
- CMake project configuration
- Package metadata (deb/rpm)
- Application runtime information

## Installation

### From Source

```bash
cd build
sudo make install
```

Default installation paths:
- Binaries: `/usr/local/bin/`
- Libraries: `/usr/local/lib/`
- Headers: `/usr/local/include/`

### Package Installation (Planned)

```bash
# Debian/Ubuntu
sudo dpkg -i modu-core_1.0.0_amd64.deb
sudo apt-get install -f

# RedHat/CentOS/Fedora
sudo rpm -i modu-core-1.0.0.x86_64.rpm
sudo yum install modu-core
```

## Packaging

### CPack Integration (Planned)

The project uses CPack for creating distribution packages:

```bash
cd build
cpack -G DEB    # Create Debian package
cpack -G RPM    # Create RPM package
```

Package features:
- Automatic dependency detection
- systemd service unit installation
- Configuration file management
- Clean uninstallation

## Development Workflow

### Adding a New Module

1. **Create module structure**:
   ```bash
   mkdir -p L5_Common/my_module/{interface,src,unit_test}
   ```

2. **Create CMakeLists.txt**: Define module build configuration

3. **Create *-config.cmake**: Define integration interface

4. **Implement module**: Add headers in `interface/`, sources in `src/`

5. **Add unit tests**: Create tests in `unit_test/`

6. **Register module**: Add to appropriate layer in root CMakeLists.txt

### Testing a Module

```bash
# Build and test specific module
cd build
make modu-core-my_module_unittest
./path/to/modu-core-my_module_unittest
```

## Contributing

Contributions are welcome! All changes must be submitted through pull requests.

**Quick Start:**

1. **Fork the repository** and create a feature branch
2. **Make your changes** following the architecture guidelines
3. **Run tests and static analysis** locally
4. **Submit a pull request** using the provided template
5. **Address review feedback** and wait for approval

**Development Workflow:**

- All changes go through pull requests (no direct commits to `main`)
- PRs require passing CI/CD checks (builds, tests, static analysis)
- At least one approval required before merging
- Squash merging enforced for clean history

**Guidelines:**

1. **Follow the architecture**: Respect layer dependencies (higher → lower only)
2. **Write tests**: All new code must have unit tests with good coverage
3. **Document interfaces**: Clear comments in header files
4. **Use modern C++**: Leverage C++20 features appropriately
5. **Error handling**: Use std::error_code for recoverable errors
6. **Code style**: Follow existing conventions (clang-format enforced)

For detailed contribution guidelines, see [CONTRIBUTING.md](.github/CONTRIBUTING.md).

## License

This project is licensed under the **BSD 2-Clause License**.

See [LICENSE](LICENSE) file for details.

## Authors

- Przemek Kieszkowski - Initial work and architecture

## Roadmap

### Current Status (v1.0.0)
- ✅ Layered architecture framework
- ✅ CMake build system
- ✅ Basic modules (comm_main, comm_terminate, infr_main)
- ✅ Unit testing infrastructure
- ✅ systemd integration
- ✅ CI/CD with automated builds and testing
- ✅ Static code analysis (clang-tidy, cppcheck, clang-format)
- ✅ Docker development container

### Planned Features
- ✅ **Docker development container with automated builds**
- ✅ **CI/CD pipeline with build and test automation**
- 🔄 **Static code analysis (clang-tidy, cppcheck, clang-format)**
- 📋 Code coverage reporting
- 📋 Automated package generation (deb/rpm)
- 📋 Documentation generation (Doxygen)
- ✅ **Docker container support**
- 📋 Performance benchmarking
- 📋 Example applications
- 📋 Storage layer implementation (L3)
- 📋 Service layer implementation (L2)
- 📋 Presentation layer implementation (L1)

## Support

For issues, questions, or contributions:
- **Issues**: https://github.com/prze-kiesz/modu-core/issues
- **Discussions**: https://github.com/prze-kiesz/modu-core/discussions
- **Contributing Guide**: [CONTRIBUTING.md](.github/CONTRIBUTING.md)
- **Pull Request Template**: Automatically loaded when creating PRs

---

**Main concepts based on software experience - developed with AI assistance**
