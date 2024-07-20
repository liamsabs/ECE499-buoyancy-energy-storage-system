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

#define NOMECH
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <libq.h>      
#include "motor_control_noinline.h"

#include "general.h"   
#include "userparms.h"

#include "control.h"
#include "clock.h"
#include "pwm.h"
#include "adc.h"
#include "port_config.h"
#include "delay.h"
#include "board_service.h"
#include "diagnostics.h"
#include "measure.h"
#include "timer.h"
#include "spi1.h"
#include "utilities.h"


volatile UGF_T uGF;




CTRL_PARM_T ctrlParm;
MOTOR_STARTUP_DATA_T motorStartUpData;
MCAPP_DATA_T mcappData;

volatile int16_t thetaElectrical = 0,thetaElectricalOpenLoop = 0;
uint16_t pwmPeriod;

MC_ALPHABETA_T valphabeta,ialphabeta;
MC_SINCOS_T sincosTheta;
MC_DQ_T vdq,idq;
MC_DUTYCYCLEOUT_T pwmDutycycle;
MC_ABC_T   vabc,iabc;

MC_PIPARMIN_T piInputIq;
MC_PIPARMOUT_T piOutputIq;
MC_PIPARMIN_T piInputId;
MC_PIPARMOUT_T piOutputId;
MC_PIPARMIN_T piInputOmega;
MC_PIPARMOUT_T piOutputOmega;

volatile uint16_t adcDataBuffer;
MCAPP_MEASURE_T measureInputs;

/*SPI calling frequency*/
#define SENSE_SAMPLING_PERIOD_SEC 0.5

/* SPI Counter */
uint8_t SPIFlag;
uint32_t SPICounter; 
#define SENSOR_SAMPLE_RATE_REVS (uint32_t)(PWMFREQUENCY_HZ * SENSE_SAMPLING_PERIOD_SEC)

#define SPI_RX_BUFFER_SIZE  1
/* SPI1 Transmit Buffer Size*/
#define SPI_TX_BUFFER_SIZE  5 

volatile uint8_t rxBuffer;
volatile uint8_t txBuffer[SPI_TX_BUFFER_SIZE];
volatile uint8_t txBufferDummy[SPI_TX_BUFFER_SIZE] = {11, 69, 21, 49, 26};
volatile uint8_t txCounter = 0;

System system = {.state = IDLE, .unpausedState = IDLE, .position = 0};

/** Definitions */
/* Fraction of DC link voltage(expressed as a squared amplitude) to set 
 * the limit for current controllers PI Output */
#define MAX_VOLTAGE_VECTOR                      0.98
/* The count corresponding to 60degree angle i.e, 32768/3  */
#define SECTOR_ANGLE    10922    
/*Offset between the zero crossing sector and the Phase A 
 * current zero crossing    */
#define OFFSET_CORRECTION   (-4000)
/*Hall angle correction divisor for correcting angle difference 
 *  between the sectors     */
#define HALL_CORRECTION_DIVISOR 3
/*Number of steps in which the Hall sensor angle correction is done
 i.e,2^HALL_CORRECTION_DIVISOR  */
#define HALL_CORRECTION_STEPS   8 

void InitControlParameters(void);
void DoControl(void);
void CalculateParkAngle(void);
void ResetParmeters(void);
void prepareTxData(void);

// *****************************************************************************
/* Function:
   main()

  Summary:
    main() function

  Description:
    program entry point, calls the system initialization function group 
    containing the buttons polling loop

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */

