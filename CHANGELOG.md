# Changelog

All notable changes are documented here. The format follows Keep a Changelog;
versions use Semantic Versioning.

## [Unreleased]

### Added

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
