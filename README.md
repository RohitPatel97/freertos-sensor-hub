# FreeRTOS Sensor Hub

[![CI](https://github.com/RohitPatel97/freertos-sensor-hub/actions/workflows/ci.yml/badge.svg)](https://github.com/RohitPatel97/freertos-sensor-hub/actions/workflows/ci.yml)

A fault-aware sensor acquisition project for **STM32F401RE**, **MPU6050**, and
**BMP280**. Four FreeRTOS tasks coordinate 100 Hz acquisition, processing, 10 Hz
UART telemetry, and watchdog decisions. A deterministic C simulator and Python
dashboard make sensor failures and recovery reproducible without hardware.

**Status:** the host application is tested, and the complete STM32 firmware
cross-compiles with pinned HAL, CMSIS, and FreeRTOS sources. Physical board
bring-up, electrical captures, timing measurements, and soak testing remain
pending. See [validation evidence](docs/VALIDATION.md).

## Run and test on your computer

Requires CMake 3.20+, a C99 compiler, and Python 3.10+. Python is required when
tests are enabled, so missing tooling cannot silently skip integration checks.

```bash
cmake -S . -B build/host -DCMAKE_BUILD_TYPE=Release
cmake --build build/host --config Release --parallel
ctest --test-dir build/host -C Release --output-on-failure
./build/host/sensor_hub_sim --duration-ms 3000
```

With a Windows multi-configuration generator, use
`build/host/Release/sensor_hub_sim.exe`. Pass `-DBUILD_TESTING=OFF` when only the
C library and simulator are needed.

### Watch an outage and recovery

```bash
./build/host/sensor_hub_sim --duration-ms 2000 --disconnect mpu --disconnect-at-ms 500 --reconnect-at-ms 1000 | python tools/dashboard.py --file - --no-ansi
```

The 20 snapshots show IMU values changing to `n/a`, the debounced sensor state
becoming `OFFLINE`, and readings returning after reconnection. The barometer
continues reporting. Use `--disconnect all` to remove both sensors. The simulator
writes telemetry to stdout and a separate JSON summary to stderr.

Without a C compiler, exercise the Python tooling model:

```bash
python tools/reference_sim.py --duration-ms 1000 | python tools/dashboard.py --file - --no-ansi
python -m unittest discover -s tests -p "test_*.py" -v
```

The Python model demonstrates the protocol; compiled C simulation supplies the
driver and scheduling regression evidence. More fault options, limits, and
expected results are in [the simulator guide](docs/SIMULATOR.md).

## Build the STM32 firmware

Requires Git, Ninja, CMake 3.20+, Python 3.10+, and Arm GNU Toolchain
**13.2.Rel1** (`arm-none-eabi-gcc` on PATH). A separate build directory prevents
mixing host and ARM objects. The first configure fetches pinned dependencies.

```bash
cmake -S . -B build/stm32 -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake -DHUB_BUILD_STM32=ON -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build/stm32 --parallel
```

PowerShell users should quote the toolchain argument:
`"-DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake"`.

Outputs in `build/stm32/target/stm32f401re/`:

- `sensor_hub_firmware.elf`: linked firmware and symbols.
- `sensor_hub_firmware.hex` / `.bin`: images for board programming.
- `sensor_hub_firmware.map`: memory placement; compiler `.su` files report local stack use.

The build checks flash size, initial stack pointer, five interrupt vectors, and
linked application/FreeRTOS symbols. CI builds both watchdog configurations and
publishes the images, maps, logs, and stack reports as downloadable artifacts.

The hardware watchdog defaults to **disabled** for initial bring-up. After UART
and task health are understood, configure a separate build with
`-DHUB_ENABLE_WATCHDOG=ON`. A successful build does not establish correct
on-board operation. Wiring, dependency revisions, configuration, and bring-up
gates are in [the target guide](target/stm32f401re/README.md).

## Design and failure behavior

```text
TIM2 ISR -> binary semaphore -> acquisition -> raw queue (8)
                                  |                  |
                             I2C mutex           processing
                              /     \                |
                          MPU6050  BMP280    latest-value queue (1)
                                                     |
Four task heartbeats -> health/watchdog         UART telemetry
                                                     |
                                               Python dashboard
```

| Mechanism | Behavior |
|---|---|
| Bounded queues | Acquisition never waits for a consumer; full queues increment drop counters |
| Shared I2C mutex | A complete transaction and up to two retries use one lock hold |
| Sensor state | Three failed acquisitions mark a device offline; successful reinitialization restores data |
| Task health | Missing heartbeats withhold watchdog service; processing must actually consume a sample |
| Telemetry | Version-1 NDJSON; invalid sensor samples are `null`, even before offline debounce completes |
| Dashboard | File, stdin, demo, or optional serial input; malformed lines are skipped with diagnostics |

[Architecture and task deadlines](docs/ARCHITECTURE.md) ·
[Protocol specification](docs/PROTOCOL.md)

## Verification and optimization

October 12, 2026 validation:

- **6 C test groups**, **12 simulator scenarios**, **69 CLI rejection cases**.
- **22 Python tests** and **2 simulator-to-dashboard recovery replays**.
- Real STM32 ELF/HEX/BIN builds with watchdog off and on; interrupt-vector and memory checks pass.
- Eight simulator cases retain byte-for-byte telemetry and summary output after formatter changes.

The formatter writes directly into the caller's buffer, removing 352 bytes of
temporary payload arrays. With Arm GCC 13.2.1 at `-Os`, its reported local stack
frame decreases from **448 to 96 bytes**, plus a **28-byte append helper**.
These are compiler reports, excluding libc callees; on-board stack margins and
execution time still need measurement. Non-finite active payloads and buffer
truncation fail cleanly without exposing a previous frame.

## Live dashboard and hardware limits

```bash
python -m pip install -r requirements.txt
python tools/dashboard.py --port COM5 --baud 115200
```

Use `--file capture.ndjson` for replay, `--file -` for stdin, `--demo` for a
simulated feed, and `--max-frames N` to stop after N valid frames. Serial input
requires the optional pyserial dependency.

Pending physical work includes sensor identity/waveforms, UART capture, DWT
timing distributions, unplug/replug readiness, watchdog reset capture, stack
high-water marks, and soak/power-cycle tests. BMP280 conversion is asynchronous
to 100 Hz polling, so readings can repeat. Target timestamps use the 1 kHz RTOS
tick and wrap with that counter; DWT supplies duration measurements.

[Validation checklist](docs/VALIDATION.md) · [Project workflow](WORKFLOW.md) ·
[Contributing](CONTRIBUTING.md) · [Changelog](CHANGELOG.md)

## License

MIT for project code; fetched dependencies retain their upstream licenses.
See [LICENSE](LICENSE) and [dependency versions/licenses](target/stm32f401re/README.md#pinned-dependencies).
