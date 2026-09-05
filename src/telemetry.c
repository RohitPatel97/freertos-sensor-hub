#include "sensor_hub/telemetry.h"
#include "sensor_hub/config.h"

#include <stdio.h>

static const char *state(bool online) {
    return online ? "online" : "offline";
}

int hub_telemetry_format(char *buffer, size_t capacity,
                         const HubProcessedSample *sample,
                         const HubStatus *status,
                         const HubCounters *counters) {
    char imu_json[192];
    char baro_json[160];
    int part_written;
    int written;

    if (buffer == NULL || capacity == 0U || sample == NULL ||
        status == NULL || counters == NULL) {
        return -1;
    }

    if ((sample->valid_mask & HUB_MPU6050_VALID) != 0U) {
        part_written = snprintf(
            imu_json, sizeof(imu_json),
            "{\"accel_g\":[%.4f,%.4f,%.4f],\"gyro_dps\":[%.3f,%.3f,%.3f],\"temperature_c\":%.2f}",
            (double)sample->accel_g[0], (double)sample->accel_g[1],
            (double)sample->accel_g[2], (double)sample->gyro_dps[0],
            (double)sample->gyro_dps[1], (double)sample->gyro_dps[2],
            (double)sample->imu_temperature_c);
        if (part_written < 0 || (size_t)part_written >= sizeof(imu_json)) {
            return -1;
        }
    } else {
        (void)snprintf(imu_json, sizeof(imu_json), "null");
    }
    if ((sample->valid_mask & HUB_BMP280_VALID) != 0U) {
        part_written = snprintf(
            baro_json, sizeof(baro_json),
            "{\"temperature_c\":%.2f,\"pressure_pa\":%.1f,\"altitude_m\":%.2f}",
            (double)sample->baro_temperature_c, (double)sample->pressure_pa,
            (double)sample->altitude_m);
        if (part_written < 0 || (size_t)part_written >= sizeof(baro_json)) {
            return -1;
        }
    } else {
        (void)snprintf(baro_json, sizeof(baro_json), "null");
    }

    written = snprintf(
        buffer, capacity,
        "{\"schema\":1,\"type\":\"telemetry\",\"seq\":%lu,\"ts_us\":%llu,"
        "\"sensors\":{\"mpu6050\":\"%s\",\"bmp280\":\"%s\"},"
        "\"imu\":%s,\"baro\":%s,"
        "\"health\":{\"i2c_retries\":%lu,\"i2c_errors\":%lu,\"queue_drops\":%lu,"
        "\"deadline_misses\":%lu,\"watchdog\":\"%s\"},"
        "\"timing\":{\"acquisition_us\":%lu,\"processing_us\":%lu}}\n",
        (unsigned long)sample->sequence,
        (unsigned long long)sample->timestamp_us,
        state(status->mpu6050_online), state(status->bmp280_online),
        imu_json, baro_json, (unsigned long)counters->i2c_retries,
        (unsigned long)counters->i2c_errors,
        (unsigned long)(counters->raw_queue_drops + counters->telemetry_queue_drops),
        (unsigned long)counters->deadline_misses,
        status->watchdog_fed ? "fed" : "withheld",
        (unsigned long)sample->acquisition_time_us,
        (unsigned long)sample->processing_time_us);
    if (written < 0 || (size_t)written >= capacity) {
        if (capacity > 0U) {
            buffer[0] = '\0';
        }
        return -1;
    }
    return written;
}
