# comm_config Module

Application configuration management module using TOML format.

## Features

- ✅ **TOML Format** - Modern, human-readable configuration format
- ✅ **XDG Base Directory** - Linux/Unix standard compliance
- ✅ **Configuration Hierarchy** - System → User → Runtime
- ✅ **Nested keys** - Dot notation support for nested keys
- ✅ **Type-safe API** - Strongly typed getters and setters
- ✅ **Merge configurations** - Combine multiple configuration files

## Usage

### Basic Operations

```cpp
#include "comm_config.h"

// Create and load configuration
comm::Config config;
auto err = config.load_from_file("/etc/myapp/config.toml");

// Read values
auto host = config.get_string("server.host");
auto port = config.get_int("server.port");
auto debug = config.get_bool("server.debug");
auto timeout = config.get_double("database.timeout");

// Set values
config.set_string("app.name", "myapp");
config.set_int("app.threads", 4);
config.set_bool("app.debug", true);

// Save to file
config.save_to_file("/home/user/.config/myapp/config.toml");
```

### XDG Hierarchy Loading

```cpp
// Automatic loading following XDG standard:
// 1. /etc/myapp/config.toml (system-wide)
// 2. $XDG_CONFIG_HOME/myapp/config.toml (user)
//    or ~/.config/myapp/config.toml

comm::Config config;
auto err = config.load_xdg_hierarchy("myapp");

// Override values programmatically later
config.set_string("runtime.override", "value");
```

### Merge Configurations

```cpp
comm::Config config;

// Default values in code
config.set_string("server.host", "localhost");
config.set_int("server.port", 8080);

// Merge system configuration
config.merge_from_file("/etc/myapp/config.toml");  // port → 9090

// Merge user configuration
config.merge_from_file("~/.config/myapp/config.toml");  // debug → true
```

## TOML File Example

```toml
# config.toml
[server]
host = "localhost"
port = 8080
debug = false

[database]
connection_string = "postgresql://localhost:5432/db"
max_connections = 100
timeout = 30.5

[features]
enabled = ["auth", "logging", "metrics"]
```

## API Reference

### Constructors
- `Config()` - Empty configuration
- `Config(const std::filesystem::path&)` - Load from file

### File Operations
- `load_from_file()` - Load configuration
- `save_to_file()` - Save configuration
- `merge_from_file()` - Merge (override values)
- `load_xdg_hierarchy()` - Load following XDG standard

### Getters (std::optional)
- `get_string(key)` - String
- `get_int(key)` - int64_t
- `get_double(key)` - double
- `get_bool(key)` - bool
- `get_string_array(key)` - std::vector<std::string>

### Setters
- `set_string(key, value)`
- `set_int(key, value)`
- `set_double(key, value)`
- `set_bool(key, value)`
- `set_string_array(key, value)`

### Utility
- `has_key(key)` - Check if key exists
- `remove_key(key)` - Remove key
- `clear()` - Clear all values
- `get_all_keys()` - List all keys

## Error Handling

```cpp
auto err = config.load_from_file("config.toml");
if (err) {
    LOG(ERROR) << "Failed to load config: " << err.message();
    // ConfigError::FileNotFound
    // ConfigError::ParseError
    // ConfigError::WriteError
}
```

## Dependencies

- **toml++** - Header-only TOML parser (v3.4.0)
- **glog** - Logging framework
- **C++20** - std::filesystem, std::optional

## Tests

```bash
# Build and run tests
cd build
make modu-core-comm_config_unittest
ctest -R ConfigTest --output-on-failure

# Standalone build (faster iteration)
cd L5_Common/comm_config/unit_test
mkdir build && cd build
cmake .. && make
./modu-core-comm_config_unittest
```

## Module Structure

```
L5_Common/comm_config/
├── interface/
│   └── comm_config.h          # Public API interface
├── src/
│   └── comm_config.cpp        # Implementation
├── unit_test/
│   ├── CMakeLists.txt         # Standalone build support
│   └── comm_config_test.cpp   # 13 unit tests
├── CMakeLists.txt             # Module configuration
└── comm_config-config.cmake   # Find package config
```

## Integration

```cmake
# In your module's CMakeLists.txt
find_package("comm_config" REQUIRED)

target_link_libraries(your_target PRIVATE
    modu-core-comm_config
)
```
