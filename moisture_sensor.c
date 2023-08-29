#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/adc.h>
#include "moisture_sensor.h"

/*
 * Global declarations
 */

// Moisture sensor reading states
typedef struct {
    uint32_t raw_data;
    bool new_data_ready;
} MoistureSensorState;

static MoistureSensorState moisture_sensor = {0, false};

// Pre-declare the ISR function
static void MoistureSensorISR();

/*
 * Initialize ADC to use moisture sensor
 *
 * Resources: ADC0, sequence #1, special channel 10 (moisture sensor)
 * Configurations: processor trigger, interrupt enabled, use step 0 only
 */
void MsInit()
{
    // Enable the ADC0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Configure ADC0's sequencer #1
    ADCSequenceConfigure(ADC0_BASE, 2 /* sequencer */, ADC_TRIGGER_PROCESSOR, 0 /* priority */);

    // Configure step 0 of sequencer 1 to use the moisture sensor, with
    // interrupt enable, as the end of the sequence
    ADCSequenceStepConfigure(ADC0_BASE, 2 /* sequencer */, 0 /* step */,
                             ADC_CTL_CH5 | ADC_CTL_IE | ADC_CTL_END);

    // Enable ADC0, sequencer 1
    ADCSequenceEnable(ADC0_BASE, 2 /* sequencer */);

    // Configure the interrupt system for moisture sensor
    ADCIntRegister(ADC0_BASE, 2 /* sequencer */, MoistureSensorISR);

    IntPrioritySet(INT_ADC0SS1, 0);

    ADCIntEnable(ADC0_BASE, 2 /* sequencer */);
}

/*
 * Trigger moisture reading
 */
void MsTriggerSampling()
{
    moisture_sensor.new_data_ready = false;

    ADCProcessorTrigger(ADC0_BASE, 2);
}

/*
 * Check if any new data is ready
 */
bool MsNewDataReady()
{
    return moisture_sensor.new_data_ready;
}

/*
 * Read the current input from the moisture sensor
 */
uint32_t MsReadRaw()
{
    // Reset new data ready flag and return the reading
    moisture_sensor.new_data_ready = false;
    return moisture_sensor.raw_data;
}

/*
 * ISR for reading moisture data. It is called when ADC conversion is done.
 */
static void MoistureSensorISR()
{
    // Read the ADC data
    ADCSequenceDataGet(ADC0_BASE, 2 /* sequencer */,
                       &moisture_sensor.raw_data /* pointer to data buffer */);

    // Set the flag for new data ready
    moisture_sensor.new_data_ready = true;

    // IMPORTANT: Clear the interrupt flag
    ADCIntClear(ADC0_BASE, 2 /* sequencer */);
}
