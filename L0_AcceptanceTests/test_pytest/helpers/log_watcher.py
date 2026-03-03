# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Przemek Kieszkowski

"""
LogWatcher - reads lines from a subprocess stream in a background thread
and provides wait_for() to block until a matching line is found or timeout.
"""

import re
import threading
import time
from collections import deque
from typing import Optional


class LogWatcher:
    """
    Reads lines from a stream (typically subprocess stderr) in a background
    thread. Provides wait_for() to assert that a specific log message appears
    within a timeout.

    Usage:
        watcher = LogWatcher(process.stderr)
        watcher.start()
        watcher.wait_for("Application daemon has successfully started up.", timeout=5)
    """

    def __init__(self, stream, encoding: str = "utf-8") -> None:
        self._stream = stream
        self._encoding = encoding
        self._lines: deque[str] = deque()
        self._lock = threading.Lock()
        self._new_line_event = threading.Event()
        self._thread: Optional[threading.Thread] = None
        self._stopped = False

    def start(self) -> "LogWatcher":
        """Start background reader thread."""
        self._thread = threading.Thread(target=self._reader, daemon=True)
        self._thread.start()
        return self

    def stop(self) -> None:
        """Signal the reader to stop (stream close will unblock it)."""
        self._stopped = True

    def _reader(self) -> None:
        try:
            for raw_line in self._stream:
                if self._stopped:
                    break
                if isinstance(raw_line, bytes):
                    line = raw_line.decode(self._encoding, errors="replace")
                else:
                    line = raw_line
                line = line.rstrip("\n")
                with self._lock:
                    self._lines.append(line)
                self._new_line_event.set()
        except (OSError, ValueError):
            pass  # stream closed

    def lines(self) -> list[str]:
        """Return a copy of all lines captured so far."""
        with self._lock:
            return list(self._lines)

    def mark(self) -> int:
        """
        Return the current number of captured lines.

        Use the returned value as ``from_mark`` in a subsequent ``wait_for()``
        call to restrict matching to lines that arrive *after* this point.

        Example::
            pos = app.logs.mark()
            app.sighup()
            app.logs.wait_for("Reload complete", from_mark=pos)
        """
        with self._lock:
            return len(self._lines)

    def wait_for(
        self,
        pattern: str,
        timeout: float = 10.0,
        regexp: bool = False,
        from_mark: int = 0,
    ) -> str:
        """
        Block until a line matching *pattern* appears in the log, or raise
        TimeoutError.

        Args:
            pattern:   Substring to search for (or regex if regexp=True).
            timeout:   Maximum seconds to wait.
            regexp:    If True, treat pattern as a regular expression.
            from_mark: Start scanning from this line index (see :meth:`mark`).
                       Defaults to 0 (scan from the beginning).

        Returns:
            The matched line.

        Raises:
            TimeoutError: If no matching line is found within *timeout* seconds.
        """
        deadline = time.monotonic() + timeout
        checked_up_to: int = from_mark

        while True:
            with self._lock:
                snapshot = list(self._lines)

            for line in snapshot[checked_up_to:]:
                if regexp:
                    if re.search(pattern, line):
                        return line
                else:
                    if pattern in line:
                        return line

            checked_up_to = len(snapshot)

            remaining = deadline - time.monotonic()
            if remaining <= 0:
                all_logs = "\n  ".join(snapshot) if snapshot else "<no output>"
                raise TimeoutError(
                    f"Pattern {pattern!r} not found within {timeout}s.\n"
                    f"Captured log lines:\n  {all_logs}"
                )

            # Wait for a new line or check again after a short sleep
            self._new_line_event.wait(timeout=min(0.1, remaining))
            self._new_line_event.clear()
