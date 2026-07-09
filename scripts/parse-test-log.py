#!/usr/bin/env python3
"""Parse a doors test log and print a summary.

Normal mode (default):
    parse-test-log.py <log-path>

Crash-test mode:
    parse-test-log.py --crash-type=panic --qemu-exit=<code> <log-path>
"""

import argparse
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
  with open(log_path, encoding="utf-8") as f:
    for line in f:
      line = line.strip()
      if not line.startswith("{"):
        continue
      try:
        events.append(json.loads(line))
      except json.JSONDecodeError:
        pass
  return events


def check_normal(log_path, events):
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


def check_crash(events, crash_type, qemu_exit):
  crash_evs = [e for e in events if e.get("event") == "crash"]

  if not crash_evs:
    print("  [FAIL] No crash event found in log")
    sys.exit(1)

  ev = crash_evs[0]
  print(f"  [EVENT] {ev['name']}")

  # qemu_exit is a string ("0", "1", "Timeout", etc.) or None
  is_clean = qemu_exit == "0"

  if crash_type in ("poweroff", "reboot"):
    if qemu_exit is not None and not is_clean:
      print(f"  [FAIL] Expected clean exit for {crash_type}, got \"{qemu_exit}\"")
      sys.exit(1)
    label = "powered off" if crash_type == "poweroff" else "rebooted"
    print(f"  [PASS] system {label} cleanly")
  else:
    if qemu_exit is not None and is_clean:
      print("  [FAIL] Expected crash/halt, but QEMU exited 0")
      sys.exit(1)
    print(f"  [PASS] system crashed/halted as expected (QEMU: {qemu_exit or 'not checked'})")

  sys.exit(0)


def check_log_file(log_path, args):
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

  if args.crash_type:
    check_crash(events, args.crash_type, args.qemu_exit)
  else:
    check_normal(log_path, events)


def main():
  parser = argparse.ArgumentParser(description="Parse doors test log")
  parser.add_argument("--crash-type", choices=["panic", "halt", "reboot", "poweroff"],
                      help="Validate as a single-shot crash test of this type")
  parser.add_argument("--qemu-exit", type=str, default=None,
                      help="QEMU exit code or status string (for crash-test validation)")
  parser.add_argument("log", nargs="?", default=None, help="Path to log file")
  args = parser.parse_args()

  log_path = Path(args.log) if args.log else Path("doors-test.log")
  check_log_file(log_path, args)


main()
