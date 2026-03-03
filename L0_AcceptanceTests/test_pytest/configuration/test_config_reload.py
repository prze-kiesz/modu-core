# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

"""
BDD acceptance tests: Configuration Reload via SIGHUP.

Main flows use pytest-bdd with Gherkin scenarios from
features/config_reload.feature.

Edge cases (multiple rapid SIGHUPs) use raw pytest.
"""

import textwrap
from pathlib import Path

import pytest
from pytest_bdd import scenario, given

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
    app.logs.wait_for("Waiting for application termination", timeout=10)

    for _ in range(3):
        app.sighup()
        app.logs.wait_for("Received SIGHUP, queuing config reload event", timeout=5)

    assert app.proc.poll() is None, "Application crashed after multiple SIGHUPs"

    app.sigterm()
    assert app.wait(timeout=5) == 0
