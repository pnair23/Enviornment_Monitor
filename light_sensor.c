#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/adc.h>
#include "light_sensor.h"
//All temps changed to light and sequence #0 changed to sequence #1
/*
 * Global declarations
 */
// Light sensor reading states
typedef struct {

uint32_t raw_data;

bool new_data_ready;
} LightSensorState;
static LightSensorState light_sensor = {0, false};
// Pre-declare the ISR function
static void LightSensorISR();
/*
 * Initialize ADC to use light sensor
 *
 * Resources: ADC0, sequence #1, special channel 7 (light sensor)
 * Configurations: processor trigger, interrupt enabled, use step 0 only
 */
void LsInit()
{

// Enable the ADC0 peripheral

SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

// Configure ADC0's sequencer #1

ADCSequenceConfigure(ADC0_BASE, 1 /* sequencer */, ADC_TRIGGER_PROCESSOR, 0 /* priority */);

// Configure step 0 of sequencer 1 to use the light sensor, with

// interrupt enable, as the end of the sequence

ADCSequenceStepConfigure(ADC0_BASE, 1 /* sequencer */, 0 /* step */,

ADC_CTL_CH7 | ADC_CTL_IE | ADC_CTL_END);

// Enable ADC0, sequencer 1

ADCSequenceEnable(ADC0_BASE, 1 /* sequencer */);

// Configure the interrupt system for light sensor

ADCIntRegister(ADC0_BASE, 1 /* sequencer */, LightSensorISR);

IntPrioritySet(INT_ADC0SS1, 0);

ADCIntEnable(ADC0_BASE, 1 /* sequencer */);
}
/*
 * Trigger light reading
 */
void LsTriggerSampling()
{

light_sensor.new_data_ready = false;

ADCProcessorTrigger(ADC0_BASE, 1);
}
/*
 * Check if any new data is ready
 */
bool LsNewDataReady()
{

return light_sensor.new_data_ready;
}
/*
 * Read the current input from the light sensor
 */
uint32_t LsReadRaw()
{

// Reset new data ready flag and return the reading

light_sensor.new_data_ready = false;

return light_sensor.raw_data;
}
/*
 * ISR for reading light data. It is called when ADC conversion is done.
 */
static void LightSensorISR()
{

// Read the ADC data

ADCSequenceDataGet(ADC0_BASE, 1 /* sequencer */,&light_sensor.raw_data /* pointer to data buffer */);

// Set the flag for new data ready

light_sensor.new_data_ready = true;

// IMPORTANT: Clear the interrupt flag

ADCIntClear(ADC0_BASE, 1 /* sequencer */);
}
