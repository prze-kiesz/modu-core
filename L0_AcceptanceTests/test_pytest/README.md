# modu-core Acceptance Tests (pytest)

Black-box acceptance tests that exercise the running `modu-core` binary through
its external interfaces:

- **Log output** (glog → stderr): verified with `LogWatcher`
- **POSIX signals**: `SIGHUP`, `SIGTERM`, `SIGINT` sent via `os.kill`
- **Configuration files**: TOML files injected through `XDG_CONFIG_HOME`

## Prerequisites

```bash
pip install -r requirements.txt   # pytest, pytest-timeout
```

## Running

```bash
# Build the binary first (from project root)
cmake -S . -B build-test -DBUILD_TESTS=ON
cmake --build build-test -j$(nproc)

# Run all acceptance tests
cd L0_AcceptanceTests/test_pytest
pytest

# Point to a custom binary
MODU_CORE_BINARY=/path/to/modu-core pytest

# Run a single test file
pytest test_startup_shutdown.py -v
```

## Structure

```
test_pytest/
├── conftest.py                 # Fixtures: binary_path, toml_config_dir, launch_app
├── helpers/
│   └── log_watcher.py          # Background log reader with wait_for()
├── test_startup_shutdown.py    # Startup, SIGTERM, SIGINT scenarios
├── test_config_reload.py       # SIGHUP config reload scenarios
├── pytest.ini                  # pytest configuration
└── requirements.txt
```

## How it works

### `launch_app` fixture

Creates an `AppProcess` wrapping a `subprocess.Popen` of the binary:

```python
def test_example(launch_app):
    app = launch_app()
    app.logs.wait_for("Waiting for application termination", timeout=10)
    app.sigterm()
    assert app.wait() == 0
```

### Config injection

Every test gets a temporary `XDG_CONFIG_HOME` directory (via `toml_config_dir`
fixture). The binary picks up `{XDG_CONFIG_HOME}/modu-core/config.toml`
automatically — no changes to the binary are needed.

### Sending signals

```python
app.sighup()   # config reload
app.sigterm()  # graceful shutdown
app.sigint()   # graceful shutdown (2nd = force, exit 130)
```

### Waiting for log lines

```python
app.logs.wait_for("some log message", timeout=5)      # substring match
app.logs.wait_for(r"error.*code \d+", regexp=True)    # regex match
```

Raises `TimeoutError` with the full captured log if the pattern is not found.
