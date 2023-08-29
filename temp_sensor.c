/*
 * temp_sensor.c: Starter code for ECE 266 Lab 6, spring 2023
 *
 * Temperature sensor module
 *
 * Created by Zhao Zhang
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/adc.h>
#include "temp_sensor.h"

/*
 * Global declarations
 */

// Temperature sensor reading states
typedef struct {
    uint32_t raw_data;
    bool new_data_ready;
} TempSensorState;

static TempSensorState temp_sensor = {0, false};

// Pre-declare the ISR function
static void TempSensorISR();

/*
 * Initialize ADC to use Tiva C's internal temperature sensor
 *
 * Resources: ADC0, sequence #0, special channel TS (temperature sensor)
 * Configurations: processor trigger, interrupt enabled, use step 0 only
 */
void TsInit()
{
    // Enable the ADC0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Configure ADC0's sequencer #0
    ADCSequenceConfigure(ADC0_BASE, 0 /* sequencer */, ADC_TRIGGER_PROCESSOR, 0 /* priority */);

    // Configure step 0 of sequencer 0 to use the temperature sensor, with
    // interrupt enable, as the end of the sequence
    ADCSequenceStepConfigure(ADC0_BASE, 0 /* sequencer */, 0 /* step */,
                             ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);

    // Enable ADC0, sequencer 0
    ADCSequenceEnable(ADC0_BASE, 0 /* sequencer */);

    // Configure the interrupt system for temperature sensor
    ADCIntRegister(ADC0_BASE, 0 /* sequencer */, TempSensorISR);
    ADCIntEnable(ADC0_BASE, 0 /* sequencer */);
}

/*
 * Trigger temperature reading
 */
void TsTriggerSampling()
{
    // Trigger the sampling
    ADCProcessorTrigger(ADC0_BASE, 0);
}

/*
 * Check if any new data is ready
 */
bool TsNewDataReady()
{
    return temp_sensor.new_data_ready;
}

/*
 * Read the current input from the temperature sensor
 */
uint32_t TsReadRaw()
{
    // Reset new data ready flag and return the reading
    temp_sensor.new_data_ready = false;
    return temp_sensor.raw_data;
}

/*
 * ISR for reading temperature data. It is called when ADC conversion is done.
 */
static void TempSensorISR()
{
    // Read the ADC data
    ADCSequenceDataGet(ADC0_BASE, 0 /* sequencer */,
                       &temp_sensor.raw_data /* pointer to data buffer */);

    // Set the flag for new data ready
    temp_sensor.new_data_ready = true;

    // IMPORTANT: Clear the interrupt flag
    ADCIntClear(ADC0_BASE, 0 /* sequencer */);
}