int main ( void )
{
    SPICounter=0;
    SPIFlag=0;
    InitOscillator();
    SetupGPIOPorts();
    /* Turn on LED2 to indicate the device is programmed */
    LED2 = 1;
    /* Initialize Peripherals */
    InitPeripherals();
    DiagnosticsInit();
    SPI1_Initialize();
    __builtin_enable_interrupts();
    
    BoardServiceInit();
    HAL_MC1HallStateChangeTimerPrescalerSet(TIMER_PRESCALER);
    HAL_MC1HallStateChangeMaxPeriodSet(0xFFFF);
    HAL_MC1HallStateChangeTimerStart();
    CORCONbits.SATA = 0;
    mcappData.PeriodKFilter = Q15(0.001);
    mcappData.period = 0xFFF0;
    while(1)
    {        
        /* Reset parameters used for running motor through Inverter A*/
        ResetParmeters();
        
        while(1)
        {
            /*Check if the SPI flag is set if it is request data*/
            LATEbits.LATE1=1;
           /* if(SPIFlag){
                SPIFlag=0;
                prepareTxData(); //acquire sensor and system state data
                LATEbits.LATE1 = 1; //set PORTE1 high (MicroBus_A_AN)
            }
            * */
            /*Button logic for onboard*/
            if (IsPressed_Button1())
            {

                
                ResetParmeters();
            }

            else if(IsPressed_Button2())
            { 

                ResetParmeters();
                uGF.bits.MotorState = 0b01;
                EnablePWMOutputsInverterAMotor();

                LED1 = 0;

            }
            else if(IsPressed_Button3()){
                ResetParmeters();
                uGF.bits.MotorState = 0b10;
                EnablePWMOutputsInverterAGenerator();

                //LED2 = 1;
           }
            
            
            DiagnosticsStepMain();
            BoardService();
                     
        }

    } // End of Main loop
    // should never get here
    while(1){}
}

// *****************************************************************************
/* Function:
    ResetParmsA()

  Summary:
    This routine resets all the parameters required for Motor through Inv-A

  Description:
    Reinitializes the duty cycle,resets all the counters when restarting motor

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void ResetParmeters(void)
{
    /* Make sure ADC does not generate interrupt while initializing parameters*/
	DisableADCInterrupt(); 
    HAL_MC1HallStateChangeDetectionEnable();
#ifdef SINGLE_SHUNT
    /* Initialize Single Shunt Related parameters */
    SingleShunt_InitializeParameters(&singleShuntParam);
    INVERTERA_PWM_TRIGA = ADC_SAMPLING_POINT;
    INVERTERA_PWM_TRIGB = LOOPTIME_TCY>>1;
    INVERTERA_PWM_TRIGC = LOOPTIME_TCY-1;
    INVERTERA_PWM_PHASE3 = MIN_DUTY;
    INVERTERA_PWM_PHASE2 = MIN_DUTY;
    INVERTERA_PWM_PHASE1 = MIN_DUTY;
#else
    INVERTERA_PWM_TRIGA = ADC_SAMPLING_POINT;
