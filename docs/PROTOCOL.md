# Telemetry protocol v1

Transport is UTF-8 newline-delimited JSON (NDJSON), one object per line, at
115200 baud, 8 data bits, no parity, one stop bit. Firmware emits telemetry at
10 Hz. Consumers should impose a reasonable line limit (the firmware buffer is
768 bytes), ignore unknown fields, and select decoding behavior from `schema`.

## Required top-level fields

| Field | Type | Meaning |
|---|---|---|
| `schema` | integer | Protocol version; currently `1` |
| `type` | string | `telemetry` |
| `seq` | non-negative integer | Acquisition sequence of the reported latest sample; wraps at 32 bits |
| `ts_us` | non-negative integer | Acquisition timestamp in microseconds since boot |
| `sensors` | object | `mpu6050` and `bmp280`, each `online` or `offline` |
| `imu` | object or null | Scaled acceleration, angular rate, and die temperature |
| `baro` | object or null | Compensated temperature/pressure and derived altitude |
| `health` | object | Cumulative fault/backpressure counters and watchdog decision |
| `timing` | object | Acquisition and processing duration in microseconds |

The target sends `null` when an acquisition contains no valid data for a sensor.
Sensor state changes only after the configured consecutive-failure threshold,
so a frame can contain `null` while state is still `online` during the debounce
window. Consumers must use payload presence for per-sample validity and the
state field for the debounced health status.

## Counter semantics

- `i2c_retries`: additional physical attempts after an initial failed attempt.
- `i2c_errors`: logical I2C operations that failed after all attempts, plus mutex
  acquisition timeouts.
- `queue_drops`: sum of raw and telemetry queue drops. The simulator summary
  exposes more detail.
- `deadline_misses`: task deadline failures observed by the 1 Hz health task.
- `watchdog`: the most recent health decision, `fed` or `withheld`.

All counters are unsigned and monotonic modulo 32 bits. A consumer should treat a
lower value after reconnect as reboot/wrap, not a negative delta.

## Parser behavior

`tools/telemetry.py` rejects malformed JSON, unsupported schemas, negative
counters, invalid states, and missing required objects. It preserves the raw
mapping so future additive fields remain available. `tools/dashboard.py` skips
bad lines to tolerate boot banners or partial serial reads.
