# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

"""
conftest.py — shared pytest fixtures and shared BDD step definitions.

Environment variables
---------------------
MODU_CORE_BINARY
    Path to the modu-core executable to test.
    Default: <workspace_root>/build-test/main/modu-core
"""

import logging
import os
import signal
import subprocess
import textwrap
from pathlib import Path

import pytest
from pytest_bdd import given, when, then, parsers

_log = logging.getLogger(__name__)

from helpers.log_watcher import LogWatcher

# ---------------------------------------------------------------------------
# Workspace root
# ---------------------------------------------------------------------------
_WORKSPACE_ROOT = Path(__file__).resolve().parents[2]
_DEFAULT_BINARY = _WORKSPACE_ROOT / "build-test" / "main" / "modu-core"


# ---------------------------------------------------------------------------
# Infrastructure fixtures
# ---------------------------------------------------------------------------

@pytest.fixture(scope="session")
def binary_path() -> Path:
    """
    Absolute path to the modu-core binary under test.
    Override with MODU_CORE_BINARY env variable.
    """
    env_val = os.environ.get("MODU_CORE_BINARY")
    path = Path(env_val) if env_val else _DEFAULT_BINARY
    if not path.exists():
        pytest.fail(
            f"modu-core binary not found: {path}\n"
            "Build the project first or set MODU_CORE_BINARY."
        )
    return path


@pytest.fixture
def toml_config_dir(tmp_path: Path) -> Path:
    """
    Temporary XDG config directory with a minimal modu-core config.toml.
    Yields the xdg_home path; config file is at xdg_home/modu-core/config.toml.
    """
    xdg_home = tmp_path / "xdg_config"
    config_dir = xdg_home / "modu-core"
    config_dir.mkdir(parents=True)
    (config_dir / "config.toml").write_text(
        textwrap.dedent("""\
            # Minimal acceptance-test configuration
            [app]
            log_level = "info"
        """)
    )
    return xdg_home


class AppProcess:
    """Wrapper around a running modu-core subprocess."""

    def __init__(self, proc: subprocess.Popen, logs: LogWatcher) -> None:
        self.proc = proc
        self.logs = logs

    def send_signal(self, sig: signal.Signals) -> None:
        try:
            self.proc.send_signal(sig)
        except ProcessLookupError:
            pass

    def sighup(self) -> None:
        self.send_signal(signal.SIGHUP)

    def sigterm(self) -> None:
        self.send_signal(signal.SIGTERM)

    def sigint(self) -> None:
        self.send_signal(signal.SIGINT)

    def wait(self, timeout: float = 5.0) -> int:
        return self.proc.wait(timeout=timeout)

    @property
    def pid(self) -> int:
        return self.proc.pid

    @property
    def returncode(self) -> int | None:
        return self.proc.returncode


@pytest.fixture
def launch_app(binary_path: Path, toml_config_dir: Path):
    """
    Factory fixture — launches modu-core, returns AppProcess.

    Usage:
        app = launch_app()
        app = launch_app(extra_args=["--set", "key=val"])
        app = launch_app(xdg_home=custom_path)
    """
    processes: list[AppProcess] = []

    def _factory(
        extra_args: list[str] | None = None,
        extra_env: dict[str, str] | None = None,
        xdg_home: Path | None = None,
    ) -> AppProcess:
        env = os.environ.copy()
        env["XDG_CONFIG_HOME"] = str(xdg_home or toml_config_dir)
        if extra_env:
            env.update(extra_env)
        proc = subprocess.Popen(
            [str(binary_path)] + (extra_args or []),
            stderr=subprocess.PIPE,
            stdout=subprocess.DEVNULL,
            env=env,
        )
        logs = LogWatcher(proc.stderr).start()
        app = AppProcess(proc, logs)
        processes.append(app)
        return app

    yield _factory

    for app in processes:
        if app.proc.poll() is None:
            app.sigterm()
            try:
                app.proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                app.proc.kill()
                app.proc.wait()
        app.logs.stop()


# ---------------------------------------------------------------------------
# Shared BDD step definitions
# Steps defined here are available to all subdirectories automatically.
# ---------------------------------------------------------------------------

@given("the application is running", target_fixture="running_app")
def app_is_running(launch_app):
    """Start the application and wait until it is fully ready."""
    app = launch_app()
    matched = app.logs.wait_for("Waiting for application termination", timeout=10)
    _log.info("app ready  | %s", matched)
    return app


@when("SIGTERM is sent")
def send_sigterm(running_app):
    running_app.sigterm()


@when("SIGINT is sent")
def send_sigint(running_app):
    running_app.sigint()


@when("two SIGINTs are sent in quick succession")
def send_double_sigint(running_app):
    import threading

    def send_second():
        # Block until signal handler logs first SIGINT acknowledgment,
        # then immediately fire the second one.
        running_app.logs.wait_for("First SIGINT received", timeout=5)
        running_app.sigint()

    t = threading.Thread(target=send_second, daemon=True)
    t.start()           # thread is now waiting on the log line
    running_app.sigint() # send first SIGINT
    t.join(timeout=10)


@when("SIGHUP is sent")
def send_sighup(running_app):
    running_app.sighup()


@then(parsers.parse('the log contains "{message}"'))
def log_contains(running_app, message):
    matched = running_app.logs.wait_for(message, timeout=30)
    _log.info("log match  | %s", matched)


@then(parsers.parse("the exit code is {code:d}"))
def check_exit_code(running_app, code):
    actual = running_app.wait(timeout=5)
    assert actual == code, f"Expected exit code {code}, got {actual}"


@then("the application is still running")
def app_is_still_running(running_app):
    assert running_app.proc.poll() is None, \
        "Application terminated unexpectedly"
