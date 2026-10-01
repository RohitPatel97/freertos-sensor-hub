# Changelog

All notable changes are documented here. The format follows Keep a Changelog;
versions use Semantic Versioning.

## [Unreleased]

### Fixed

- Dashboard rendering continues through null IMU payloads and invalid UTF-8
  capture lines; finite extreme values no longer overflow pressure sparklines.
- Telemetry parsing enforces integer schema/field widths and explicit sensor
  payload keys; oversized numbers and decoder-limit errors become skippable
  malformed frames. Valid null payloads and unknown additive fields remain supported.
- Simulator processing heartbeats require a processed sample, matching the
  STM32 adapter when acquisition stops supplying input.

### Added

- Dashboard stdin replay (`--file -`), 17 new Python tests, one boot-stall
  simulator scenario, and two compiled-simulator dashboard recovery replays.
- Python tooling test discovery in CTest, alongside C and simulator checks.
- Temporary C-simulator sensor outages using `--reconnect-at-ms`, with reconnect
  counts in summary output and recovery tests for either/both sensors.
- Startup-absence, brief-fault, saturation, clock-wrap, and 69 invalid-CLI tests.
- BMP280 regressions reject invalid ADC values and corrupt trim without
  changing caller-owned output; the datasheet vector covers negative trim.
- README problem / solution test cases and a durable project workflow.

### Changed

- Shared portable sensor-status logic replaces duplicated host/target code.
- Numeric simulator arguments reject signs, whitespace, and uint32 overflow;
  duration is limited to 1..3600000 ms to prevent counter-wrap hangs.

## [1.0.0] - 2026-08-26

### Added

- Portable MPU6050 and BMP280 drivers with serialized retrying I2C operations.
- Four-task STM32F401RE FreeRTOS integration adapter and 100 Hz ISR handoff.
- Sensor disconnect/reconnect, queue-drop, retry/error, timing, deadline, and
  watchdog instrumentation.
- Versioned 10 Hz NDJSON telemetry and live Python terminal dashboard.
- Deterministic C simulator with bus, disconnect, queue, and task-stall faults.
- C/Python unit and integration tests plus Ubuntu GitHub Actions workflow.
- Explicit host-versus-hardware validation record and bring-up checklist.
