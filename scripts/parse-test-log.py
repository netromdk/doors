#!/usr/bin/env python3
"""Parse a doors-test.log and print a summary."""

import json
import sys
from pathlib import Path


_INSTRUCTIONS = """\
The integration test suite was probably not run yet.
To run it:

  1. Configure with `BUILD_INTEGRATION_TESTS` enabled:
       cmake --preset integration-test
     Or:
       cmake -DBUILD_INTEGRATION_TESTS=ON -DSERIAL_DEBUG=ON <path-to-source>

  2. Build the test ISO:
       ninja test-iso

  3. Boot it in QEMU and capture serial output to doors-test.log:
       ninja run-int-test

  4. Run this check:
       ninja check-int-test
"""


def parse_events(log_path):
  events = []
  with open(log_path) as f:
    for line in f:
      line = line.strip()
      if not line.startswith("{"):
        continue
      try:
        events.append(json.loads(line))
      except json.JSONDecodeError:
        pass
  return events


def main():
  log_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("doors-test.log")

  if not log_path.exists():
    print(f"Error: {log_path} not found.")
    print()
    print(_INSTRUCTIONS)
    sys.exit(2)

  if log_path.stat().st_size == 0:
    print(f"Error: {log_path} is empty.")
    print("QEMU may not have started, or serial capture failed before any output was written.")
    print()
    print(_INSTRUCTIONS)
    sys.exit(2)

  events = parse_events(log_path)

  if not events:
    print(f"Error: {log_path} contains no JSON events.")
    print("The kernel may have crashed before the test runner started,")
    print("or the file is from a non-test boot (missing `--test` cmdline flag).")
    sys.exit(2)

  passed_evs = []
  failed_evs = []
  last_run = None
  done = None

  for ev in events:
    kind = ev.get("event")
    if kind == "run":
      last_run = ev.get("name")
    elif kind == "pass":
      passed_evs.append(ev)
      last_run = None
    elif kind == "fail":
      failed_evs.append(ev)
      last_run = None
    elif kind == "done":
      done = ev

  crashed = last_run is not None and done is None

  print(f"Test log: {log_path}")
  print()

  for ev in passed_evs:
    print(f"  [PASS] {ev['name']} ({ev.get('ms', '?')}ms)")

  for ev in failed_evs:
    print(f"  [FAIL] {ev['name']}: {ev.get('reason', 'unknown')} ({ev.get('ms', '?')}ms)")

  if crashed:
    print(f"  [CRASH] kernel crashed during: {last_run}")

  print()

  if done:
    passed = done.get("passed", 0)
    failed = done.get("failed", 0)
    total = done.get("total", passed + failed)
    ms = done.get("ms", "?")
    status = "OK" if failed == 0 else "FAILED"
    print(f"Result: {status} "
          f"({passed} passed, {failed} failed, "
          f"{total} total, {ms}ms)")
    sys.exit(0 if failed == 0 else 1)
  elif crashed:
    print("Result: CRASH. Suite did not complete!")
    print("Check the boot output above the JSON events for a panic or exception.")
    sys.exit(1)
  else:
    print("Result: INCOMPLETE. No 'done' event found!")
    sys.exit(2)


main()