#endif
    /* Re initialize the duty cycle to minimum value */
    INVERTERA_PWM_PDC3 = MIN_DUTY;
    INVERTERA_PWM_PDC2 = MIN_DUTY;
    INVERTERA_PWM_PDC1 = MIN_DUTY;
    
    DisablePWMOutputsInverterA();
    
    /* Stop the motor   */
    uGF.bits.MotorState = 0b11;        
    /* Set the reference speed value to 0 */
    ctrlParm.qVelRef = 0;
    /* Restart in open loop */
    uGF.bits.OpenLoop = 1;
    /* Change mode */
    uGF.bits.ChangeMode = 1;
    
    /* Initialize PI control parameters */
    InitControlParameters();
    /* Initialize measurement parameters */
    MCAPP_MeasureCurrentInit(&measureInputs);

    MCAPP_MeasureAvgInit(&measureInputs.MOSFETTemperature,
            MOSFET_TEMP_AVG_FILTER_SCALE);
    /* Enable ADC interrupt and begin main loop timing */
    ClearADCIF();
    adcDataBuffer = ClearADCIF_ReadADCBUF();
    EnableADCInterrupt();
}
// *****************************************************************************
/* Function:
    DoControl()

  Summary:
    Executes one PI iteration for each of the three loops Id,Iq,Speed for motoring mode

  Description:
    This routine executes one PI iteration for each of the three loops
    Id,Iq,Speed

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void DoControl( void )
{
    /* Temporary variables for sqrt calculation of q reference */
    volatile int16_t temp_qref_pow_q15;
    
    if  (uGF.bits.OpenLoop)
    {
        /* OPENLOOP:  force rotating angle,Vd and Vq */
        if  (uGF.bits.ChangeMode)
        {
            /* Just changed to open loop */
            uGF.bits.ChangeMode = 0;

            /* Synchronize angles */
            /* VqRef & VdRef not used */
            ctrlParm.qVqRef = 0;
            ctrlParm.qVdRef = 0;

            /* Reinitialize variables for initial speed ramp */
            motorStartUpData.startupLock = 0;
            motorStartUpData.startupRamp = 0;
        }

        /* PI control for D */
        piInputId.inMeasure = idq.d;
        piInputId.inReference  = ctrlParm.qVdRef;
        MC_ControllerPIUpdate_Assembly(piInputId.inReference,
                                       piInputId.inMeasure,
                                       &piInputId.piState,
                                       &piOutputId.out);
        vdq.d = piOutputId.out;
         /* Dynamic d-q adjustment
         with d component priority 
         vq=sqrt (vs^2 - vd^2) 
        limit vq maximum to the one resulting from the calculation above */
        temp_qref_pow_q15 = (int16_t)(__builtin_mulss(piOutputId.out ,
                                                      piOutputId.out) >> 15);
        temp_qref_pow_q15 = Q15(MAX_VOLTAGE_VECTOR) - temp_qref_pow_q15;
        piInputIq.piState.outMax = _Q15sqrt (temp_qref_pow_q15);
        piInputIq.piState.outMin = - piInputIq.piState.outMax;    
        /* PI control for Q */
        /* Speed reference */
        if(uGF.bits.MotorState==0b01){
            ctrlParm.qVelRef = Q_CURRENT_REF_OPENLOOP;
        }
        else if(uGF.bits.MotorState==0b10){
           ctrlParm.targetSpeed = -((__builtin_mulss(measureInputs.potValue,
                    NOMINALSPEED_ELECTR-ENDSPEED_ELECTR)>>15) +
                    ENDSPEED_ELECTR);
            #ifdef NOMECH
            ctrlParm.targetSpeed=0;
            #endif
        }
        
        /* q current reference is equal to the velocity reference 
         while d current reference is equal to 0
        for maximum startup torque, set the q current to maximum acceptable 
        value represents the maximum peak value */
        ctrlParm.qVqRef = ctrlParm.qVelRef;
        piInputIq.inMeasure = idq.q;
        piInputIq.inReference = ctrlParm.qVqRef;
        MC_ControllerPIUpdate_Assembly(piInputIq.inReference,
                                       piInputIq.inMeasure,
                                       &piInputIq.piState,
                                       &piOutputIq.out);
        vdq.q = piOutputIq.out;

    }
    else
    /* Closed Loop Vector Control */
    {
        /* Potentiometer value is scaled between ENDSPEED_ELECTR 
        * and NOMINALSPEED_ELECTR to set the speed reference*/
        
        //Maybe will remove for our speed, replace potValue with our actual speed
        if(uGF.bits.MotorState==0b01){
            ctrlParm.targetSpeed = (__builtin_mulss(measureInputs.potValue,
                    NOMINALSPEED_ELECTR-ENDSPEED_ELECTR)>>15) +
                    ENDSPEED_ELECTR; 
        }
        else if(uGF.bits.MotorState==0b10){
            ctrlParm.targetSpeed = -((__builtin_mulss(measureInputs.potValue,
                    NOMINALSPEED_ELECTR-ENDSPEED_ELECTR)>>15) +
                    ENDSPEED_ELECTR); 
            #ifdef NOMECH
                ctrlParm.targetSpeed = 0;
            #endif
        }
        
        //Only execute speed control ever SPEEDREFRAMP_COUNT Itterations
        if  (ctrlParm.speedRampCount < SPEEDREFRAMP_COUNT)
        {
           ctrlParm.speedRampCount++; 
        }
        else
        {
            /* Ramp generator to limit the change of the speed reference
              the rate of change is defined by CtrlParm.qRefRamp */
            ctrlParm.qDiff = ctrlParm.qVelRef - ctrlParm.targetSpeed;
            /* Speed Ref Ramp */
            if (ctrlParm.qDiff < 0)
            {
                /* Set this cycle reference as the sum of
                previously calculated one plus the reference ramp value */
                ctrlParm.qVelRef = ctrlParm.qVelRef+ctrlParm.qRefRamp;
            }
            else
            {
                /* Same as above for speed decrease */
                ctrlParm.qVelRef = ctrlParm.qVelRef-ctrlParm.qRefRamp;
            }
            /* If difference less than half of ref ramp, set reference
            directly from the pot */
            if (_Q15abs(ctrlParm.qDiff) < (ctrlParm.qRefRamp << 1))
            {
                ctrlParm.qVelRef = ctrlParm.targetSpeed;
            }
            ctrlParm.speedRampCount = 0;
        }
        if (uGF.bits.ChangeMode)
        {
            /* Just changed from open loop */
            uGF.bits.ChangeMode = 0;
            piInputOmega.piState.integrator = (int32_t)ctrlParm.qVqRef << 13;
            if(uGF.bits.MotorState==0b01){
                ctrlParm.qVelRef = ENDSPEED_ELECTR;
                #
            }
            else if(uGF.bits.MotorState==0b10){
                ctrlParm.qVelRef = -ENDSPEED_ELECTR;   
                #ifdef NOMECH
                ctrlParm.qVelRef = 0;
                #endif
            }
        }

        /* If TORQUE MODE skip the speed controller */
        #ifndef	TORQUE_MODE
            /* Execute the velocity control loop */
            piInputOmega.inMeasure =  mcappData.SpeedHall;
            piInputOmega.inReference = ctrlParm.qVelRef;
            MC_ControllerPIUpdate_Assembly(piInputOmega.inReference,
                                           piInputOmega.inMeasure,
                                           &piInputOmega.piState,
                                           &piOutputOmega.out);
            ctrlParm.qVqRef = piOutputOmega.out;
        #else
            ctrlParm.qVqRef = ctrlParm.qVelRef;
        #endif

        /* PI control for D */
        piInputId.inMeasure = idq.d;
        piInputId.inReference  = ctrlParm.qVdRef;
        MC_ControllerPIUpdate_Assembly(piInputId.inReference,
                                       piInputId.inMeasure,
                                       &piInputId.piState,
                                       &piOutputId.out);
        vdq.d    = piOutputId.out;

        /* Dynamic d-q adjustment
         with d component priority 
         vq=sqrt (vs^2 - vd^2) 
        limit vq maximum to the one resulting from the calculation above */
        temp_qref_pow_q15 = (int16_t)(__builtin_mulss(piOutputId.out ,
                                                      piOutputId.out) >> 15);
        temp_qref_pow_q15 = Q15(MAX_VOLTAGE_VECTOR) - temp_qref_pow_q15;
        piInputIq.piState.outMax = _Q15sqrt (temp_qref_pow_q15);
        piInputIq.piState.outMin = - piInputIq.piState.outMax;
        /* PI control for Q */
        piInputIq.inMeasure  = idq.q;
        piInputIq.inReference  = ctrlParm.qVqRef;
        MC_ControllerPIUpdate_Assembly(piInputIq.inReference,
                                       piInputIq.inMeasure,
                                       &piInputIq.piState,
                                       &piOutputIq.out);
        vdq.q = piOutputIq.out;
    }
}

