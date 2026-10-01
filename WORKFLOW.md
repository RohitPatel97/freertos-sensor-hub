# FreeRTOS Sensor Hub workflow

## October 1, 2026: telemetry fault recovery

Published implementation: [9c1ca0a](https://github.com/RohitPatel97/freertos-sensor-hub/commit/9c1ca0a8ac1e5ac4417a164dfa4abaad1f2c8007).

**Problem:** a valid `imu: null` frame crashed the dashboard. Invalid UTF-8 and
oversized numeric values could also escape malformed-frame handling. The
simulator refreshed processing health even when no input was available, unlike
the STM32 adapter.

**Solution:** render missing payloads as `n/a`, decode captures line by line,
normalize parser failures, validate schema and unsigned wire widths, and provide
stdin replay with `--file -`. Processing now heartbeats only after a sample is
processed. CTest discovers the complete Python suite alongside C/integration
checks. README and protocol documentation include the updated behavior and demo.

| Regression case | Reproduced before the fix | Observed after the fix |
|---|---|---|
| MPU/all-sensor outage followed by recovery | Null IMU raises `TypeError` while rendering | All 20 snapshots render in each of two compiled-simulator recovery replays; missing values become `n/a` and later recover |
| Malformed input followed by a valid frame | Bad UTF-8, huge sensor integers, and decoder limits can abort the session | Bad lines are reported/skipped; subsequent valid frames are decoded and rendered |
| Schema and integer boundaries | `true`/`1.0` schema accepted; oversized integer fields accepted | Only integer schema 1 and uint32/uint64 field ranges accepted; maxima, zero, explicit null, and additive fields remain supported |
| Acquisition stalls at 1200 ms or from boot | Existing simulator reports only two deadline misses | Fresh simulator reports four misses, accounting for acquisition and processing; watchdog refresh withheld |

**Validation:** fresh CMake 4.4.2 / Ninja Release build on Windows, Zig 0.16.0 /
Clang 21.1.0, Python 3.12.14. All three CTest suites passed: 5 C test groups,
12 simulator scenarios, 2 dashboard recovery replays, 69 CLI rejection cases,
and 22 Python tests. Sample dashboard replay, Python syntax compilation, and
diff review passed. The old ignored executable was used only to reproduce the
heartbeat bug; acceptance ran against a fresh build of the changed sources.

**Remaining limits:** these are host tests and deterministic device/scheduling
models. Target compilation, physical UART/I2C behavior, measured timing,
watchdog resets, stack margins, and soak reliability still require hardware
artifacts. The NDJSON wire format remains version 1; previously accepted
malformed frames can now be rejected. Replay tests do not establish live serial
transport behavior or physical sensor recovery time.

## September 4, 2026: completed update

- Added temporary sensor-outage injection and reconnect summary counters.
- Shared sensor-status code between the host simulator and STM32 adapter.
- Added recovery, startup-absence, brief-dropout, status saturation, clock-wrap,
  and invalid-command-line regression cases.
- Added invalid-ADC and corrupt-calibration regressions for BMP280 compensation;
  use a fresh build, since existing ignored executables may predate source fixes.
- Documented the engineering problem, solution, and acceptance criteria in
  [README.md](README.md); hardware gates remain in [VALIDATION.md](docs/VALIDATION.md).

## Reproduce validation

Use a C99 compiler, CMake 3.16+, and Python 3.10+. Configure after installing
Python so CTest includes all three suites. Standard CMake builds run in GitHub
CI; the October 1 local run used a fresh build directory and a `zig ar` wrapper
for the Zig toolchain's archiver.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
python -m unittest discover -s tests -p "test_*.py" -v
python tools/dashboard.py --file examples/sample_telemetry.ndjson --no-ansi --max-frames 1
```

The suite contains 5 C test groups, 12 simulator scenarios, 2 dashboard recovery
replays, 69 CLI rejection cases, and 22 Python tests. Host simulation exercises portable code;
it does not compile FreeRTOS or establish measured timing on an MCU.

## Next three tasks

1. Create a version-pinned STM32CubeMX configuration and save a real target build
   log with the selected HAL, FreeRTOS, and compiler versions.
2. Measure sensor startup/reinitialization readiness and worst-case I2C retries;
   collect physical unplug/replug and waveform evidence before claiming a
   hardware recovery time or 10 ms acquisition deadline.
3. Capture watchdog resets and task stack high-water marks on the board, then
   evaluate DMA UART only if measurements justify the additional complexity.

For each change, record the problem, solution, focused regression, observed
result, remaining limits, and published implementation commit here. Run the
commands above, review the diff, and inspect CI after publishing.
Resume claims may cite embedded C, STM32/FreeRTOS design, I2C/UART, interrupts,
bounded queues, mutexes, fault injection, unit/integration testing, CMake, and CI.
Physical firmware builds, logic-analyzer captures, timing, and soak tests remain
unverified until their artifacts are recorded.
