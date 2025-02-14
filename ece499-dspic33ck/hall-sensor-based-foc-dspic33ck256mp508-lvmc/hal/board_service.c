/*******************************************************************************
* Copyright (c) 2017 released Microchip Technology Inc.  All rights reserved.
*
* SOFTWARE LICENSE AGREEMENT:
* 
* Microchip Technology Incorporated ("Microchip") retains all ownership and
* intellectual property rights in the code accompanying this message and in all
* derivatives hereto.  You may use this code, and any derivatives created by
* any person or entity by or on your behalf, exclusively with Microchip's
* proprietary products.  Your acceptance and/or use of this code constitutes
* agreement to the terms and conditions of this notice.
*
* CODE ACCOMPANYING THIS MESSAGE IS SUPPLIED BY MICROCHIP "AS IS".  NO
* WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE APPLY TO THIS CODE, ITS INTERACTION WITH MICROCHIP'S
* PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
*
* YOU ACKNOWLEDGE AND AGREE THAT, IN NO EVENT, SHALL MICROCHIP BE LIABLE,
* WHETHER IN CONTRACT, WARRANTY, TORT (INCLUDING NEGLIGENCE OR BREACH OF
* STATUTORY DUTY),STRICT LIABILITY, INDEMNITY, CONTRIBUTION, OR OTHERWISE,
* FOR ANY INDIRECT, SPECIAL,PUNITIVE, EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL
* LOSS, DAMAGE, FOR COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE CODE,
* HOWSOEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR
* THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWABLE BY LAW,
* MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS CODE,
* SHALL NOT EXCEED THE PRICE YOU PAID DIRECTLY TO MICROCHIP SPECIFICALLY TO
* HAVE THIS CODE DEVELOPED.
*
* You agree that you are solely responsible for testing the code and
* determining its suitability.  Microchip has no obligation to modify, test,
* certify, or support the code.
*
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "port_config.h"
#include "board_service.h"
#include "userparms.h"
#include "adc.h"
#include "pwm.h"
#include "cmp.h"
#include "timer.h"

BUTTON_T button1;
BUTTON_T button2;
BUTTON_T button3;

uint16_t boardServiceISRCounter = 0;

void DisablePWMOutputsInverterA(void);
void EnablePWMOutputsInverterAMotor(void);
void EnablePWMOutputsInverterAGenerator(void);
void ClearPWMPCIFaultInverterA(void);
void BoardServiceInit(void);
void BoardServiceStepIsr(void);
void BoardService(void);
bool IsPressed_Button1(void);
bool IsPressed_Button2(void);
bool IsPressed_Button3(void);
void PWMDutyCycleSetDualEdge(MC_DUTYCYCLEOUT_T *,MC_DUTYCYCLEOUT_T *);
void PWMDutyCycleSet(MC_DUTYCYCLEOUT_T *);
void pwmDutyCycleLimitCheck(MC_DUTYCYCLEOUT_T *,uint16_t,uint16_t);

static void ButtonGroupInitialize(void);
static void ButtonScan(BUTTON_T * ,bool);

bool IsPressed_Button1(void)
{
    if (button1.status)
    {
        button1.status = false;
        return true;
    }
    else
    {
        return false;
    }
}

bool IsPressed_Button2(void)
{
    if (button2.status)
    {
        button2.status = false;
        return true;
    }
    else
    {
        return false;
    }
}

bool IsPressed_Button3(void)
{
    if (button3.status)
    {
        button3.status = false;
        return true;
    }
    else
    {
        return false;
    }
    
}

void BoardServiceStepIsr(void)
{
    if (boardServiceISRCounter <  BOARD_SERVICE_TICK_COUNT)
    {
        boardServiceISRCounter += 1;
    }
}
void BoardService(void)
{
    if (boardServiceISRCounter ==  BOARD_SERVICE_TICK_COUNT)
    {
        /* Button scanning loop for Button 1*/
        ButtonScan(&button1,SW1);

        /* Button scanning loop for SW2*/
        ButtonScan(&button2,SW2);
        
        /* Button scanning loop for SW3*/
        ButtonScan(&button3,SW3);
        
        
        boardServiceISRCounter = 0;
    }
}
void BoardServiceInit(void)
{
    ButtonGroupInitialize();
    boardServiceISRCounter = BOARD_SERVICE_TICK_COUNT;
}

