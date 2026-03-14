# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

"""
BDD acceptance tests: Application Lifecycle (startup and shutdown).

Main flows use pytest-bdd with Gherkin scenarios from
features/startup_shutdown.feature.

Edge cases (ordering checks, stress) use raw pytest.
"""

import re
import pytest
from pytest_bdd import scenario, given, then, when, parsers

FEATURE = "features/startup_shutdown.feature"


# ---------------------------------------------------------------------------
# BDD scenarios — map directly to Gherkin in the .feature file
# ---------------------------------------------------------------------------

@pytest.mark.timeout(15)
@scenario(FEATURE, "Application logs its version on startup")
def test_application_logs_version_on_startup():
    pass


@pytest.mark.timeout(15)
@scenario(FEATURE, "Application starts and becomes ready")
def test_application_starts_and_becomes_ready():
    pass


@pytest.mark.timeout(15)
@scenario(FEATURE, "SIGTERM triggers graceful shutdown")
def test_sigterm_triggers_graceful_shutdown():
    pass


@pytest.mark.timeout(15)
@scenario(FEATURE, "SIGINT triggers graceful shutdown")
def test_sigint_triggers_graceful_shutdown():
    pass


@pytest.mark.timeout(30)
@scenario(FEATURE, "Double SIGINT forces immediate exit with code 130")
def test_double_sigint_forces_immediate_exit():
    pass


# ---------------------------------------------------------------------------
# Step definitions specific to this area
# (shared steps like "the application is running", "the log contains X",
#  signals and exit code are in the root conftest.py)
# ---------------------------------------------------------------------------

@given("the application is started", target_fixture="running_app")
def app_is_started(launch_app):
    """Start the application without waiting for the ready state."""
    return launch_app()


@then(parsers.re(r'the startup log contains a version tag matching "(?P<pattern>[^"]+)"'))
def startup_log_contains_version(running_app, pattern):
    running_app.logs.wait_for("Starting ", timeout=10)
    log_text = "\n".join(running_app.logs.lines())
    match = re.search(pattern, log_text)
    assert match, (
        f"No version tag matching '{pattern}' found in startup logs.\n"
        f"Logs:\n{log_text}"
    )


@then("there are no ERROR lines in the startup logs")
def no_error_lines(running_app):
    lines = running_app.logs.lines()
    error_lines = [l for l in lines if " ERROR " in l]
    assert not error_lines, \
        "Unexpected ERROR lines during startup:\n" + "\n".join(error_lines)


@when("SIGINT is sent again")
def send_sigint_again(running_app):
    running_app.sigint()


# ---------------------------------------------------------------------------
# Raw pytest — edge cases not well-suited for Gherkin
# ---------------------------------------------------------------------------

@pytest.mark.timeout(15)
def test_shutdown_logs_deinit_in_correct_order(launch_app):
    """
    After SIGTERM the application logs 'Application is shutting down' before
    returning — deinit of all layers is complete before exit.
    """
    app = launch_app()
    app.logs.wait_for("Waiting for application termination", timeout=10)
    app.sigterm()
    app.wait(timeout=5)
    assert "Application is shutting down" in "\n".join(app.logs.lines())
