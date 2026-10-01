# Contributing

Thanks for improving the sensor hub. Keep changes small, testable, and honest
about their validation boundary.

## Development workflow

1. Create a focused branch and explain the failure mode or capability in the PR.
2. Build with strict warnings and run `ctest --test-dir build --output-on-failure`.
3. Run `python -m unittest discover -s tests -p "test_*.py" -v` for Python-only
   changes (also included in CTest when Python is available).
4. Add a deterministic regression for fixes to drivers, counters, scheduling, or
   the protocol. Avoid wall-clock sleeps and random unseeded inputs.
5. Update `docs/PROTOCOL.md` for wire changes and `docs/VALIDATION.md` for new
   evidence or hardware requirements.

Portable code must remain C99, avoid heap allocation, use fixed-width types at
wire/hardware boundaries, and compile with the repository warning policy. Keep
HAL and FreeRTOS dependencies inside `target/`. Python runtime tools should use
the standard library except for the optional serial transport.

Hardware claims require an artifact and the exact board/tool versions. A host
simulation pass must not be described as physical validation.

## Commit and review guidance

Use imperative commit subjects (for example, `Detect terminal I2C retries`). A
review should check ISR legality, bounded blocking, queue policy, unit scaling,
integer overflow, counter semantics, watchdog failure behavior, and protocol
compatibility. Security issues should be reported privately to the repository
owner rather than opened with exploit details.