void ButtonScan(BUTTON_T *pButton,bool button) 
{
   // pButton->status=false;
    if (button == true) 
    {
        if (pButton->debounceCount < BUTTON_DEBOUNCE_COUNT) 
        {
            pButton->debounceCount--;
            pButton->state = BUTTON_DEBOUNCE;
        }
    } 
    else 
    {
        if (pButton->debounceCount < BUTTON_DEBOUNCE_COUNT) 
        {
            pButton->state = BUTTON_NOT_PRESSED;
        } 
        else 
        {
            pButton->state = BUTTON_PRESSED;
            pButton->status = true;
        }
        pButton->debounceCount = 0;
    }
}
void ButtonGroupInitialize(void)
{
    button1.state = BUTTON_NOT_PRESSED;
    button1.debounceCount = 0;
    button1.state = false;

    button2.state = BUTTON_NOT_PRESSED;
    button2.debounceCount = 0;
    button2.state = false;
    
    button3.state = BUTTON_NOT_PRESSED;
    button3.debounceCount = 0;
    button3.state = false;

}

uint16_t HAL_HallValueRead(void) 
{
    uint16_t buffer;
    uint16_t hallValue;

    buffer = PORTE;
    buffer = buffer >> 8;
    hallValue = buffer & 0x0007;
    
    return hallValue;
}
// *****************************************************************************
/* Function:
    Init_Peripherals()

  Summary:
    Routine initializes controller peripherals

  Description:
    Routine to initialize Peripherals used for Inverter Control

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void InitPeripherals(void)
{           
    Init_Timer (); 
    uint16_t cmpReference = 0;
    CMP_Initialize();
    CMP1_ModuleEnable(true);
    cmpReference = (uint16_t)(__builtin_mulss(Q15_OVER_CURRENT_THRESHOLD,2047)>>15);
    cmpReference = cmpReference + 2048; 
    CMP1_ReferenceSet(cmpReference);
    InitializeADCs();
    
    InitPWMGenerators();
    
    /* Make sure ADC does not generate interrupt while initializing parameters*/
    DisableADCInterrupt();
}
/**
 * Disable the PWM channels assigned for Inverter #A by overriding them to low state.
 * @exampleS
 * <code>
 * DisablePWMOutputsInverterA();
 * </code>
 */
void DisablePWMOutputsInverterA(void)
{
    /** Set Override Data on all PWM outputs */
    // 0b00 = State for PWM4H,L, if Override is Enabled
    PG4IOCONLbits.OVRDAT = 0;
    // 0b00 = State for PWM2H,L, if Override is Enabled
    PG2IOCONLbits.OVRDAT = 0; 
    // 0b00 = State for PWM1H,L, if Override is Enabled
    PG1IOCONLbits.OVRDAT = 0;  
    
    // 1 = OVRDAT<1> provides data for output on PWM4H
    PG4IOCONLbits.OVRENH = 1; 
    // 1 = OVRDAT<0> provides data for output on PWM4L
    PG4IOCONLbits.OVRENL = 1; 
    
    // 1 = OVRDAT<1> provides data for output on PWM2H
    PG2IOCONLbits.OVRENH = 1;
    // 1 = OVRDAT<0> provides data for output on PWM2L
    PG2IOCONLbits.OVRENL = 1; 
    
    // 1 = OVRDAT<1> provides data for output on PWM1H
    PG1IOCONLbits.OVRENH = 1;  
    // 1 = OVRDAT<0> provides data for output on PWM1L
    PG1IOCONLbits.OVRENL = 1;     
}

/**
 * Enable the PWM channels assigned for Inverter #A by removing Override.
 * @example
 * <code>
 * EnablePWMOutputsInverterAMotor();
 * </code>
 */
void EnablePWMOutputsInverterAMotor(void)
{    
    // 0 = PWM Generator provides data for the PWM4H pin
    PG4IOCONLbits.OVRENH = 0; 
    // 0 = PWM Generator provides data for the PWM4L pin
    PG4IOCONLbits.OVRENL = 0; 
    
    // 0 = PWM Generator provides data for the PWM2H pin
    PG2IOCONLbits.OVRENH = 0;
    // 0 = PWM Generator provides data for the PWM2L pin
    PG2IOCONLbits.OVRENL = 0; 
    
    // 0 = PWM Generator provides data for the PWM1H pin
    PG1IOCONLbits.OVRENH = 0;  
    // 0 = PWM Generator provides data for the PWM1L pin
    PG1IOCONLbits.OVRENL = 0;     
}

