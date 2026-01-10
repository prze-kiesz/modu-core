# SIGHUP Configuration Reload Pattern

## Overview

The `comm_terminate` module now supports configuration reload via SIGHUP signal without application restart. The implementation uses a listener pattern with asynchronous event processing to ensure signal safety and modularity.

## Architecture

### Thread Model

The system uses three threads:

1. **Main Thread**: Application logic and waiting for termination
2. **Signal Handler Thread**: Dedicated thread using `sigwait()` to receive POSIX signals
3. **Event Processor Thread**: Processes queued events and invokes registered listeners

### Signal Flow

```
SIGHUP signal received
    ↓
Signal Handler Thread (sigwait)
    ↓
Push ConfigReload event to queue
    ↓
Notify event processor via condition variable
    ↓
Event Processor Thread wakes up
    ↓
Pop event from queue
    ↓
Invoke all registered listeners
    ↓
Continue waiting for signals (no termination)
```

### Key Design Decisions

1. **Signal Handler is Minimal**: Only queues events, doesn't call listeners directly
   - Ensures signal handler remains async-signal-safe
   - Avoids blocking signal delivery with long-running callbacks

2. **Separate Event Processor Thread**: Invokes listeners in known execution context
   - Listeners can perform arbitrary work without signal safety concerns
   - Exceptions in listeners are caught and logged, don't crash process

3. **Thread-Safe Event Queue**: Mutex + condition variable for synchronization
   - Signal handler can queue events without blocking
   - Event processor efficiently waits for new events

4. **Multiple Listeners Supported**: Other modules can register callbacks
   - Decoupled from comm_terminate implementation
   - Listeners execute in registration order

## API Usage

### Registering a Config Reload Listener

```cpp
#include "comm_terminate.h"

// Register listener during module initialization
comm::Terminate::Instance().RegisterConfigReloadListener([]() {
    LOG(INFO) << "Config changed, reloading module state";
    MyModule::instance().reload_settings();
});
```

### Example with Multiple Listeners

```cpp
// Module A registers its listener
Terminate::Instance().RegisterConfigReloadListener([]() {
    ModuleA::reload_config();
});

// Module B registers its listener
Terminate::Instance().RegisterConfigReloadListener([]() {
    ModuleB::update_settings();
});

// Both listeners will be called when SIGHUP is received
```

### Sending SIGHUP

```bash
# Find process ID
ps aux | grep modu-core

# Send SIGHUP signal
kill -SIGHUP <pid>

# Or use systemd
systemctl reload modu-core.service
```

## Systemd Integration

The terminate handler integrates with systemd notify protocol:

- `READY=1` - Sent when application starts successfully
- `RELOADING=1` - Sent when SIGHUP received, before processing
- `READY=1` - Sent again after config reload completes
- `STOPPING=1` - Sent when termination signal received

### Systemd Service Configuration

```ini
[Unit]
Description=Modu Core Application
After=network.target

[Service]
Type=notify
ExecStart=/usr/bin/modu-core
ExecReload=/bin/kill -SIGHUP $MAINPID
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

With this configuration:
- `systemctl reload modu-core` sends SIGHUP automatically
- systemd knows when reload is complete via READY=1 notification

## Signal Handling

### Supported Signals

| Signal | Behavior | Termination | Event |
|--------|----------|-------------|-------|
| SIGINT | Graceful shutdown | Yes | - |
| SIGTERM | Graceful shutdown | Yes | - |
| SIGQUIT | Graceful shutdown | Yes | - |
| SIGHUP | Config reload | No | ConfigReload |

### Signal Handler Thread Safety

The signal handler thread uses `sigwait()` which is signal-safe:
- Blocks signals in main thread via `sigprocmask()`
- Signals are only delivered to the dedicated handler thread
- Handler synchronously waits for signals without race conditions

## Thread Safety Guarantees

### Event Queue Protection

```cpp
{
    std::lock_guard<std::mutex> lock(m_event_mutex);
    m_event_queue.push(EventType::ConfigReload);
}
m_event_cv.notify_one();  // Outside lock to avoid thundering herd
```

### Listener Invocation

```cpp
// Copy listeners under lock
std::vector<std::function<void()>> listeners_copy;
{
    std::lock_guard<std::mutex> listeners_lock(m_listeners_mutex);
    listeners_copy = m_config_reload_listeners;
}

