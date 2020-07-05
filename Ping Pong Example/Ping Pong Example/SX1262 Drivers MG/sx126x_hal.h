/*
This library is a device independant implementation of the library by Semtech
from Miguel Luis, Gregory Cristian and Matthieu Verdy
-> https://os.mbed.com/teams/Semtech/code/SX126xLib/

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

#ifndef __SX126x_HAL_H__
#define __SX126x_HAL_H__

#include "device_specific_implementation.h"
#include "sx126x_commands.h"

/*!
 * \brief Abstraction layer for the sx126x commands
 */


/*!
    * \brief Initialize the SPI communication on the selected microcontroller
    */
void SX126xHal_SpiInit( void );

/*!
    * \brief Initialize the interrupt on the selected microcontroller
    */
void SX126xHal_IoIrqInit( void );

/*!
    * \brief Soft resets the radio
    */
void SX126xHal_Reset( void );

/*!
    * \brief Wakes up the radio
    */
void SX126xHal_Wakeup( void );

/*!
    * \brief Send a command that write data to the radio
    *
    * \param [in]  opcode        Opcode of the command
    * \param [in]  buffer        Buffer to be send to the radio
    * \param [in]  size          Size of the buffer to send
    */
void SX126xHal_WriteCommand( RadioCommands_t *opcode, uint8_t *buffer, uint16_t size );

/*!
    * \brief Send a command that read data from the radio
    *
    * \param [in]  opcode        Opcode of the command
    * \param [out] buffer        Buffer holding data from the radio
    * \param [in]  size          Size of the buffer
    */
void SX126xHal_ReadCommand( RadioCommands_t *opcode, uint8_t *buffer, uint16_t size );

/*!
    * \brief Write data to the radio memory
    *
    * \param [in]  address       The address of the first byte to write in the radio
    * \param [in]  buffer        The data to be written in radio's memory
    * \param [in]  size          The number of bytes to write in radio's memory
    */
void SX126xHal_WriteRegister( uint16_t address, uint8_t *buffer, uint16_t size );

/*!
    * \brief Write a single byte of data to the radio memory
    *
    * \param [in]  address       The address of the first byte to write in the radio
    * \param [in]  value         The data to be written in radio's memory
    */
void SX126xHal_WriteReg( uint16_t address, uint8_t *value );

/*!
    * \brief Read data from the radio memory
    *
    * \param [in]  address       The address of the first byte to read from the radio
    * \param [out] buffer        The buffer that holds data read from radio
    * \param [in]  size          The number of bytes to read from radio's memory
    */
void SX126xHal_ReadRegister( uint16_t address, uint8_t *buffer, uint16_t size );

/*!
    * \brief Read a single byte of data from the radio memory
    *
    * \param [in]  address       The address of the first byte to write in the
    *                            radio
    *
    * \retval      value         The value of the byte at the given address in
    *                            radio's memory
    */
void SX126xHal_ReadReg( uint16_t address , uint8_t *data );

/*!
    * \brief Write data to the buffer holding the payload in the radio
    *
    * \param [in]  offset        The offset to start writing the payload
    * \param [in]  buffer        The data to be written (the payload)
    * \param [in]  size          The number of byte to be written
    */
void SX126xHal_WriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size );

/*!
    * \brief Read data from the buffer holding the payload in the radio
    *
    * \param [in]  offset        The offset to start reading the payload
    * \param [out] buffer        A pointer to a buffer holding the data from the radio
    * \param [in]  size          The number of byte to be read
    */
void SX126xHal_ReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size );

/*!
    * \brief Returns the status of DIOs pins
    *
    * \retval      dioStatus     A byte where each bit represents a DIO state:
    *                            [ DIO3 | DIO2 | DIO1 | BUSY ]
    */
uint8_t SX126xHal_GetDioStatus( void );

/*!
    * \brief RF Switch power on
    */
void SX126xHal_AntSwOn( void );

/*!
    * \brief RF Switch power off
    */
void SX126xHal_AntSwOff( void );

#endif // __SX126x_HAL_H__
