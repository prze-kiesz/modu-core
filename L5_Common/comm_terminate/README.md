# comm_terminate

Graceful shutdown handler with signal management and systemd integration for C++20 applications.

## Features

- **Signal Handling**: SIGINT, SIGTERM, SIGQUIT (termination), SIGHUP (config reload)
- **Double Ctrl-C**: First initiates graceful shutdown, second forces immediate exit
- **Config Reload**: SIGHUP triggers registered listeners without restart
- **systemd Integration**: READY/STOPPING/RELOADING notifications
- **Thread-Safe**: Dedicated signal handler and event processor threads
- **Singleton Pattern**: One instance per application (Meyers' Singleton)

## Quick Start

```cpp
#include "comm_terminate.h"

int main() {
    // Optional: Register config reload listener
    comm::Terminate::Instance().RegisterConfigReloadListener([]() {
        LOG(INFO) << "Reloading configuration...";
        // Your reload logic here
    });

    // Start signal handling
    if (auto err = comm::Terminate::Instance().Start(); err) {
        LOG(ERROR) << "Failed to start: " << err.message();
        return 1;
    }

    // Your application logic runs here...

    // Wait for termination signal
    std::string reason = comm::Terminate::Instance().WaitForTermination();
    LOG(INFO) << "Shutting down: " << reason;

    return 0;
}
```

## API

### Core Methods

```cpp
// Get singleton instance
static Terminate& Instance();

// Start signal handling (must call before WaitForTermination)
std::error_code Start();

// Block until termination signal received
std::string WaitForTermination();

// Register callback for SIGHUP (config reload)
void RegisterConfigReloadListener(std::function<void()> callback);

// Programmatic termination
void TerminateApp(uint32_t milis_to_wait = 0);
```

## Signal Behavior

### SIGINT (Ctrl-C)
- **First press**: Graceful shutdown, cleanup continues
- **Second press**: Immediate termination via `std::_Exit(130)`

### SIGTERM / SIGQUIT
- Graceful shutdown (single signal)

### SIGHUP
- Triggers config reload listeners
- Does NOT terminate process
- Sends systemd RELOADING=1 / READY=1 notifications

## Architecture

```
┌─────────────┐     ┌──────────────────┐     ┌────────────────────┐
│ Main Thread │───▶│ Signal Handler   │────▶│ Event Processor    │
│             │     │ Thread           │     │ Thread             │
│ Application │     │ (sigwait)        │     │ (config reload)    │
│ Logic       │     │                  │     │                    │
└─────────────┘     └──────────────────┘     └────────────────────┘
       │                     │                         │
       │                     ├─ SIGINT #1 ─────────────┤
       │◀────semaphore──────┤   (continue waiting)    │
       │                     │                         │
       │ (graceful cleanup)  ├─ SIGINT #2              │
       │                     └─ std::_Exit(130)        │
       │                                               │
       │◀────── SIGHUP ──────────────────────────────▶│
       │                          (invoke listeners)   │
```

## Dependencies

- **C++20**: `std::binary_semaphore`, threading
- **glog**: Logging
- **systemd**: Notify protocol integration

## Testing

```bash
# Unit tests
cd build
ctest -R comm_terminate_unittest

# Integration tests
cd L5_Common/comm_terminate/integration_test
./run_test.sh                    # SIGHUP config reload
./run_test_double_sigint.sh      # Double Ctrl-C handling
```

## Documentation

- [docs/SIGHUP_CONFIG_RELOAD.md](../../docs/SIGHUP_CONFIG_RELOAD.md) - Configuration reload pattern
- [integration_test/README.md](integration_test/README.md) - Integration tests overview
- [integration_test/README_DOUBLE_SIGINT.md](integration_test/README_DOUBLE_SIGINT.md) - Double Ctrl-C test details
- [interface/comm_terminate.h](interface/comm_terminate.h) - Full API documentation (Doxygen)

## Example: Config Reload

```cpp
// Module A
Terminate::Instance().RegisterConfigReloadListener([]() {
    LOG(INFO) << "Module A reloading config";
    load_config_from_file();
});

// Module B
Terminate::Instance().RegisterConfigReloadListener([]() {
    LOG(INFO) << "Module B updating settings";
    update_runtime_settings();
});

// Both listeners will be called when: kill -SIGHUP <pid>
```

## Example: Programmatic Termination

```cpp
// Trigger shutdown from application logic
if (fatal_error_detected()) {
    LOG(ERROR) << "Fatal error, initiating shutdown";
    Terminate::Instance().TerminateApp(1000);  // 1 second delay
}
```

## systemd Integration

Service file example:

```ini
[Service]
Type=notify
ExecStart=/usr/bin/modu-core
ExecReload=/bin/kill -HUP $MAINPID
Restart=on-failure
```

The module automatically sends:
- `READY=1` when startup complete
- `RELOADING=1` when SIGHUP received
- `READY=1` after config reload complete
- `STOPPING=1` when shutdown initiated

## License

BSD-2-Clause