// Invoke without holding lock
for (const auto& listener : listeners_copy) {
    try {
        listener();
    } catch (...) {
        LOG(ERROR) << "Exception in listener";
    }
}
```

## Testing

### Unit Tests

See [comm_terminate_test.cpp](../L5_Common/comm_terminate/unit_test/comm_terminate_test.cpp):
- `RegisterConfigReloadListenerAddsCallback` - Verifies registration
- `MultipleListenersCanBeRegistered` - Tests multiple listeners

### Integration Test

The integration test is located in the module's integration_test directory.

**Quick Run (Recommended):**

```bash
cd L5_Common/comm_terminate/integration_test
./run_test.sh
```

The script will:
- Build the test if needed
- Run the test executable in background
- Send SIGHUP signals to trigger config reload
- Send SIGTERM to gracefully terminate
- Validate results and display log output

**Manual Run:**

Build and run:

```bash
cd build
make modu-core-comm_terminate_integration_test
cd main/comm_terminate/integration_test
./modu-core-comm_terminate_integration_test &
PID=$!

# Send SIGHUP to trigger config reload
kill -SIGHUP $PID

# Terminate gracefully
kill -SIGTERM $PID
```

Or run via CTest:
```bash
cd build
ctest -R comm_terminate_integration_test -V
```

Expected output:
```
I... SIGHUP Integration Test Started
I... Process PID: 12345
I... Registered 2 config reload listeners
I... Terminate handler started successfully
I... Application daemon has successfully started up.
I... Received SIGHUP, queuing config reload event
I... Processing ConfigReload event, invoking listeners
I... >>> Config reload listener invoked! Reload #1
I... >>> Config reload completed
I... Config reload event processed, invoked 2 listeners
I... Application daemon shutting down. Received signal 15 (Termination request)
I... Application terminated: Termination request
```

## Performance Characteristics

- **Signal Handler Latency**: < 1 µs (just queue push + notify)
- **Listener Invocation**: Asynchronous, doesn't block signal handler
- **Memory Overhead**: One event queue, one condition variable, vector of callbacks
- **Thread Count**: +1 event processor thread (in addition to signal handler thread)

## Error Handling

1. **Exception in Listener**:
   - Caught and logged by event processor
   - Other listeners still execute
   - Event processing continues

2. **Event Queue Full**: Not possible (std::queue dynamically grows)

3. **Thread Creation Failure**:
   - `Start()` returns error code
   - Application should not continue without terminate handler

## Best Practices

### Listener Implementation

✅ **DO**:
- Keep listeners fast and focused
- Use async operations for slow work
- Log errors clearly
- Handle exceptions internally
- Make listeners idempotent (may be called multiple times)

❌ **DON'T**:
- Block indefinitely in listeners
- Assume ordering between different modules' listeners
- Modify shared state without locking
- Throw exceptions out of listeners

### Example: Async Config Reload

```cpp
Terminate::Instance().RegisterConfigReloadListener([]() {
    LOG(INFO) << "Queuing async config reload";
    
    // Queue work on thread pool instead of blocking
    ThreadPool::instance().enqueue([]() {
        auto new_config = Config::load_from_file();
        Config::instance().update(new_config);
    });
});
```

## Future Enhancements

Potential improvements:
- Priority-based listener ordering
- Async listener support with futures
- Listener unregistration API
- More event types (SIGUSR1, SIGUSR2)
- Listener execution timeout
- Listener dependency graph for ordering

## References

- [POSIX Signal Handling](https://man7.org/linux/man-pages/man7/signal-safety.7.html)
- [systemd Notify Protocol](https://www.freedesktop.org/software/systemd/man/sd_notify.html)
- [C++ Condition Variables](https://en.cppreference.com/w/cpp/thread/condition_variable)
- [Observer Pattern](https://refactoring.guru/design-patterns/observer)
