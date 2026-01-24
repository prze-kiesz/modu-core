# comm_config-toml

TOML-based configuration management module with XDG Base Directory support and runtime overrides.

## Features

- ✅ **XDG Base Directory compliance** - Standard Linux config hierarchy
- ✅ **Hierarchical configuration merge** - System → User → CLI overrides
- ✅ **Type-safe serialization** - ADL-based (Argument-Dependent Lookup)
- ✅ **Runtime reloading** - SIGHUP support for live config updates
- ✅ **CLI overrides** - Command-line arguments with highest priority
- ✅ **Automatic type inference** - Smart conversion from strings
- ✅ **Singleton pattern** - Thread-safe initialization

## Architecture

### Configuration Hierarchy

The module implements a 3-level priority system following Unix best practices:

```
Priority (lowest → highest):
1. System config:     /etc/<app>/config.toml
2. User config:       ~/.config/<app>/config.toml (or $XDG_CONFIG_HOME)
3. CLI overrides:     --set key=value
```

Later sources override earlier ones, enabling flexible configuration management.

### XDG Base Directory Specification

Complies with [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html):

- Respects `$XDG_CONFIG_HOME` environment variable
- Falls back to `~/.config` if not set
- System-wide configs in `/etc/<app>/`

## Quick Start

### Basic Usage

```cpp
#include "comm_config_toml.h"

// Initialize with XDG hierarchy
auto& config = comm::Config::Instance();
auto err = config.Initialize("modu-core");
if (err) {
    LOG(ERROR) << "Config init failed: " << err.message();
}

// Get configuration section
auto infr_config = config.Get<infr::InfrMainConfig>("infr_main");
```

### With CLI Overrides

```cpp
// Application startup (in main)
auto err = comm::Main::init(argc, argv);

// CLI usage:
./modu-core --set infr_main.port=9000 --set infr_main.device_name=test
```

### Configuration Reload

```cpp
// Register reload listener (typically in module init)
comm::Terminate::Instance().RegisterConfigReloadListener([]() {
    auto err = comm::Config::Instance().Reload();
    if (!err) {
        LOG(INFO) << "Configuration reloaded";
        // Apply new configuration to module
    }
});

// Runtime reload via signal:
kill -SIGHUP <pid>
```

## API Reference

### Core Methods

#### Initialize
```cpp
std::error_code Initialize(const std::string& app_name);
```
Initializes configuration system with XDG hierarchy.
- Loads `/etc/<app>/config.toml`
- Loads `~/.config/<app>/config.toml` and merges
- Creates empty config if no files exist

**Parameters:**
- `app_name` - Application name for config directory

**Returns:** Error code (empty on success)

#### Get
```cpp
template <typename T>
T Get(const std::string& path) const;
```
Retrieves typed configuration section using ADL serialization.

**Parameters:**
- `path` - Section name in TOML (e.g., "infr_main")

**Returns:** Deserialized configuration struct

**Note:** Requires `from_toml(const toml::value&, T&)` function in same namespace as `T`

#### SetOverride
```cpp
void SetOverride(const std::string& path, const std::string& value);
```
Overrides specific configuration value (highest priority).

**Parameters:**
- `path` - Dot-separated key path (e.g., "infr_main.port")
- `value` - String value (auto-converted to appropriate type)

**Type Inference:**
- `"true"` / `"false"` → bool
- `"123"` → int64_t
- `"3.14"` → double
- `"text"` → string

#### Reload
```cpp
std::error_code Reload();
```
Reloads configuration from all sources (XDG hierarchy + overrides).
Preserves CLI overrides across reload.

**Returns:** Error code (empty on success)

### Helper Methods

#### IsInitialized
```cpp
bool IsInitialized() const;
```
Checks if configuration system is initialized.

#### GetData
```cpp
const toml::value& GetData() const;
```
Returns raw TOML data (for advanced use cases).

## Configuration Files

### Directory Structure

```
/etc/modu-core/
  └── config.toml              # System defaults

~/.config/modu-core/
  └── config.toml              # User overrides
```

### Example Configuration

**System config** (`/etc/modu-core/config.toml`):
```toml
# System-wide defaults
[infr_main]
device_name = "default_device"
port = 8080
enable_logging = true
timeout_seconds = 30
```

**User config** (`~/.config/modu-core/config.toml`):
```toml
# User-specific overrides
[infr_main]
device_name = "my_device"    # Overrides system default
port = 9000                   # Overrides system default
# enable_logging inherited from system config
```

**Runtime override**:
```bash
./modu-core --set infr_main.port=7777
# Final value: port = 7777 (CLI has highest priority)
```

## Custom Type Serialization

To use custom structs with the configuration system, implement ADL serialization functions:

### Defining Serializable Struct

```cpp
// infr_config.h
namespace infr {

struct InfrMainConfig {
    std::string device_name = "default_device";
    int port = 8080;
    bool enable_logging = true;
    int timeout_seconds = 30;
};

// ADL serialization functions (in same namespace)
void to_toml(toml::value& dest, const InfrMainConfig& value);
void from_toml(const toml::value& src, InfrMainConfig& value);

}  // namespace infr
```

### Implementing Serialization

```cpp
// infr_config.cpp
namespace infr {

void to_toml(toml::value& dest, const InfrMainConfig& value) {
    dest["device_name"] = value.device_name;
    dest["port"] = value.port;
    dest["enable_logging"] = value.enable_logging;
    dest["timeout_seconds"] = value.timeout_seconds;
}

void from_toml(const toml::value& src, InfrMainConfig& value) {
    static const InfrMainConfig defaults;  // Default values
    
    value.device_name = toml::find_or(src, "device_name", defaults.device_name);
    value.port = toml::find_or(src, "port", defaults.port);
    value.enable_logging = toml::find_or(src, "enable_logging", defaults.enable_logging);
    value.timeout_seconds = toml::find_or(src, "timeout_seconds", defaults.timeout_seconds);
}

}  // namespace infr
```

