#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

void MsInit();

void MsTriggerSampling();

bool MsNewDataReady();

uint32_t MsReadRaw();


#endif // MOISTURE_SENSOR_H