/******************************************************************************
 * Description: The calculation of six sector to equivalent angle in Q15 format.
 *              Here the angle is varying from -32768 to 32767. The 
 *****************************************************************************/
int16_t sectorToAngle[8] =  // 3, 2, 6, 4, 5, 1
{
    0,
    21845,      // sector-1 =
    -21864,     // sector-2 = (-32768+10922)
    -32768,     // sector-3
    0,          // sector-4
    10922,      // sector-5
    -10924,     //sector-6
    0
};

/******************************************************************************
 * Description: The CND Interrupt is serviced at every hall signal transition. 
 *              This enables to calculate the speed based on the time taken for
 *              360 degree electrical rotation.
 *****************************************************************************/
void __attribute__((interrupt, no_auto_psv)) HAL_MC1HallStateChangeInterrupt ()
{
     LATEbits.LATE1=0;//Disable SPI
    mcappData.timerValue = TMR1;
    TMR1 = 0;
//    if(mcappData.timerValue == 0)
//    {
//        mcappData.period = 1;
//    }
//    else
//    {
        mcappData.period = mcappData.timerValue;
//    }
    mcappData.sector = HAL_HallValueRead();
    // Instead of making abrupt correction to the angle corresponding to hall sector, find the error and make gentle correction 
  
    mcappData.hallThetaError = (sectorToAngle[mcappData.sector] + OFFSET_CORRECTION) - mcappData.HallTheta;
    
    // Find the correction to be done in every step
    // If "hallThetaError" is 2000, and "hallCorrectionFactor" = (2000/8) = 250
    // So the error of 2000 will be corrected in 8 steps, with each step being 250
    mcappData.hallCorrectionFactor = mcappData.hallThetaError >> HALL_CORRECTION_DIVISOR;
    mcappData.hallCorrectionCounter = HALL_CORRECTION_STEPS;
    
    if(system.state == STORING){
        
        if(++system.position >= STORING_DONE_POS){ //increments then checks if end of storing
            
            system.state = STORED; //set state to stored
            uGF.bits.MotorState=0b11;
            /*TODO: Engage Locking Mechanism*/
            prepareTxData(); //acquire sensor and system state data
            LATEbits.LATE1 = 1; //set PORTE1 high (MicroBus_A_AN)
            
            
        } 
        
    }else if (system.state == GENERATING){
        
        if(--system.position <= GEN_DONE_POS){ //decrements then checks if end of generating
            
            system.state = IDLE; // set state to IDLE
            uGF.bits.MotorState=0b11;
            ResetParmeters(); // stop generating
            prepareTxData(); //acquire sensor and system state data
            LATEbits.LATE1 = 1; //set PORTE1 high (MicroBus_A_AN)
            
        }
    }
    HAL_MC1HallStateChangeInterruptFlagClear();
}
// *****************************************************************************
/* Function:
   _ADCInterrupt()

  Summary:
   _ADCInterrupt() ISR routine

  Description:
    Does speed calculation and executes the vector update loop
    The ADC sample and conversion is triggered by the PWM period.
    The speed calculation assumes a fixed time interval between calculations.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void __attribute__((__interrupt__,no_auto_psv)) _ADCInterrupt()
{
    LATEbits.LATE1=0;//Disable SPI
#ifdef SINGLE_SHUNT 
    if (IFS4bits.PWM1IF ==1)
    {
        singleShuntParam.adcSamplePoint = 0;
        IFS4bits.PWM1IF = 0;
    }    
    /* If single shunt algorithm is enabled, two ADC interrupts will be
     serviced every PWM period in order to sample current twice and
     be able to reconstruct the three phases */

    switch(singleShuntParam.adcSamplePoint)
    {
        case SS_SAMPLE_BUS1:
            /*Set Trigger to measure BusCurrent Second sample during PWM 
              Timer is counting up*/
            singleShuntParam.adcSamplePoint = 1;  
            /* Ibus is measured and offset removed from measurement*/
            singleShuntParam.Ibus1 = (int16_t)(ADCBUF_INV_A_IBUS) - 
                                            measureInputs.current.offsetIbus;                        
        break;

        case SS_SAMPLE_BUS2:
            /*Set Trigger to measure BusCurrent first sample during PWM 
              Timer is counting up*/
            INVERTERA_PWM_TRIGA = ADC_SAMPLING_POINT;
            singleShuntParam.adcSamplePoint = 0;
            /* this interrupt corresponds to the second trigger and 
                save second current measured*/
            /* Ibus is measured and offset removed from measurement*/
            singleShuntParam.Ibus2 = (int16_t)(ADCBUF_INV_A_IBUS) - 
                                            measureInputs.current.offsetIbus;
            ADCON3Lbits.SWCTRG = 1;
        break;

        default:
        break;  
    }
