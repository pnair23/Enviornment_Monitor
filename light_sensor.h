#ifndef LIGHT_SENSOR_H_
#define LIGHT_SENSOR_H_

#include <stdbool.h>
#include <stdint.h>

// Initialize ADC to use the light sensor
void LsInit();

// Trigger light reading
void LsTriggerSampling();

// Check if any new reading is ready
bool LsNewDataReady();

// Read the most recent light reading
uint32_t LsReadRaw();

#endif /* LIGHT_SENSOR_H_ */