### Usage

```cpp
// Load config section
auto config = comm::Config::Instance().Get<infr::InfrMainConfig>("infr_main");

LOG(INFO) << "Device: " << config.device_name;
LOG(INFO) << "Port: " << config.port;
```

## CLI Arguments

### --set Override

Override any configuration value at runtime:

```bash
# Single override
./modu-core --set infr_main.port=9000

# Multiple overrides
./modu-core --set infr_main.port=8080 \
            --set infr_main.device_name=test_device \
            --set infr_main.enable_logging=false

# Nested keys
./modu-core --set server.database.host=localhost \
            --set server.database.port=5432

# Different types (auto-detected)
./modu-core --set debug=true \           # bool
            --set port=3000 \             # int
            --set timeout=2.5 \           # float
            --set hostname=myserver       # string
```

### Format

```
--set <key>=<value>
```

**Key format:** Dot-separated path matching TOML structure
**Value format:** String (automatically converted to appropriate type)

## Integration with comm_main

The module integrates seamlessly with the Common layer initialization:

```cpp
// In main.cpp
int main(int argc, const char* argv[]) {
    // Initialize Common layer (includes config)
    auto err = comm::Main::init(argc, argv);
    if (err) {
        return 1;
    }
    
    // Config is now available globally
    auto config = comm::Config::Instance();
    auto my_settings = config.Get<MyConfig>("my_section");
    
    // ... application logic ...
    
    return 0;
}
```

**Automatic features:**
- Application name extracted from `argv[0]`
- XDG hierarchy loaded automatically
- CLI overrides parsed and applied
- SIGHUP reload listener registered

## Error Handling

### Error Codes

```cpp
enum class ConfigError {
    Success = 0,
    FileNotFound,       // Config file doesn't exist
    ParseError,         // TOML syntax error
    ValidationError,    // Config validation failed
    NotInitialized      // Config not initialized before use
};
```

### Checking Errors

```cpp
auto err = config.Initialize("myapp");
if (err) {
    if (err == comm::ConfigError::FileNotFound) {
        LOG(WARNING) << "No config file, using defaults";
    } else if (err == comm::ConfigError::ParseError) {
        LOG(ERROR) << "Config syntax error: " << err.message();
        return 1;
    }
}
```

### Error Code Integration

The module uses `std::error_code` for seamless integration with standard error handling:

```cpp
std::error_code init_app() {
    auto err = comm::Config::Instance().Initialize("myapp");
    if (err) return err;  // Propagate error
    
    // ... more initialization ...
    return {};  // Success
}
```

## Thread Safety

- ✅ **Singleton initialization** - Thread-safe (C++11 magic statics)
- ✅ **Read operations** - Safe for concurrent reads after initialization
- ⚠️ **Write operations** - `SetOverride()` and `Reload()` require external synchronization
- ⚠️ **Reload during SIGHUP** - Handled in separate thread, safe by design

**Best practice:** Perform all configuration modifications during initialization or in dedicated reload handler.

## Testing

### Unit Tests

Located in `unit_test/comm_config_toml_test.cpp`:
- XDG hierarchy loading
- Configuration merging
- Type inference
- Override mechanism
- Reload functionality

Run tests:
```bash
cd build
ctest --output-on-failure -R comm_config
```

### Manual Testing

Create test configs:
```bash
# System config
sudo mkdir -p /etc/modu-core
sudo tee /etc/modu-core/config.toml << EOF
[infr_main]
port = 8080
EOF

# User config
mkdir -p ~/.config/modu-core
cat > ~/.config/modu-core/config.toml << EOF
[infr_main]
port = 9000
device_name = "my_device"
EOF

# Run with override
./modu-core --set infr_main.port=7777
```

## Dependencies

- **toml++** v3.4.0+ - Header-only TOML v1.0.0 parser
- **glog** - Google logging library
- **C++20** - Required for concepts and ranges

## CMake Integration

```cmake
# In your module's CMakeLists.txt
find_package(modu-core-comm_config-toml REQUIRED)

target_link_libraries(your_target
    PRIVATE
        modu-core::comm_config-toml
)
```

## Best Practices

### 1. Use Default Values in Structs
```cpp
struct Config {
    int port = 8080;              // ✅ Good - has default
    std::string host = "localhost"; // ✅ Good - has default
    // int timeout;                 // ❌ Bad - no default
};
```

### 2. Validate Configuration
```cpp
void from_toml(const toml::value& src, Config& value) {
    // Load values
    value.port = toml::find_or(src, "port", 8080);
    
    // Validate
    if (value.port < 1 || value.port > 65535) {
        throw std::runtime_error("Invalid port number");
    }
}
```

### 3. Document Your Config Schema
```toml
# config.toml
[server]
# Port number for HTTP server (1-65535)
port = 8080

# Maximum number of concurrent connections
max_connections = 100
```

### 4. Handle Missing Sections Gracefully
```cpp
auto config = comm::Config::Instance().Get<MyConfig>("optional_section");
// from_toml should handle empty TOML value and return defaults
```

## License

BSD-2-Clause (see LICENSE file in repository root)

## See Also

- [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html)
- [TOML v1.0.0 Specification](https://toml.io/en/v1.0.0)
- [toml++ Library](https://github.com/marzer/tomlplusplus)
- [Module Template Guide](../../../docs/MODULE_TEMPLATE.md)