#endif
    if (uGF.bits.MotorState!=0b11)
    {
        //Measure the currents and store them in measure Inputs variable
        measureInputs.current.Ia = ADCBUF_INV_A_IPHASE1;
        measureInputs.current.Ib = ADCBUF_INV_A_IPHASE2; 
        measureInputs.current.Ibus= ADCBUF_INV_A_IBUS;

        //Calibrate based on offset (subtract offset from the measured value)
        MCAPP_MeasureCurrentCalibrate(&measureInputs);
        iabc.a = measureInputs.current.Ia;
        iabc.b = measureInputs.current.Ib;

        /* Calculate qId,qIq from qSin,qCos,qIa,qIb */
        MC_TransformClarke_Assembly(&iabc,&ialphabeta);
        MC_TransformPark_Assembly(&ialphabeta,&sincosTheta,&idq);
        /* Calculate control values */
        DoControl();
       
        /* Calculate qAngle */
        CalculateParkAngle();

        mcappData.periodStateVar+= (((long int)mcappData.period - 
                (long int)mcappData.periodFilter)*(int)(mcappData.PeriodKFilter));
        
        mcappData.periodFilter = (int)(mcappData.periodStateVar>>15);
        
        //If it's in reverse make sure the speed and the phase inc reflect that
        if(uGF.bits.MotorState==0b01){
            mcappData.phaseInc = __builtin_divud((uint32_t)PHASE_INC_MULTI,
                                        (unsigned int)(mcappData.periodFilter));

            mcappData.SpeedHall = __builtin_divud((uint32_t)SPEED_MULTI, 
                                        (unsigned int)(mcappData.periodFilter));
        }
        //Otherwise it's not backwards so we good
        else if (uGF.bits.MotorState==0b10){
            mcappData.phaseInc = -__builtin_divud((uint32_t)PHASE_INC_MULTI,
                                        (unsigned int)(mcappData.periodFilter));

            mcappData.SpeedHall = -__builtin_divud((uint32_t)SPEED_MULTI, 
                                        (unsigned int)(mcappData.periodFilter));
        }
        /* if open loop */
        if (uGF.bits.OpenLoop == 1)
        {
            /* the angle is given by hall sensor in six steps */
            thetaElectrical = thetaElectricalOpenLoop;
        }
        //We are doing closed loop, add phase inclination to 
        else
        {
            /* if closed loop, angle generated by hall sensor */
            thetaElectrical = mcappData.HallTheta;

            mcappData.HallTheta = mcappData.HallTheta + mcappData.phaseInc; 

            if(mcappData.hallCorrectionCounter > 0)
            {
                mcappData.HallTheta = mcappData.HallTheta + mcappData.hallCorrectionFactor;
                mcappData.hallCorrectionCounter--;
            }
        }


        MC_CalculateSineCosine_Assembly_Ram(thetaElectrical,&sincosTheta);
        MC_TransformParkInverse_Assembly(&vdq,&sincosTheta,&valphabeta);

        MC_TransformClarkeInverseSwappedInput_Assembly(&valphabeta,&vabc);

        MC_CalculateSpaceVectorPhaseShifted_Assembly(&vabc,pwmPeriod,
                                                    &pwmDutycycle);
        PWMDutyCycleSet(&pwmDutycycle);            
    }
  
    else
    {
        INVERTERA_PWM_TRIGA = ADC_SAMPLING_POINT;
        pwmDutycycle.dutycycle3 = MIN_DUTY;
        pwmDutycycle.dutycycle2 = MIN_DUTY;
        pwmDutycycle.dutycycle1 = MIN_DUTY;
        PWMDutyCycleSet(&pwmDutycycle);
    } 
    if (uGF.bits.MotorState == 0b11)
    {
        measureInputs.current.Ia = ADCBUF_INV_A_IPHASE1;
        measureInputs.current.Ib = ADCBUF_INV_A_IPHASE2; 
        measureInputs.current.Ibus = ADCBUF_INV_A_IBUS; 
    }
   
    //If the offset value wasn't recently updated we measure it.
    if (MCAPP_MeasureCurrentOffsetStatus(&measureInputs) == 0)
    {
       //
        MCAPP_MeasureCurrentOffset(&measureInputs);
    }
    else
    {
        //Increments number of times it's entered ISR, don't know why we only do this sometimes
        BoardServiceStepIsr(); 
    }
    
    //Need to change the potvalue, this will instead be controlled by spi inputs
    measureInputs.potValue = (int16_t)( ADCBUF_SPEED_REF_A>>1);
    
    
    measureInputs.dcBusVoltage = (int16_t)( ADCBUF_VBUS_A>>1);

    //Measuring temp is good for errors
    MCAPP_MeasureTemperature(&measureInputs,(int16_t)(ADCBUF_MOSFET_TEMP_A>>1));

    //Update X2C scope
    DiagnosticsStepIsr();
    
    // State and message processing logic
    /*if(++SPICounter>=SENSOR_SAMPLE_RATE_REVS){
        
        SPIFlag = 1;
        SPICounter=0;
    }
     * */



    /* Read ADC Buffet to Clear Flag */
	adcDataBuffer = ClearADCIF_ReadADCBUF();
    ClearADCIF();   
}

