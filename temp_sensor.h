/*
 * temp_sensor.h: Starter code for ECE 266 Lab 6, spring 2023
 *
 * Temperature sensor module
 *
 * Created by Zhao Zhang
 */

#ifndef TEMP_SENSOR_H_
#define TEMP_SENSOR_H_

#include <stdbool.h>
#include <stdint.h>

// Initialize ADC to use Tiva C's internal temperature sensor
void TsInit();

// Trigger temperature reading
void TsTriggerSampling();

// Check if any new reading is ready
bool TsNewDataReady();

// Read the most recent temperature reading
uint32_t TsReadRaw();

#endif /* TEMP_SENSOR_H_ */
