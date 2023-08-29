#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "launchpad.h"
#include "seg7.h"
#include "temp_sensor.h"
#include "light_sensor.h"
#include "moisture_sensor.h"
#include "buzzer.h"


#define RED_LED   GPIO_PIN_1
#define GREEN_LED GPIO_PIN_3

typedef enum { SUMMER, FALL, NONE } Season;
Season current_season = NONE;

bool first_run = true;

uint32_t g_temp_F = 0;
uint32_t g_light_reading = 0;
uint32_t g_moist_reading = 0;

// The state of the 7-segment display. Initially it shows "00:00" with the colon turned on.
Seg7Display seg7 = { {0, 0, 0, 0 }, false /* colon on */};
// Function to play a tone using the buzzer
void BzTone(uint32_t frequency, uint32_t duration)
{

    // Calculate the delay in microseconds
    uint32_t delay = 1000000 / (frequency * 2); // Calculate the delay in microseconds

    // Calculate the number of iterations needed
    uint32_t count = (duration * 1000) / (delay * 2); // Calculate the number of iterations needed

    int i;
    // Loop through the iterations
    for (i = 0; i < count; i++)
    {
        BuzzerOn(); // Turn on the buzzer
        SysCtlDelay((SysCtlClockGet() / 3000000) * delay); // Delay for half of the period

        BuzzerOff(); // Turn off the buzzer
        SysCtlDelay((SysCtlClockGet() / 3000000) * delay); // Delay for half of the period
    }
}

void UpdateLedState(uint32_t temp_F, uint32_t light_reading, uint32_t moist_reading)
{
    // If the temperature, light, and moisture readings indicate summer conditions
    if (temp_F > 670 && light_reading > 800 && moist_reading > 300)
    {
        // Turn off the RED LED and turn on the GREEN LED
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, 0);
        GPIOPinWrite(GPIO_PORTF_BASE, GREEN_LED, GREEN_LED);

        // If the current season is not summer, play a sound and update the current season to summer
        if (current_season != SUMMER) {
            if (!first_run) {
                uprintf("\n%s\n\n\r", "ITS SUMMER TIME BABY\n");
                BzTone(3000, 300); // Play a high pitch sound for 100 ms
            }
            current_season = SUMMER;
        }
    }
    // If the temperature, light, and moisture readings indicate fall conditions
    else if (temp_F <= 670 && light_reading <= 800 && moist_reading <= 300)
    {
        // Turn off the GREEN LED and turn on the RED LED
        GPIOPinWrite(GPIO_PORTF_BASE, GREEN_LED, 0);
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, RED_LED);

        // If the current season is not fall, play a sound and update the current season to fall
        if (current_season != FALL) {
            if (!first_run) {
                uprintf("%s\n\r","ITS FALL TIME\n");
                BzTone(500, 300); // Play a low pitch sound for 100 ms
            }
            current_season = FALL;
        }
    }
    // If the readings don't indicate either summer or fall conditions
    else
    {
        // Turn off both the RED and GREEN LEDs
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED | GREEN_LED, 0);
        current_season = NONE;
    }
    // Set first_run to false after the first execution of the function
    first_run = false;
}
/*
 * Trigger the temperature sensor reading periodically
 */
void MoistSensorSampling(uint32_t time)
{
    // Trigger ADC sampling for the motion sensor
    MsTriggerSampling();

    // Schedule the next callback in 4000 milliseconds
    ScheduleCallback(MoistSensorSampling, time + 4000);
}

void TempSensorSampling(uint32_t time)
{
    // Trigger ADC sampling for the temperature sensor
    TsTriggerSampling();

    // Schedule the next callback in 4000 milliseconds
    ScheduleCallback(TempSensorSampling, time + 4000);
}


/*
 * Trigger the light sensor reading periodically
 */
void LightSensorSampling(uint32_t time)
{
    // Trigger ADC sampling for the light sensor
    LsTriggerSampling();

    // Schedule the next callback in 4000 milliseconds
    ScheduleCallback(LightSensorSampling, time + 4000);
}

/*
 * Check temperature sensor reading, print if ready
 */