void __attribute__((interrupt, no_auto_psv)) _SPI1RXInterrupt(void)
{
      
    // Check if SPI receive buffer is full
    if (SPI1STATLbits.SPIRBF == 0x01) {
        
        rxBuffer = SPI1BUFL;  // Read received data
        
        /* Processing Commands */
        if (rxBuffer == 0x01) { // store command
            
            /* State Processing Logic */
            if (system.state == IDLE || system.state == GENERATING) {
                
                system.state = STORING; // set state to storing 
                
                /* Begin Motor Spinning */
                ResetParmeters();
                uGF.bits.MotorState = 0b01;
                EnablePWMOutputsInverterAMotor();
                
            } else if (system.state == PAUSED) {
                
                if (system.unpausedState == IDLE || system.unpausedState == STORING || system.unpausedState == GENERATING) {
                    
                    system.state = STORING; // set state to storing 

                    /* Begin Motor Spinning */
                    ResetParmeters();
                    uGF.bits.MotorState = 0b01;
                    EnablePWMOutputsInverterAMotor();
                    if (system.unpausedState != IDLE) {
                        /* TODO: Unlock System */
                    }
                
                } else if (system.unpausedState == STORED) {
                    system.state = STORED; // Motor has no place to go, set back to stored
                }
            } 
               
        } else if (rxBuffer == 0x02) { // generate 
            
            /* State Processing Logic */
            if (system.state == STORING || system.state == STORED) {
                
                system.state = GENERATING; // set state to generating 
                
                /* Begin Motor Spinning */
                ResetParmeters();
                uGF.bits.MotorState = 0b10;
                EnablePWMOutputsInverterAGenerator();
                
                SPI1BUFL = 0;
                
            } else if (system.state == PAUSED) {
                
                if (system.unpausedState == IDLE) {
                    
                    system.state = IDLE;
                    
                } else {
                    system.state = GENERATING; // set state to generating 

                    /* Begin Motor Spinning */
                    ResetParmeters();
                    uGF.bits.MotorState = 0b10;
                    EnablePWMOutputsInverterAGenerator();
                }
                
                
            }   
            
        } else if (rxBuffer == 0x03) { // pause
            
            /* State Processing Logic */
            if (system.state != PAUSED) { // if IDLE or STORED, simply store state and pause
                
                if (system.state == STORING || system.state == GENERATING) {
                    ResetParmeters(); // Stop the motor, reset motor parameters
                    /* TODO: Engage Locking Mechanism */
                }
                system.unpausedState = system.state;
                system.state = PAUSED; 
                 
            } else {
                if (system.unpausedState == STORING) {
                    
                    system.state = STORING; // set back to storing
                    
                    /* Begin Motor Spinning */
                    ResetParmeters();
                    uGF.bits.MotorState = 0b01;
                    EnablePWMOutputsInverterAMotor();
                    /* TODO: Unlock mechanism */
                    
                } else if (system.unpausedState == GENERATING) {
                    
                    system.state = GENERATING; // set back to generating
                    
                    /* Begin Motor Spinning */
                    ResetParmeters();
                    uGF.bits.MotorState = 0b10;
                    EnablePWMOutputsInverterAGenerator();
                    /* TODO: Unlock mechanism */
                } else {
                    system.state = system.unpausedState;
                }
            } 
            
            
            
        } else if (rxBuffer == 0x04) { // sensor data request
            /* If this is a sensor data request*/
            while(1U == SPI1STATLbits.SPITBF); //wait to make sure SPI transfer buffer is not full
            SPI1BUFL = txBufferDummy[txCounter];  // Transmit first byte then increment
            txCounter++;
            //IEC0bits.SPI1RXIE = 0; //disable SPI1 Recieve Interrupt
            SPI1IMSKLbits.SPITBEN = 1;  //sets interrupt to trigger on SPI transmit buffer empty
            IEC0bits.SPI1TXIE = 1; //enable SPI1 Transmit Interrupt
            IFS0bits.SPI1TXIF = 0; //clear TX interrupt flag
        }   
    }
    
    SPI1BUFL = 0;
    SPI1BUFH = 0;
    
    // Clear SPI interrupt flag
    IFS0bits.SPI1RXIF = 0;    // Clear SPI1 interrupt flag  
}

