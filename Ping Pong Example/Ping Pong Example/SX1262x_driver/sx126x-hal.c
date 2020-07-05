/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Handling of the node configuration protocol

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Gregory Cristian and Matthieu Verdy

__/\\\\____________/\\\\_____/\\\\\\\\\\\\_        
 _\/\\\\\\________/\\\\\\___/\\\//////////__       
  _\/\\\//\\\____/\\\//\\\__/\\\_____________      
   _\/\\\\///\\\/\\\/_\/\\\_\/\\\____/\\\\\\\_     
    _\/\\\__\///\\\/___\/\\\_\/\\\___\/////\\\_    
     _\/\\\____\///_____\/\\\_\/\\\_______\/\\\_   
      _\/\\\_____________\/\\\_\/\\\_______\/\\\_  
       _\/\\\_____________\/\\\_\//\\\\\\\\\\\\/__ 
        _\///______________\///___\////////////____

Modifier: Marco Giordano
*/
#include "sx126x-hal.h"
#include "device_agnostic_driver.h"

/*!
 * \brief Used to block execution to give enough time to Busy to go up
 *        in order to respect Tsw, see datasheet section 8.3.1
 */
#define WaitOnCounter( )          for( uint8_t counter = 0; counter < 15; counter++ ) \
                                  {  __NOP( ); }

                                  
// This code handles cases where assert_param is undefined
#ifndef assert_param
#define assert_param( ... )
#endif

void SX126xHal_SpiInit( void )
{
    NSS_OFF
    SPI_init();

    delay_ms( 100 );
}


void SX126xHal_IoIrqInit( DioIrqHandler irqHandler )
{
    IRQ_Init()
}

void SX126xHal_Reset( void )
{
    CRITICAL_SECTION_ENTER
    wait_ms( 20 );
    RESET_ON
    wait_ms( 50 );
    RESET_OFF
    wait_ms( 20 );
    CRITICAL_SECTION_LEAVE
}

void SX126xHal_Wakeup( void )
{
    CRITICAL_SECTION_ENTER

    //Don't wait for BUSY here
    uint8_t wakeup_sequence[2] = {RADIO_GET_STATUS, 0x00}
    NSS_OFF
    SendSpi(wakeup_sequence, sizeof(wakeup_sequence));
    NSS_ON

    // Wait for chip to be ready.
    WAIT_BUSY

    CRITICAL_SECTION_LEAVE
    
    AntSwOn( );
}

void SX126xHal_WriteCommand( RadioCommands_t *command, uint8_t *buffer, uint16_t size )
{   
    WAIT_BUSY

    NSS_OFF

    SendSpi((uint8_t *) command, 1);
    SendSpi(buffer, size);

    NSS_ON
    
    WaitOnCounter( );
}

void SX126xHal_ReadCommand( RadioCommands_t *command, uint8_t *buffer, uint16_t size )
{
    WAIT_BUSY

    NSS_OFF

    SendSpi(( uint8_t *) command, 1);
    ReadSpi(buffer, size);
    
    NSS_ON
    
}

void SX126xHal_WriteRegister( uint16_t address, uint8_t *buffer, uint16_t size )
{
    WAIT_BUSY

    NSS_OFF
    
    SendSpi(RADIO_WRITE_REGISTER, 1);
    SendSpi(&address, 2);
    SendSpi(buffer, size);
    
    NSS_ON

}

void SX126xHal_WriteReg( uint16_t address, uint8_t value )
{
    SX126xHal_WriteRegister( address, &value, 1 );
}

void SX126xHal_ReadRegister( uint16_t address, uint8_t *buffer, uint16_t size )
{
    WAIT_BUSY

    NSS_OFF
    
    SendSpi(&address, 2)
    ReadSpi(buffer, size) 
   
    NSS_ON
    
}

void SX126xHal_ReadReg( uint16_t address, uint8_t *data )
{
    ReadRegister( address, data, 1 );
}

void SX126xHal_WriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    WAIT_BUSY

    NSS_OFF

    SendSpi(RADIO_WRITE_BUFFER, 1);
    SendSpi(&offset, 1);
    SendSpi(buffer, size);
    
    NSS_ON

}

void SX126xHal_ReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    WAIT_BUSY

    NSS_OFF

    SendSpi(RADIO_READ_BUFFER, 1);
    SendSpi(offset, 1);
    SendSpi(0, 1);

    rx_data = ReadSpi(buffer, size) 

    NSS_ON
}

uint8_t SX126xHal_GetDioStatus( void )
{
    return ( read_pin(DIO3) << 3 ) | ( read_pin(DIO2) << 2 ) | ( read_pin(DIO1) << 1 ) | ( read_pin(BUSY) << 0 );
}

void SX126xHal_AntSwOn( void )
{
    antSwitchPower = 1;
}

void SX126xHal_AntSwOff( void )
{
    antSwitchPower = 0;
}
