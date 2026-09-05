"""Strict parser for the sensor hub's newline-delimited JSON protocol."""

from __future__ import annotations

import json
import math
from dataclasses import dataclass
from typing import Any, Iterable, Iterator, Mapping


class TelemetryError(ValueError):
    """Raised when a line is JSON but not a supported telemetry frame."""


@dataclass(frozen=True)
class TelemetryFrame:
    sequence: int
    timestamp_us: int
    mpu6050_online: bool
    bmp280_online: bool
    imu: Mapping[str, Any] | None
    baro: Mapping[str, Any] | None
    i2c_retries: int
    i2c_errors: int
    queue_drops: int
    deadline_misses: int
    watchdog_fed: bool
    acquisition_us: int
    processing_us: int
    raw: Mapping[str, Any]


def _mapping(value: Any, name: str) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        raise TelemetryError(f"{name} must be an object")
    return value


def _integer(value: Any, name: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise TelemetryError(f"{name} must be a non-negative integer")
    return value


def _number(value: Any, name: str) -> int | float:
    if (isinstance(value, bool) or not isinstance(value, (int, float)) or
            not math.isfinite(value)):
        raise TelemetryError(f"{name} must be numeric")
    return value


def _vector3(value: Any, name: str) -> None:
    if not isinstance(value, list) or len(value) != 3:
        raise TelemetryError(f"{name} must be a three-element array")
    for index, item in enumerate(value):
        _number(item, f"{name}[{index}]")


def decode_line(line: str | bytes) -> TelemetryFrame:
    """Decode and validate one protocol-v1 NDJSON telemetry line."""
    try:
        payload = json.loads(
            line,
            parse_constant=lambda value: (_ for _ in ()).throw(
                TelemetryError(f"non-finite JSON number: {value}")
            ),
        )
    except (json.JSONDecodeError, UnicodeDecodeError) as exc:
        raise TelemetryError(f"invalid JSON: {exc}") from exc
    root = _mapping(payload, "frame")
    if root.get("schema") != 1 or root.get("type") != "telemetry":
        raise TelemetryError("expected a schema-1 telemetry frame")
    sensors = _mapping(root.get("sensors"), "sensors")
    health = _mapping(root.get("health"), "health")
    timing = _mapping(root.get("timing"), "timing")
    for sensor_name in ("mpu6050", "bmp280"):
        if sensors.get(sensor_name) not in ("online", "offline"):
            raise TelemetryError(f"invalid state for {sensor_name}")
    if health.get("watchdog") not in ("fed", "withheld"):
        raise TelemetryError("invalid watchdog state")
    imu = root.get("imu")
    baro = root.get("baro")
    if imu is not None:
        imu = _mapping(imu, "imu")
        _vector3(imu.get("accel_g"), "imu.accel_g")
        _vector3(imu.get("gyro_dps"), "imu.gyro_dps")
        _number(imu.get("temperature_c"), "imu.temperature_c")
    if baro is not None:
        baro = _mapping(baro, "baro")
        _number(baro.get("temperature_c"), "baro.temperature_c")
        _number(baro.get("pressure_pa"), "baro.pressure_pa")
        _number(baro.get("altitude_m"), "baro.altitude_m")
    return TelemetryFrame(
        sequence=_integer(root.get("seq"), "seq"),
        timestamp_us=_integer(root.get("ts_us"), "ts_us"),
        mpu6050_online=sensors["mpu6050"] == "online",
        bmp280_online=sensors["bmp280"] == "online",
        imu=imu,
        baro=baro,
        i2c_retries=_integer(health.get("i2c_retries"), "i2c_retries"),
        i2c_errors=_integer(health.get("i2c_errors"), "i2c_errors"),
        queue_drops=_integer(health.get("queue_drops"), "queue_drops"),
        deadline_misses=_integer(health.get("deadline_misses"), "deadline_misses"),
        watchdog_fed=health["watchdog"] == "fed",
        acquisition_us=_integer(timing.get("acquisition_us"), "acquisition_us"),
        processing_us=_integer(timing.get("processing_us"), "processing_us"),
        raw=root,
    )


def decode_stream(lines: Iterable[str | bytes], *, ignore_invalid: bool = False) -> Iterator[TelemetryFrame]:
    """Yield frames from a text or serial line iterator."""
    for line in lines:
        if not line or not line.strip():
            continue
        try:
            yield decode_line(line)
        except TelemetryError:
            if not ignore_invalid:
                raise
