# FreeRTOS Sensor Hub workflow

## Completed update

- Added temporary sensor-outage injection and reconnect summary counters.
- Shared sensor-status code between the host simulator and STM32 adapter.
- Added recovery, startup-absence, brief-dropout, status saturation, clock-wrap,
  and invalid-command-line regression cases.
- Added invalid-ADC and corrupt-calibration regressions for BMP280 compensation;
  use a fresh build, since existing ignored executables may predate source fixes.
- Documented the engineering problem, solution, and acceptance criteria in
  [README.md](README.md); hardware gates remain in [VALIDATION.md](docs/VALIDATION.md).

## Reproduce validation

September 4, 2026: fresh native Zig 0.16.0 / Clang 21.1.0 builds with
`-std=c99 -Wall -Wextra -Wpedantic -Werror` passed all 5 C test groups,
11 simulator scenarios, and 69 CLI rejection cases. The dependency-free
Python tools passed 5 tests, and the sample dashboard replay passed.
Executables were built directly from current `src/*.c`,
`tests/c/test_core.c`, and `sim/host_sim.c`; pre-existing binaries were excluded.
Standard CMake builds are also configured in GitHub CI.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python -m unittest discover -s tests -p "test_python_tools.py" -v
python tools/dashboard.py --file examples/sample_telemetry.ndjson --no-ansi --max-frames 1
```

The suite contains 5 C test groups, 11 simulator scenarios, and 69 CLI rejection
cases, plus the Python tooling suite. Host simulation exercises portable code;
it does not compile FreeRTOS or establish measured timing on an MCU.

## Next three tasks

1. Create a version-pinned STM32CubeMX configuration and save a real target build
   log with the selected HAL, FreeRTOS, and compiler versions.
2. Measure sensor startup/reinitialization readiness and worst-case I2C retries;
   collect physical unplug/replug and waveform evidence before claiming a
   hardware recovery time or 10 ms acquisition deadline.
3. Capture watchdog resets and task stack high-water marks on the board, then
   evaluate DMA UART only if measurements justify the additional complexity.

For each change, reproduce the problem, add a focused test, run the commands
above, update the evidence, review the diff, and inspect CI after publishing.
Resume claims may cite embedded C, STM32/FreeRTOS design, I2C/UART, interrupts,
bounded queues, mutexes, fault injection, unit/integration testing, CMake, and CI.
Physical firmware builds, logic-analyzer captures, timing, and soak tests remain
unverified until their artifacts are recorded.
