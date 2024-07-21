
/**
 * SPI1 Generated Driver Source File
 * 
 * @file        spi1.c
 * 
 * @ingroup     spiclientdriver
 * 
 * @brief       This is the generated driver source file for SPI1 driver.
 *
 * @skipline @version     Firmware Driver Version 1.1.3
 *
 * @skipline @version     PLIB Version 1.3.0
 *
 * @skipline    Device : dsPIC33CK256MP508
*/

/*
© [2024] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

// Section: Included Files
#include <xc.h>
#include "../../ece499-dspic33ck/hall-sensor-based-foc-dspic33ck256mp508-lvmc/hal/spi1.h"

// Section: File specific data type definitions

/** 
 @ingroup spidriver
  @brief Dummy data to be sent for half duplex communication.
*/
#define SPI1_DUMMY_DATA 0x0

//Defines an object for SPI_CLIENT_INTERFACE.

const struct SPI_CLIENT_INTERFACE SPI1_Client = {
    .Initialize         = &SPI1_Initialize,
    .Deinitialize       = &SPI1_Deinitialize,
    .Close              = &SPI1_Close,
    .Open               = &SPI1_Open,
    .ByteExchange       = &SPI1_ByteExchange,
    .ByteRead           = &SPI1_ByteRead,    
    .ByteWrite          = &SPI1_ByteWrite,
    .IsRxReady          = &SPI1_IsRxReady,
    .IsTxReady          = &SPI1_IsTxReady,
};
        

/**
 @ingroup spiclientdriver
 @struct SPI1_CONFIG 
 @brief Defines the SPI1 configuration.
*/
struct SPI1_CLIENT_CONFIG
{ 
    uint16_t controlRegister1; //SPI1CON1L
};

static const struct SPI1_CLIENT_CONFIG config[] = {  
                                        { 
                                            /*Configuration setting for CLIENT_CONFIG.
                                            SPI Mode : Mode 1*/
                                            0x81,//SPI1CON1L
                                        },
                                    };

// Section: Driver Interface Function Definitions

void SPI1_Initialize (void)
{
    /*Clearing SPI BUF Registers*/
    SPI1BUFH = 0;
    SPI1BUFL = 0;
    
    /*Clearing Interrupts for Configuration*/
    IFS0bits.SPI1RXIF = 0; // Clear RX Interrupt Flag
    IFS0bits.SPI1TXIF = 0; //clear TX Interrupt Flag
    IEC0bits.SPI1RXIE = 0; // Disable RX Interrupt
    IEC0bits.SPI1TXIE = 0; // Disable TX Interrupt
    
    /*Set SPI Interrupt Priority*/
    IPC31bits.SPI1IP = 7;
    IPC2bits.SPI1RXIP = 7;
    IPC2bits.SPI1TXIP = 6;
    
    /* SPI1CON1 Register Settings*/
    SPI1CON1bits.DISSCK = 0; // Internal Serial Clock is enabled
    SPI1CON1bits.DISSDO = 0; // SDOx pin is controlled by the module
    SPI1CON1bits.MODE16 = 0; // Communication is byte-wide (8 bits)
    SPI1CON1bits.SMP = 0; // Input data is sampled at the middle of data output time.
    SPI1CON1bits.CKE = 1; // Serial output data changes on transition from Idle clock state to active clock state
    SPI1CON1bits.CKP = 0; // Idle state for clock is a low level; active
    SPI1CON1bits.MSTEN = 0; // Master mode disabled
    SPI1CON1bits.SSEN = 1; //Set Slave select pin to active for slave mode
    
    /*SPI1 Status Register Setting*/ 
    SPI1STATLbits.SPIROV = 0; // No Receive Overflow has occurred
    
    /*SPI1 Interrupt Mask Register*/
    SPI1IMSKLbits.SPIRBFEN = 1; //sets interrupt to trigger on SPI recieve buffer full
    
    /*Enabling the Interrupt Module*/
    IFS0bits.SPI1RXIF = 0; // Clear RX Interrupt Flag
    IFS0bits.SPI1TXIF = 0; // Clear TX Interrupt Flag 
    IEC0bits.SPI1RXIE = 1; //enable SPI1 RX interrupt
    SPI1CON1bits.SPIEN = 1; //enable SPI module
    
}

void SPI1_Deinitialize (void)
{
    SPI1_Close();
    
    SPI1BRGL = 0x0;
    SPI1CON1L = 0x0;
    SPI1CON1H = 0x0;
    SPI1CON2L = 0x0;
    SPI1STATL = 0x28;
    SPI1URDTL = 0x0;
    SPI1URDTH = 0x0;
}

void SPI1_Close(void)
{
    SPI1CON1Lbits.SPIEN = 0U;
}

bool SPI1_Open(uint8_t spiConfigIndex)
{
    bool status = false;
    if(!SPI1CON1Lbits.SPIEN)
    {
        SPI1CON1L = config[spiConfigIndex].controlRegister1;
        SPI1CON1Lbits.SPIEN = 1U;
        
        status = true;
    }
    return status;
}

uint8_t SPI1_ByteExchange(uint8_t byteData)
{
    while(1U == SPI1STATLbits.SPITBF)
    {

    }

    SPI1BUFL = byteData;

    while (1U == SPI1STATLbits.SPIRBE)
    {
    
    }

    return SPI1BUFL;
}

void SPI1_ByteWrite(uint8_t byteData)
{
    while(1U == SPI1STATLbits.SPITBF)
    {

    }
    
    SPI1BUFL = byteData;
}

uint8_t SPI1_ByteRead(void)
{
    while (1U == SPI1STATLbits.SPIRBE)
    {
    
    }
    
    return SPI1BUFL;
}

bool SPI1_IsRxReady(void)
{    
    return (!SPI1STATLbits.SPIRBE);
}

bool SPI1_IsTxReady(void)
{    
    return (!SPI1STATLbits.SPITBF);
}

