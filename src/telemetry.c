#include "sensor_hub/telemetry.h"
#include "sensor_hub/config.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>

typedef struct {
    char *buffer;
    size_t capacity;
    size_t used;
} FrameWriter;

static bool append(FrameWriter *writer, const char *format, ...) {
    int count;
    const size_t remaining = writer->capacity - writer->used;
    va_list arguments;
    va_start(arguments, format);
    count = vsnprintf(writer->buffer + writer->used, remaining, format, arguments);
    va_end(arguments);
    if (count < 0 || (size_t)count >= remaining) { return false; }
    writer->used += (size_t)count;
    return true;
}

static bool finite_payloads(const HubProcessedSample *sample) {
    unsigned int axis;
    if ((sample->valid_mask & HUB_MPU6050_VALID) != 0U) {
        for (axis = 0U; axis < 3U; ++axis) {
            if (!isfinite(sample->accel_g[axis]) || !isfinite(sample->gyro_dps[axis])) {
                return false;
            }
        }
        if (!isfinite(sample->imu_temperature_c)) { return false; }
    }
    return (sample->valid_mask & HUB_BMP280_VALID) == 0U ||
           (isfinite(sample->baro_temperature_c) && isfinite(sample->pressure_pa) &&
            isfinite(sample->altitude_m));
}

int hub_telemetry_format(char *buffer, size_t capacity,
                         const HubProcessedSample *sample,
                         const HubStatus *status,
                         const HubCounters *counters) {
    FrameWriter writer = {buffer, capacity, 0U};
    if (buffer == NULL || capacity == 0U) { return -1; }
    buffer[0] = '\0';
    if (sample == NULL || status == NULL || counters == NULL || !finite_payloads(sample)) {
        return -1;
    }

    /* Write into the caller's bounded buffer, avoiding 352 bytes of temporary
       payload arrays and their final copies into the frame. */
    if (!append(&writer,
                "{\"schema\":1,\"type\":\"telemetry\",\"seq\":%lu,\"ts_us\":%llu,"
                "\"sensors\":{\"mpu6050\":\"%s\",\"bmp280\":\"%s\"},\"imu\":",
                (unsigned long)sample->sequence, (unsigned long long)sample->timestamp_us,
                status->mpu6050_online ? "online" : "offline",
                status->bmp280_online ? "online" : "offline")) { goto failed; }
    if ((sample->valid_mask & HUB_MPU6050_VALID) != 0U) {
        if (!append(&writer,
                    "{\"accel_g\":[%.4f,%.4f,%.4f],\"gyro_dps\":[%.3f,%.3f,%.3f],\"temperature_c\":%.2f}",
                    (double)sample->accel_g[0], (double)sample->accel_g[1],
                    (double)sample->accel_g[2], (double)sample->gyro_dps[0],
                    (double)sample->gyro_dps[1], (double)sample->gyro_dps[2],
                    (double)sample->imu_temperature_c)) { goto failed; }
    } else if (!append(&writer, "null")) { goto failed; }
    if ((sample->valid_mask & HUB_BMP280_VALID) != 0U) {
        if (!append(&writer,
                    ",\"baro\":{\"temperature_c\":%.2f,\"pressure_pa\":%.1f,\"altitude_m\":%.2f}",
                    (double)sample->baro_temperature_c, (double)sample->pressure_pa,
                    (double)sample->altitude_m)) { goto failed; }
    } else if (!append(&writer, ",\"baro\":null")) { goto failed; }
    if (!append(&writer,
                ",\"health\":{\"i2c_retries\":%lu,\"i2c_errors\":%lu,\"queue_drops\":%lu,"
                "\"deadline_misses\":%lu,\"watchdog\":\"%s\"},"
                "\"timing\":{\"acquisition_us\":%lu,\"processing_us\":%lu}}\n",
                (unsigned long)counters->i2c_retries, (unsigned long)counters->i2c_errors,
                (unsigned long)(counters->raw_queue_drops + counters->telemetry_queue_drops),
                (unsigned long)counters->deadline_misses,
                status->watchdog_fed ? "fed" : "withheld",
                (unsigned long)sample->acquisition_time_us,
                (unsigned long)sample->processing_time_us)) { goto failed; }
    return (int)writer.used;

failed:
    buffer[0] = '\0';
    return -1;
}
