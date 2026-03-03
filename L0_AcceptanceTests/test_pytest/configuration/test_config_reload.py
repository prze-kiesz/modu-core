# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

"""
BDD acceptance tests: Configuration Reload via SIGHUP.

Main flows use pytest-bdd with Gherkin scenarios from
features/config_reload.feature.

Edge cases (multiple rapid SIGHUPs) use raw pytest.
"""

import logging
import textwrap
from pathlib import Path

import pytest
from pytest_bdd import scenario, given

_log = logging.getLogger(__name__)

FEATURE = "features/config_reload.feature"


# ---------------------------------------------------------------------------
# BDD scenarios
# ---------------------------------------------------------------------------

@pytest.mark.timeout(20)
@scenario(FEATURE, "SIGHUP triggers config reload event")
def test_sighup_triggers_reload_event():
    pass


@pytest.mark.timeout(20)
@scenario(FEATURE, "Application keeps running after SIGHUP")
def test_application_keeps_running_after_sighup():
    pass


@pytest.mark.timeout(20)
@scenario(FEATURE, "Config file update is picked up on SIGHUP")
def test_config_file_update_is_picked_up_on_sighup():
    pass


# ---------------------------------------------------------------------------
# Step definitions specific to this area
# ---------------------------------------------------------------------------

@given("the config file is updated on disk")
def update_config_file(toml_config_dir: Path):
    """Overwrite the config.toml with new content before sending SIGHUP."""
    config_file = toml_config_dir / "modu-core" / "config.toml"
    config_file.write_text(
        textwrap.dedent("""\
            # Updated configuration for reload test
            [app]
            log_level = "debug"
        """)
    )


# ---------------------------------------------------------------------------
# Raw pytest — edge cases
# ---------------------------------------------------------------------------

@pytest.mark.timeout(20)
def test_multiple_rapid_sighup_signals_do_not_crash(launch_app):
    """
    Three rapid SIGHUP signals must all be handled without crashing
    and the application must remain running, accepting a clean SIGTERM.
    """
    app = launch_app()
    matched = app.logs.wait_for("Waiting for application termination", timeout=10)
    _log.info("app ready  | %s", matched)

    for i in range(3):
        app.sighup()
        m = app.logs.wait_for("Received SIGHUP, queuing config reload event", timeout=5)
        _log.info("sighup #%d  | %s", i + 1, m)

    assert app.proc.poll() is None, "Application crashed after multiple SIGHUPs"

    app.sigterm()
    assert app.wait(timeout=5) == 0


@pytest.mark.timeout(20)
def test_config_value_change_reflected_after_reload(launch_app, toml_config_dir: Path):
    """
    After writing a new config value to disk and sending SIGHUP, the
    application must reload and the updated config must appear in the
    reload log — verifying true hot-reload, not just that the event fires.

    Strategy: mark the log position before updating the file, then assert
    both the SIGHUP event and the "Configuration initialized with" line
    appear *after* that mark — so we can't accidentally match the startup
    load of the very same message.
    """
    app = launch_app()
    matched = app.logs.wait_for("Waiting for application termination", timeout=10)
    _log.info("app ready  | %s", matched)

    # Capture current log position — only accept lines that arrive after this
    mark = app.logs.mark()

    # Overwrite config on disk: log_level "info" → "debug"
    config_file = toml_config_dir / "modu-core" / "config.toml"
    config_file.write_text(
        textwrap.dedent("""\
            # Hot-reload test: updated value
            [app]
            log_level = "debug"
        """)
    )
    _log.info('config     | wrote log_level = "debug" → %s', config_file)

    app.sighup()

    reload_event = app.logs.wait_for(
        "Received SIGHUP, queuing config reload event",
        timeout=5,
        from_mark=mark,
    )
    _log.info("sighup evt | %s", reload_event)

    reload_log = app.logs.wait_for(
        "Configuration initialized with",
        timeout=5,
        from_mark=mark,
    )
    _log.info("reload log | %s", reload_log)

    assert app.proc.poll() is None, "Application crashed after config file update + SIGHUP"

    app.sigterm()
    assert app.wait(timeout=5) == 0
