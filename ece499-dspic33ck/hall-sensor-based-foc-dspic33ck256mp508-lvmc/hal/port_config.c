/*******************************************************************************
  Input / Output Port COnfiguration Routine source File

  File Name:
    port_config.c

  Summary:
    This file includes subroutine for initializing GPIO pins as analog/digital,
    input or output etc. Also to PPS functionality to Remap-able input or output 
    pins

  Description:
    Definitions in the file are for dsPIC33CK256MP508 on Motor Control 
    Development board from Microchip
 
*******************************************************************************/
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <xc.h>
#include "port_config.h"
#include "userparms.h"

// *****************************************************************************
/* Function:
    SetupGPIOPorts()

  Summary:
    Routine to set-up GPIO ports

  Description:
    Function initializes GPIO pins for input or output ports,analog/digital pins,
    remap the peripheral functions to desires RPx pins.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */

void SetupGPIOPorts(void)
{
    // Reset all PORTx register (all inputs)
    #ifdef TRISA
        TRISA = 0xFFFF;
        LATA  = 0x0000;
    #endif
    #ifdef ANSELA
        ANSELA = 0x0000;
    #endif

    #ifdef TRISB
        TRISB = 0xFFFF;
        LATB  = 0x0000;
    #endif
    #ifdef ANSELB
        ANSELB = 0x0000;
    #endif

    #ifdef TRISC
        TRISC = 0xFFFF;
        LATC  = 0x0000;
    #endif
    #ifdef ANSELC
        ANSELC = 0x0000;
    #endif

    #ifdef TRISD
        TRISD = 0xFFFF;
        LATD  = 0x0000;
    #endif
    #ifdef ANSELD
        ANSELD = 0x0000;
    #endif

    #ifdef TRISE
        TRISE = 0xFFFF;
        LATE  = 0x0000;
    #endif
    #ifdef ANSELE
        ANSELE = 0x0000;
    #endif

    MapGPIOHWFunction();

    return;
}
// *****************************************************************************
/* Function:
    Map_GPIO_HW_Function()

  Summary:
    Routine to setup GPIO pin used as input/output analog/digital etc

  Description:
    Function initializes GPIO pins as input or output port pins,analog/digital 
    pins,remap the peripheral functions to desires RPx pins.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */

void MapGPIOHWFunction(void)
{
    
    /* ANALOG SIGNALS */

    // Configure Port pins for Motor Current Sensing
    
    // Ia Out
    ANSELAbits.ANSELA0 = 1;
    TRISAbits.TRISA0 = 1;   // Pin 16: OA1OUT/AN0/CMP1A/IBIAS0/RA0
    
    //Ib Out
    ANSELBbits.ANSELB2 = 1;
    TRISBbits.TRISB2 = 1;   //Pin 41: OA2OUT/AN1/AN7/ANA0/CMP1D/CMP2D/CMP3D/RP34/SCL3/INT0/RB2
    
    //Ic Out
    ANSELBbits.ANSELB8 = 1;
    TRISBbits.TRISB8 = 1;
    
    //Ibus Out
    ANSELAbits.ANSELA4 = 1;
    TRISAbits.TRISA4 = 1;   //Pin 23: OA3OUT/AN4/CMP3B/IBIAS3/RA4
    
    //External current sensor,
    ANSELEbits.ANSELE0 = 1;
    TRISEbits.TRISE0 = 1;
    
    //External battery current sensor
    ANSELEbits.ANSELE1 = 1;
    TRISEbits.TRISE1 = 1;
    
    
    
#ifdef INTERNAL_OPAMP_CONFIG
    
    //Ia-
    ANSELAbits.ANSELA1 = 1;
    TRISAbits.TRISA1 = 1;   //Pin 18: OA1IN-/ANA1/RA1
    
    //Ia+ 
    ANSELAbits.ANSELA2 = 1;
    TRISAbits.TRISA2 = 1;   //Pin 20: OA1IN+/AN9/PMA6/RA2
    
    //Ib- 
    ANSELBbits.ANSELB3 = 1;
    TRISBbits.TRISB3 = 1;   //Pin 43: PGD2/OA2IN-/AN8/RP35/RB3
    
    //Ib+ 
    ANSELBbits.ANSELB4 = 1;
    TRISBbits.TRISB4 = 1;   //Pin 45: PGC2/OA2IN+/RP36/RB4
    
    //Ibus- 
    ANSELCbits.ANSELC1 = 1;
    TRISCbits.TRISC1 = 1;   //Pin 43: PGD2/OA2IN-/AN8/RP35/RB3
    
    //Ibus+ 
    ANSELCbits.ANSELC2 = 1;
    TRISCbits.TRISC2 = 1;   //Pin 45: PGC2/OA2IN+/RP36/RB4
    //Op-Amp Configuration
    AMPCON1Hbits.NCHDIS2 = 0;    //Wide input range for Op Amp #2
    AMPCON1Lbits.AMPEN2 = 1;     //Enables Op Amp #2
    
    AMPCON1Hbits.NCHDIS1 = 0;    //Wide input range for Op Amp #1
    AMPCON1Lbits.AMPEN1 = 1;     //Enables Op Amp #1
    
    AMPCON1Hbits.NCHDIS3 = 0;    //Wide input range for Op Amp #3
    AMPCON1Lbits.AMPEN3 = 1;     //Enables Op Amp #3
    
    AMPCON1Lbits.AMPON = 1;      //Enables op amp modules if their respective AMPENx bits are also asserted
     
#endif
    
    // Potentiometer #1 input - used as Speed Reference
    // POT1 
    ANSELBbits.ANSELB9= 1;
    TRISBbits.TRISB9 = 1;   // PIN61: PGC1/AN11/RP41/SDA1/RB9
    
    /*TEMPERATURE*/
    ANSELCbits.ANSELC0 = 1;
    TRISCbits.TRISC0 = 1;  //PIN15:AN12/ANN0/RP48/RC0
    /*DC Bus Voltage Signals*/
    ANSELCbits.ANSELC3 = 1;
    TRISCbits.TRISC3 = 1;   //PIN33: AN15/CMP2A/IBIAS2/RP51/PMD11/PMA11/RC3

    /* Digital SIGNALS */   
    // DIGITAL INPUT/OUTPUT PINS

    // Inverter Control - PWM Outputs
    // PWM1L : PIN #3  RP47/PWM1L/PMD6/RB15
    // PWM1H : PIN #1  RP46/PWM1H/PMD5/RB14
    // PWM2L : PIN #80  RP45/PWM2L/PMD4/RB13
    // PWM2H : PIN #78  TDI/RP44/PWM2H/PMD3/RB12
    // PWM4L : PIN #74  RP64/PWM4L/PMD0/RD0
    // PWM4H : PIN #73  RP65/PWM4H/RD1
    TRISBbits.TRISB14 = 0 ;          
    TRISBbits.TRISB15 = 0 ;         
    TRISBbits.TRISB12 = 0 ;          
    TRISBbits.TRISB13 = 0 ;           
    TRISDbits.TRISD1 = 0 ;          
    TRISDbits.TRISD0 = 0 ;      
    
    //Hall Sensors
    TRISEbits.TRISE8 = 1;
    TRISEbits.TRISE9 = 1;
    TRISEbits.TRISE10 = 1;
    
    // Debug LEDs
    // LED2 : 
    TRISEbits.TRISE7 = 0;           // PIN:39 - RE7
    // LED1 : 
    TRISEbits.TRISE6 = 0;           // PIN:37 - RE6

    // Push button Switches
    
    // SW1 : 
    TRISEbits.TRISE11 = 1;           // PIN:59 - RE11
    // SW2 : 
    TRISEbits.TRISE12 = 1;           // PIN:62 - RE12
    //SW3 :
    TRISEbits.TRISE13 = 1;             //PIN: 64 - RE13
    
    /** Output Pin to Trigger Interrupt on Master ((MicroBus_A_AN) **/
    TRISDbits.TRISD15 = 0;            //PIN: 79  - RD15 [To write: write to LATEbits.LATE1 or _LATE1] 
    
    
	
	/** Diagnostic Interface for LVMC Board etc.
        Re-map UART Channels to the device pins connected to the following 
        PIM pins on the Motor Control Development Boards .
        UART_RX : PIN #13 - RP78/PCI21/RD14 (Input)
        UART_TX : PIN #14 - ANN2/RP77/RD13(Output)   */
    _U1RXR = 78;
    _RP77R = 0b000001;
    
    /** SPI Port Configuration for LVMC board 
    * SPI1DR (SPI1 Input(MOSI))         : PIN #36 AN19/CMP2C/RP75/PMA0/PMALL/PSA0/RD11
    * SCK1R  (SPI1 CLK Input)           : PIN #27 RP76/RD12
    * SS1R   (SPI1 Slave Select Input)  : PIN #75 TMS/RP42/PWM3H/PMD1/RB10
    * _RP74R  (SPI1 Output (MISO))      : PIN #38 AN18/CMP3C/ISRC3/RP74/PMD9/PMA9/RD10
    */
   // Inputs: Require RP (Re-mappable pin configuration)
   _SDI1R = 75;  // RP75 (RD11) as SPI1 SDI input
   _SCK1R = 76;  // RP76 (RD12) as SPI1 SCK input
   _SS1R  = 42;  // RP42 (RB10) as SPI1 SS input

   // Outputs: Require mapping RP to function
   _RP74R = _RPOUT_SDO1;  // Map RP74 (RD10) to SPI1 SDO (MISO) output
    
    // Change Notification Configuration
    CN_Configure();
}

/* Function:
    CN_Configure()

  Summary:
    Routine to setup change notifications

  Description:
    Function initializes mismatch/edge change detection and enabling
	corresponding pins to obtain change notification status

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */

void CN_Configure(void)
{
    CNCONE = 0;
/*  ON: Change Notification (CN) Control for PORTx On bit
    1 = CN is enabled
    0 = CN is disabled   */
    CNCONEbits.ON = 0;
/*    CNSTYLE: Change Notification Style Selection bit
    1 = Edge style (detects edge transitions, bits are used for a CNE)
    0 = Mismatch style (detects change from last port read event)       */    
    CNCONEbits.CNSTYLE = 0;     
     
    CNEN0E = 0;
    CNEN0Ebits.CNEN0E8 = 1;
    CNEN0Ebits.CNEN0E9 = 1;
    CNEN0Ebits.CNEN0E10 = 1;

    _CNEIF = 0;
    _CNEIE = 1;
    _CNEIP = 7;
}