void CheckNewTempReading()
{
    // If there is no new reading, return immediately
    if (!TsNewDataReady())
        return;

    // Get the current reading and convert it to temperature in Celsius
    // See Tiva C datasheet (ver. DS-TM4C123GH6PM-15842.2741), page 813
    uint32_t raw_data = TsReadRaw();

    float temp_C = 147.5 - (75 * 3.3 * raw_data / 4096);

    // Print on terminal. Note: TivaWare printf() has issue with "%f"
   // uprintf("Temperature Sensor Reading: %u.%u C\r\n", (uint32_t)temp_C, (uint32_t)(temp_C*10)%10);


    uint32_t temp_F = ((temp_C * 1.8) + 32) * 10;                                                           //converts from celsius to fahrenheit

    uprintf("Temperature Sensor Reading: %u.%u F\r\n", (uint32_t)temp_F / 10, (uint32_t)(temp_F)%10);       //prints out the  fahrenheit value to terminal

    seg7.digit[0] = 10;                                         //value of first position is 10 to display nothing in 1st position

    seg7.digit[1] = (temp_F % 10);                              // its the value for 2nd position

    seg7.digit[2] = (temp_F / 10) % 10;                         //its the value for 3rd position

    seg7.digit[3] = (temp_F / 100) % 10;                        //its the value for 4th position

    seg7.colon_on = true;                                       // this will turn the colon on
    Seg7Update(&seg7);                                          // this will update the digital display

    g_temp_F = temp_F;

}


void CheckNewLightReading()
{
    // If there is no new reading, return immediately
    if (!LsNewDataReady())
        return;
    uint32_t raw_data = LsReadRaw();

    // Print light sensor reading on the terminal
    uprintf("Light Sensor Reading: %u\r\n", raw_data);

    seg7.digit[0] = raw_data % 10;                          //set value for 1st position
    seg7.digit[1] = (raw_data / 10) % 10;                   //set value for 2nd position
    seg7.digit[2] = (raw_data / 100) % 10;                  //set value for 3rd position
    seg7.digit[3] = (raw_data / 1000) % 10;                 //set value for 4th position

    seg7.colon_on = false;                                  //turn colon off
    Seg7Update(&seg7);                                      //update seg7 (digital display)

    g_light_reading = raw_data;
}



void CheckNewMoistReading()
{
    // If there is no new reading, return immediately
    if (!MsNewDataReady())
        return;

    // Get the current reading
    uint32_t raw_data = MsReadRaw();

    // Print on terminal
    uprintf("Moisture Sensor Reading: %u \r\n", (uint32_t)raw_data);

    seg7.digit[0] = raw_data % 10;                          // set value for 1st position
    seg7.digit[1] = (raw_data / 10) % 10;                   // set value for 2nd position
    seg7.digit[2] = (raw_data / 100) % 10;                  // set value for 3rd position
    seg7.digit[3] = (raw_data / 1000) % 10;                 // set value for 4th position

    seg7.colon_on = false;                                  // turn colon off
    Seg7Update(&seg7);                                      // update seg7 (digital display)

    g_moist_reading = raw_data;

}

int main(void)

{
    LpInit();
    Seg7Init();
    TsInit();
   // LsInit();
    MsInit(); // Initialize the moisture sensor
    LsInit();
    BzInit(); // Initialize the buzzer

    ScheduleCallback(LightSensorSampling, 3000);
    uprintf("%s\n\r", "Final Project: Weather CHECK");
    ScheduleCallback(TempSensorSampling, 1000);
    ScheduleCallback(MoistSensorSampling, 4000);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED | GREEN_LED);


    // Schedule the first callback
    //ScheduleCallback(TempSensorSampling, 1000);
   // ScheduleCallback(LightSensorSampling, 3000);
   // ScheduleCallback(MoistSensorSampling, 5000); // Schedule the moisture sensor callback

    // Loop forever
    while (true)
    {
        // Wait for interrupt to happen
        __asm("    wfi");

        // Check new readings
        CheckNewTempReading();
        CheckNewLightReading();
        CheckNewMoistReading(); // Check new moisture reading

        UpdateLedState(g_temp_F, g_light_reading, g_moist_reading);



        ScheduleExecute();
    }
}

        // Call the callback scheduler