void EnablePWMOutputsInverterAGenerator(void)
{    
    
        /** Set Override Data on all PWM outputs, in other words, FETS are off
         * When disabled */
    // 0b00 = State for PWM4H,L, if Override is Enabled
    PG4IOCONLbits.OVRDAT = 0;
    // 0b00 = State for PWM2H,L, if Override is Enabled
    PG2IOCONLbits.OVRDAT = 0; 
    // 0b00 = State for PWM1H,L, if Override is Enabled
    PG1IOCONLbits.OVRDAT = 0; 
    
    // 1 = FET is off for generator mode
    PG4IOCONLbits.OVRENH = 1; 
    // 0 = PWM Generator provides data for the PWM4L pin
    PG4IOCONLbits.OVRENL = 0; 
    
    // 1 = FET is off for generator mode
    PG2IOCONLbits.OVRENH = 1;
    // 0 = PWM Generator provides data for the PWM2L pin
    PG2IOCONLbits.OVRENL = 0; 
    
    // 1 = FET is off for generator mode
    PG1IOCONLbits.OVRENH = 1;  
    // 0 = PWM Generator provides data for the PWM1L pin
    PG1IOCONLbits.OVRENL = 0;     
}

void ClearPWMPCIFaultInverterA(void)
{
    
    PG1FPCILbits.SWTERM = 1;
    PG2FPCILbits.SWTERM = 1;
    PG4FPCILbits.SWTERM = 1;
}

void PWMDutyCycleSet(MC_DUTYCYCLEOUT_T *pPwmDutycycle)
{
    pwmDutyCycleLimitCheck(pPwmDutycycle,(DDEADTIME>>1),(LOOPTIME_TCY - (DDEADTIME>>1)));  
    INVERTERA_PWM_PDC3 = pPwmDutycycle->dutycycle3;
    INVERTERA_PWM_PDC2 = pPwmDutycycle->dutycycle2;
    INVERTERA_PWM_PDC1 = pPwmDutycycle->dutycycle1;
}
void PWMDutyCycleSetDualEdge(MC_DUTYCYCLEOUT_T *pPwmDutycycle1,MC_DUTYCYCLEOUT_T *pPwmDutycycle2)
{
    pwmDutyCycleLimitCheck(pPwmDutycycle1,(DDEADTIME>>1),(LOOPTIME_TCY - (DDEADTIME>>1)));
    
    INVERTERA_PWM_PHASE3 = pPwmDutycycle1->dutycycle3 + (DDEADTIME>>1);
    INVERTERA_PWM_PHASE2 = pPwmDutycycle1->dutycycle2 + (DDEADTIME>>1);
    INVERTERA_PWM_PHASE1 = pPwmDutycycle1->dutycycle1 + (DDEADTIME>>1);
    
    pwmDutyCycleLimitCheck(pPwmDutycycle2,(DDEADTIME>>1),(LOOPTIME_TCY - (DDEADTIME>>1)));
    
    INVERTERA_PWM_PDC3 = pPwmDutycycle2->dutycycle3 - (DDEADTIME>>1);
    INVERTERA_PWM_PDC2 = pPwmDutycycle2->dutycycle2 - (DDEADTIME>>1);
    INVERTERA_PWM_PDC1 = pPwmDutycycle2->dutycycle1 - (DDEADTIME>>1);
}
void pwmDutyCycleLimitCheck (MC_DUTYCYCLEOUT_T *pPwmDutycycle,uint16_t min,uint16_t max)
{
    if (pPwmDutycycle->dutycycle1 < min)
    {
        pPwmDutycycle->dutycycle1 = min;
    }
    else if (pPwmDutycycle->dutycycle1 > max)
    {
        pPwmDutycycle->dutycycle1 = max;
    }
    
    if (pPwmDutycycle->dutycycle2 < min)
    {
        pPwmDutycycle->dutycycle2 = min;
    }
    else if (pPwmDutycycle->dutycycle2 > max)
    {
        pPwmDutycycle->dutycycle2 = max;
    }
    
    if (pPwmDutycycle->dutycycle3 < min)
    {
        pPwmDutycycle->dutycycle3 = min;
    }
    else if (pPwmDutycycle->dutycycle3 > max)
    {
        pPwmDutycycle->dutycycle3 = max;
    }
}