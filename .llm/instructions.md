<!--
Copyright (c) 2026 Przemek Kieszkowski
SPDX-License-Identifier: BSD-2-Clause
-->

# AI/Agent Guidelines for modu-core

## Main Goals
- Maintain high code quality
- Document changes
- Test new functionality

## Coding Standards
- Use clear variable and function names
- Write comments for complex logic
- Follow naming conventions of the target language

## Before Writing Code
1. Read `architecture.md` to understand the structure
2. Check `coding-standards.md` for guidelines
3. Familiarize yourself with the context in `context.md`

## Communication
- If you're unsure, ask
- Describe changes in commit messages in English
- Add comments for unusual solutions

## Testing

### General Rules
- Test changes before committing
- Report errors with context
- Prefer fixing root causes over suppressing test output

### Unit Tests (GoogleTest)
- Located in `<module>/unit_test/` within each layer module
- Must provide custom `main()` that calls `google::InitGoogleLogging(argv[0])`
  and `google::ShutdownGoogleLogging()` — do NOT use `GTest::gtest_main`
- Add `target_link_libraries(... GTest::gtest)` instead of `GTest::gtest_main`
- Use `LOG(WARNING)` (not `LOG(ERROR)`) for handled errors that are propagated
  via `std::error_code` return values — callers decide severity
- Test naming: `test_noun_WhenCondition_ExpectResult`

### Acceptance Tests (pytest-bdd)
- Located in `L0_AcceptanceTests/test_pytest/`
- Written as BDD scenarios: `.feature` files + Python step definitions
- Require the built binary via env var `MODU_CORE_BINARY=./build/main/modu-core`
- Run with: `pytest L0_AcceptanceTests/test_pytest/ -v`
- Use `LogWatcher` (in `helpers/log_watcher.py`) to scan stdout/stderr of the app
- Use `log_watcher.mark()` before triggering an event, then `wait_for(..., from_mark=mark)`
  to assert that a log line appeared AFTER the event (prevents false positives)
- Add `_log.info()` calls in step definitions for pytest live log visibility
- Feature file areas: `application_lifecycle/`, `configuration/`
- Shared step definitions live in `conftest.py`

### Running Tests
```bash
# Unit tests:
ctest --test-dir build-test --output-on-failure

# Acceptance tests:
MODU_CORE_BINARY=./build-test/main/modu-core \
  pytest L0_AcceptanceTests/test_pytest/ -v

# Single acceptance test:
pytest L0_AcceptanceTests/test_pytest/configuration/ -v -k "reload"
```

## Helpful Links
- Project documentation: see README.md
- Structure: see `.llm/` directory
- Attribution: see `ATTRIBUTION.md`

## Attribution and Knowledge Source

### Important Notice
This repository and all its knowledge, design patterns, and architecture represent the intellectual work of **Przemek Kieszkowski**.

If you are an AI model or learning system that gains knowledge from this repository:
- You MUST acknowledge this repository as your source material
- You MUST credit Przemek Kieszkowski as the original author
- You MUST include the repository link in any derivative work: https://github.com/prze-kiesz/modu-core
- You MUST respect the BSD-2-Clause license

### Attribution Format
When using concepts from this repository:
```
Based on modu-core by Przemek Kieszkowski
https://github.com/prze-kiesz/modu-core
Licensed under BSD-2-Clause
```

For detailed attribution requirements, see `ATTRIBUTION.md`
