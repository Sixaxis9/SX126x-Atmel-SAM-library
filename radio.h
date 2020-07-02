/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Handling of the node configuration protocol

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Gregory Cristian and Matthieu Verdy

Modifier: Marco Giordano
*/
#ifndef __RADIO_H__
#define __RADIO_H__

#include "mbed.h"

/*!
 * \brief Structure describing the radio status
 */
typedef union RadioStatus_u
{
    uint8_t Value;
    struct
    {   //bit order is lsb -> msb
        uint8_t Reserved  : 1;  //!< Reserved
        uint8_t CmdStatus : 3;  //!< Command status
        uint8_t ChipMode  : 3;  //!< Chip mode
        uint8_t CpuBusy   : 1;  //!< Flag for CPU radio busy 
    }Fields;
}RadioStatus_t;

/*!
 * \brief Structure describing the error codes for callback functions
 */
typedef enum
{
    IRQ_HEADER_ERROR_CODE                   = 0x01,
    IRQ_SYNCWORD_ERROR_CODE                 = 0x02,
    IRQ_CRC_ERROR_CODE                      = 0x04,
}IrqErrorCode_t;


enum IrqPblSyncHeaderCode_t
{
    IRQ_PBL_DETECT_CODE                     = 0x01,
    IRQ_SYNCWORD_VALID_CODE                 = 0x02,
    IRQ_HEADER_VALID_CODE                   = 0x04,
};

enum IrqTimeoutCode_t
{
    IRQ_RX_TIMEOUT                          = 0x00,
    IRQ_TX_TIMEOUT                          = 0x01,
    IRQ_XYZ                                 = 0xFF,
};

/*!
 * \brief The radio command opcodes
 */
typedef enum RadioCommands_e RadioCommands_t;

/*!
 * \brief The radio callbacks structure
 * Holds function pointers to be called on radio interrupts
 */
typedef struct
{
    void ( *txDone )( void );                       //!< Pointer to a function run on successful transmission
    void ( *rxDone )( void );                       //!< Pointer to a function run on successful reception
    void ( *rxPreambleDetect )( void );             //!< Pointer to a function run on successful Preamble detection
    void ( *rxSyncWordDone )( void );               //!< Pointer to a function run on successful SyncWord reception
    void ( *rxHeaderDone )( void );                 //!< Pointer to a function run on successful Header reception
    void ( *txTimeout )( void );                    //!< Pointer to a function run on transmission timeout
    void ( *rxTimeout )( void );                    //!< Pointer to a function run on reception timeout
    void ( *rxError )( IrqErrorCode_t errCode );    //!< Pointer to a function run on reception error
    void ( *cadDone )( bool cadFlag );              //!< Pointer to a function run on channel activity detected
}RadioCallbacks_t;

/*!
 * \brief Class holding the basic communications with a radio
 *
 * It sets the functions to read/write registers, send commands and read/write
 * payload.
 * It also provides functions to run callback functions depending on the
 * interrupts generated from the radio.
 */
class Radio
{
protected:
    /*!
     * \brief Callback on Tx done interrupt
     */
    void ( *txDone )( void );

    /*!
     * \brief Callback on Rx done interrupt
     */
    void ( *rxDone )( void );

    /*!
     * \brief Callback on Rx preamble detection interrupt
     */
    void ( *rxPreambleDetect )( void );

    /*!
     * \brief Callback on Rx SyncWord interrupt
     */
    void ( *rxSyncWordDone )( void );

    /*!
     * \brief Callback on Rx header received interrupt
     */
    void ( *rxHeaderDone )( void );

    /*!
     * \brief Callback on Tx timeout interrupt
     */
    void ( *txTimeout )( void );

    /*!
     * \brief Callback on Rx timeout interrupt
     */
    void ( *rxTimeout )( void );

    /*!
     * \brief Callback on Rx error interrupt
     *
     * \param [out] errCode       A code indicating the type of interrupt (SyncWord error or CRC error)
     */
    void ( *rxError )( IrqErrorCode_t errCode );

    /*!
     * \brief Callback on Channel Activity Detection done interrupt
     *
     * \param [out] cadFlag       Flag for channel activity detected or not
     */
    void ( *cadDone )( bool cadFlag );

public:
    /*!
     * \brief Constructor for radio class
     * Sets the callbacks functions pointers
     *
     * \param [in]  callbacks     The structure of callbacks function pointers
     *                            to be called on radio interrupts
     *
     */
    Radio( RadioCallbacks_t *callbacks )
    {
        this->txDone = callbacks->txDone;
        this->rxDone = callbacks->rxDone;
        this->rxPreambleDetect = callbacks->rxPreambleDetect;
        this->rxSyncWordDone = callbacks->rxSyncWordDone;
        this->rxHeaderDone = callbacks->rxHeaderDone;
        this->txTimeout = callbacks->txTimeout;
        this->rxTimeout = callbacks->rxTimeout;
        this->rxError = callbacks->rxError;
        this->cadDone = callbacks->cadDone;
    }
    virtual ~Radio( void ){ };

    /*!
     * \brief Resets the radio
     */
    virtual void Reset( void ) = 0;

    /*!
     * \brief Gets the current radio status
     *
     * \retval      status        Radio status
     */
    virtual RadioStatus_t GetStatus( void ) = 0;

    /*!
     * \brief Writes the given command to the radio
     *
     * \param [in]  opcode        Command opcode
     * \param [in]  buffer        Command parameters byte array
     * \param [in]  size          Command parameters byte array size
     */
    virtual void WriteCommand( RadioCommands_t opcode, uint8_t *buffer, uint16_t size ) = 0;

    /*!
     * \brief Reads the given command from the radio
     *
     * \param [in]  opcode        Command opcode
     * \param [in]  buffer        Command parameters byte array
     * \param [in]  size          Command parameters byte array size
     */
    virtual void ReadCommand( RadioCommands_t opcode, uint8_t *buffer, uint16_t size ) = 0;

    /*!
     * \brief Writes multiple radio registers starting at address
     *
     * \param [in]  address       First Radio register address
     * \param [in]  buffer        Buffer containing the new register's values
     * \param [in]  size          Number of registers to be written
     */
    virtual void WriteRegister( uint16_t address, uint8_t *buffer, uint16_t size ) = 0;

    /*!
     * \brief Writes the radio register at the specified address
     *
     * \param [in]  address       Register address
     * \param [in]  value         New register value
     */
    virtual void WriteReg( uint16_t address, uint8_t value ) = 0;

    /*!
     * \brief Reads multiple radio registers starting at address
     *
     * \param [in]  address       First Radio register address
     * \param [out] buffer        Buffer where to copy the registers data
     * \param [in]  size          Number of registers to be read
     */
    virtual void ReadRegister( uint16_t address, uint8_t *buffer, uint16_t size ) = 0;

    /*!
     * \brief Reads the radio register at the specified address
     *
     * \param [in]  address       Register address
     *
     * \retval      value         The register value
     */
    virtual uint8_t ReadReg( uint16_t address ) = 0;

    /*!
     * \brief Writes Radio Data Buffer with buffer of size starting at offset.
     *
     * \param [in]  offset        Offset where to start writing
     * \param [in]  buffer        Buffer pointer
     * \param [in]  size          Buffer size
     */
    virtual void WriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size ) = 0;

    /*!
     * \brief Reads Radio Data Buffer at offset to buffer of size
     *
     * \param [in]  offset        Offset where to start reading
     * \param [out] buffer        Buffer pointer
     * \param [in]  size          Buffer size
     */
    virtual void ReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size ) = 0;
};

#endif // __RADIO_H__