void __attribute__((interrupt, no_auto_psv)) _SPI1TXInterrupt(void)
{
    if (txCounter < SPI_TX_BUFFER_SIZE) {
        while (SPI1STATLbits.SPITBF);
        //SPI1BUFL = txBufferDummy[txCounter++];  // Transmit byte then increment
        SPI1BUFL = txBufferDummy[txCounter];
        txCounter++;
        if (txCounter >= SPI_TX_BUFFER_SIZE) { // If last byte of response sent
            IEC0bits.SPI1TXIE = 0; // Disable SPI1 Transmit Interrupt
            SPI1IMSKLbits.SPITBEN = 0;  //disable interrupt to trigger on SPI transmit buffer empty
            //IEC0bits.SPI1RXIE = 1; // Enable SPI1 Receive Interrupt
            IFS0bits.SPI1RXIF = 0; // clear recieve interrupt flag
            txCounter = 0; // Reset txCounter for next transmission
            LATEbits.LATE1 = 0; //clear PORTE1 (MicroBus_A_AN) to trigger future interrupts on master
        }
    }

    IFS0bits.SPI1TXIF = 0; // Clear TX interrupt flag
}

// *****************************************************************************
/* Function:
 prepareTxData(void)

  Summary:
    Loads Transmit buffer with bus voltage, bus current, and system state

  Description:
 loads array of size SPI_TX_BUFFER_SIZE into txBuffer 
 
  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */

