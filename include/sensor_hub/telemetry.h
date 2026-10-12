#ifndef SENSOR_HUB_TELEMETRY_H
#define SENSOR_HUB_TELEMETRY_H

#include <stddef.h>

#include "sensor_hub/types.h"

/* Return frame bytes (excluding the terminating NUL), or -1 for invalid
 * arguments, non-finite active payloads, or insufficient capacity. On failure
 * a non-NULL buffer with nonzero capacity is always an empty C string. */
int hub_telemetry_format(char *buffer, size_t capacity,
                         const HubProcessedSample *sample,
                         const HubStatus *status,
                         const HubCounters *counters);

#endif
