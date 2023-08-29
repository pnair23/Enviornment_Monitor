; light_sensor_asm.asm
; Lab 6, ECE 266, spring 2023
;
; Assembly code for the temperature sensor
;
; Created by Zhao Zhang

        ; To include names declared in C
        .cdecls "stdint.h","stdbool.h","inc/hw_memmap.h","inc/hw_ints.h","driverlib/pin_map.h","driverlib/sysctl.h","driverlib/interrupt.h","driverlib/adc.h","light_sensor.h"

        .data

;******************************************************************************
; Static/global variables
;******************************************************************************

; Temperature sensor reading states
;typedef struct {
;    uint32_t raw_data;
;    bool new_data_ready;
;} LightSensorState;
light_sensor
        .word   0
        .byte   false

        .text

;******************************************************************************
; Declaration of fields
;******************************************************************************
_ADC0_PERIPH        .field     SYSCTL_PERIPH_ADC0
_ADC0_BASE          .field     ADC0_BASE
_light_sensor       .field     light_sensor
_LightSensorISR     .field     LightSensorISR

;******************************************************************************
; voidTsInit()
;
; Initialize ADC to use Tiva C's internal temperature sensor
;
; Resources: ADC0, sequence #0, special channel TS (temperature sensor)
; Configurations: processor trigger, interrupt enabled, use step 0 only
;******************************************************************************
LsInit  .asmfunc
        push {lr}

        ; Enable the ADC0 peripheral
        ; call SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

        LDR     r0, _ADC0_PERIPH

        BL      SysCtlPeripheralEnable

        ; Configure ADC0's sequencer #1
        ; call ADCSequenceConfigure(ADC0_BASE, 1 /* sequencer */, ADC_TRIGGER_PROCESSOR, 0 /* priority */)
        LDR     r0, _ADC0_BASE

        MOV	    r1, #1

        MOV	    r2, #0

        BL      ADCSequenceConfigure

       ; Configure step 0 of sequencer 0 to use the temperature sensor, with
       ; interrupt enable, as the end of the sequence
        ; call ADCSequenceStepConfigure(ADC0_BASE, 1 /* sequencer */, 0 /* step */,
        ;                         ADC_CTL_CH7 | ADC_CTL_IE | ADC_CTL_END)

        LDR		r0, _ADC0_BASE

        MOV		r1, #1

        MOV		r2, #0

        MOV		r3, #(ADC_CTL_CH7 | ADC_CTL_IE | ADC_CTL_END)

        BL 		ADCSequenceStepConfigure

        ; Enable ADC0, sequencer 0
        ; call ADCSequenceEnable(ADC0_BASE, 1 /* sequencer */);

        LDR     r0, _ADC0_BASE

        MOV     r1, #1

        BL      ADCSequenceEnable

        ; Configure the interrupt system for temperature sensor
        ; call ADCIntRegister(ADC0_BASE, 1 /* sequencer */, LightSensorISR)
        ; call ADCIntEnable(ADC0_BASE, 1 /* sequencer */)

        LDR     r0, _ADC0_BASE

        MOV     r1, #1

        LDR     r2, _LightSensorISR

        BL      ADCIntRegister

        LDR     r0, _ADC0_BASE

        MOV     r1, #1

        BL      ADCIntEnable

	    pop     {pc}
	    .endasmfunc



;******************************************************************************
; void TsTriggerSampling()
;
; Trigger temperature reading
;******************************************************************************
LsTriggerSampling
        .asmfunc
        push    {lr}

        ; Set new_data_ready to false

	    LDR     r0, _light_sensor

	    MOV     r1, #0

	    STRB    r1, [r0, #4]

	    ; Trigger the sampling

	    LDR     r0, _ADC0_BASE

	    MOV     r1, #1

	    BL      ADCProcessorTrigger

        pop     {pc}
        .endasmfunc

;******************************************************************************
; TsNewDataReady
;
; Check if any new data is ready
;******************************************************************************
LsNewDataReady
        .asmfunc

	    ; Load the address of light_sensor into r0
	    LDR    r0, _light_sensor

	    ; Load the value of new_data_ready into r0
	    LDRB   r0, [r0, #4]

        BX      lr
        .endasmfunc

;******************************************************************************
; Read the current input from the temperature sensor
;******************************************************************************
LsReadRaw
        .asmfunc

	    ; Reset new data ready flag
	    LDR     r0, _light_sensor

	    MOV     r1, #0

	    STRB    r1, [r0, #4] ; write 0 to new_data_ready field

	    ; Return the reading

	    LDR     r0, _light_sensor

	    LDR     r0, [r0, #0] ; read raw_data field

        BX      lr
        .endasmfunc

;******************************************************************************
; ISR for reading temperature data. It is called when ADC conversion is done.
;******************************************************************************
LightSensorISR
        .asmfunc
        push    {lr}


        LDR  	r0, _ADC0_BASE        ; loads the address of _ADC0_BASE into register 0

        MOV		r1, #1  			  ; moves 1 into register 1 to set the sequence number

        LDR     r2, _light_sensor     ; loads the address of _light_sensor into register 2

        BL		ADCSequenceDataGet    ; calls to ADCSequenceDataGet


        LDR		r0, _light_sensor     ; loads the address of _light_sensor into register 0

        MOV		r1, #1				  ; moves 1 into register 1 to set it to true and the flag of the struct var. to true

        STRB	r1, [r0, #4]	      ; stores a byte with offset 4


        LDR		r0, _ADC0_BASE        ; loads the address of _ADC0_BASE into register 0

        MOV     r1, #1				  ; moves 1 into register 1 to set the sequence number

        BL      ADCIntClear			  ; calls to ADCIntClear


        pop     {pc}
        .endasmfunc