void prepareTxData(void){

    //load bus voltage into transmit buffer
    txBuffer[0] = (uint8_t)(measureInputs.dcBusVoltage >> 8);
    txBuffer[1] = (uint8_t)(measureInputs.dcBusVoltage);
        
    //load bus current into transmit buffer
    txBuffer[2] = (uint8_t)(measureInputs.current.Ibus >> 8);
    txBuffer[3] = (uint8_t)(measureInputs.current.Ibus);
        
    //load system state into transmit buffer
    txBuffer[4] = (uint8_t)system.state;
    
}


// *****************************************************************************
/* Function:
    CalculateParkAngle ()

  Summary:
    Function calculates the angle for open loop control

  Description:
    Generate the start sine waves feeding the motor terminals
    Open loop control, forcing the motor to align and to start speeding up .
 
  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void CalculateParkAngle(void)
{
    /* if open loop */
    if (uGF.bits.OpenLoop)
    {
        /* begin with the lock sequence, for field alignment */
        if (motorStartUpData.startupLock < LOCK_TIME)
        {
            motorStartUpData.startupLock += 1;
        }
        /* Then ramp up till the end speed */
        else if (motorStartUpData.startupRamp < END_SPEED)
        {
            motorStartUpData.startupRamp += OPENLOOP_RAMPSPEED_INCREASERATE;
        }
        /* Switch to closed loop */
        else 
        {
                #ifndef OPEN_LOOP_FUNCTIONING
                    uGF.bits.ChangeMode = 1;
                    uGF.bits.OpenLoop = 0;
                #endif
            }
        /* The angle set depends on startup ramp */
        thetaElectricalOpenLoop = sectorToAngle[mcappData.sector];
        mcappData.HallTheta = thetaElectricalOpenLoop;
    }
}
// *****************************************************************************
/* Function:
    InitControlParameters()

  Summary:
    Function initializes control parameters

  Description:
    Initialize control parameters: PI coefficients, scaling constants etc.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void InitControlParameters(void)
{
    
    ctrlParm.qRefRamp = SPEEDREFRAMP;
    ctrlParm.speedRampCount = SPEEDREFRAMP_COUNT;
    /* Set PWM period to Loop Time */
    pwmPeriod = LOOPTIME_TCY;
 
    /* PI - Id Current Control */
    piInputId.piState.kp = D_CURRCNTR_PTERM;
    piInputId.piState.ki = D_CURRCNTR_ITERM;
    piInputId.piState.kc = D_CURRCNTR_CTERM;
    piInputId.piState.outMax = D_CURRCNTR_OUTMAX;
    piInputId.piState.outMin = -piInputId.piState.outMax;
    piInputId.piState.integrator = 0;
    piOutputId.out = 0;

    /* PI - Iq Current Control */
    piInputIq.piState.kp = Q_CURRCNTR_PTERM;
    piInputIq.piState.ki = Q_CURRCNTR_ITERM;
    piInputIq.piState.kc = Q_CURRCNTR_CTERM;
    piInputIq.piState.outMax = Q_CURRCNTR_OUTMAX;
    piInputIq.piState.outMin = -piInputIq.piState.outMax;
    piInputIq.piState.integrator = 0;
    piOutputIq.out = 0;

    /* PI - Speed Control */
    piInputOmega.piState.kp = SPEEDCNTR_PTERM;
    piInputOmega.piState.ki = SPEEDCNTR_ITERM;
    piInputOmega.piState.kc = SPEEDCNTR_CTERM;
    piInputOmega.piState.outMax = SPEEDCNTR_OUTMAX;
    piInputOmega.piState.outMin = -piInputOmega.piState.outMax;
    piInputOmega.piState.integrator = 0;
    piOutputOmega.out = 0;
}

void __attribute__((__interrupt__,no_auto_psv)) _PWMInterrupt()
{
    ResetParmeters();
    ClearPWMPCIFaultInverterA();
    LED1 = 1; 
    ClearPWMIF(); 
}
