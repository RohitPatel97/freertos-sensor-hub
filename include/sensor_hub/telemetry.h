#ifndef SENSOR_HUB_TELEMETRY_H
#define SENSOR_HUB_TELEMETRY_H

#include <stddef.h>

#include "sensor_hub/types.h"

int hub_telemetry_format(char *buffer, size_t capacity,
                         const HubProcessedSample *sample,
                         const HubStatus *status,
                         const HubCounters *counters);

#endif
