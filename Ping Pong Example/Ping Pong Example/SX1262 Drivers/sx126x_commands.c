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

#include <string.h>

#include "sx126x_commands.h"
#include "sx126x_hal.h"

/*!
 * \brief Radio registers definition
 */
typedef struct
{
    uint16_t      Addr;                             //!< The address of the register
    uint8_t       Value;                            //!< The value of the register
}RadioRegisters_t;

/*!
 * \brief Stores the last frequency error measured on LoRa received packet
 */
volatile uint32_t FrequencyError = 0;

/*!
 * \brief Hold the status of the Image calibration
 */
static bool ImageCalibrated = false;


void SX126x_Init( void ){
        CalibrationParams_t calibParam;

        SX126xHal_SpiInit();

        SX126xHal_Reset( );

        SX126xHal_IoIrqInit();

        SX126xHal_Wakeup( );
        SX126x_SetStandby( STDBY_RC );

        if( XTAL == 0 )
        {
                SX126x_SetDio3AsTcxoCtrl( TCXO_CTRL_1_7V, 320 ); //5 ms
                calibParam.Value = 0x7F;
                SX126x_Calibrate( calibParam );
        }

        SX126xHal_AntSwOn( );
        SX126x_SetDio2AsRfSwitchCtrl( true );
        
        OperatingMode = MODE_STDBY_RC;
        
        SX126x_SetPacketType( PACKET_TYPE_LORA );


        #ifdef USE_CONFIG_PUBLIC_NETOWRK
                uint8_t pub_sync_h = (( LORA_MAC_PUBLIC_SYNCWORD >> 8 ) & 0xFF);
                uint8_t pub_sync_l = ( LORA_MAC_PUBLIC_SYNCWORD & 0xFF);
                // Change LoRa modem Sync Word for Public Networks
                SX126xHal_WriteReg( REG_LR_SYNCWORD, &pub_sync_h  );
                SX126xHal_WriteReg( REG_LR_SYNCWORD + 1, &pub_sync_l );
        #else
                uint8_t priv_sync_h = (( LORA_MAC_PRIVATE_SYNCWORD >> 8 ) & 0xFF);
                uint8_t priv_sync_l = ( LORA_MAC_PRIVATE_SYNCWORD & 0xFF);
                // Change LoRa modem SyncWord for Private Networks
                SX126xHal_WriteReg( REG_LR_SYNCWORD, &priv_sync_h );
                SX126xHal_WriteReg( REG_LR_SYNCWORD + 1, &priv_sync_l );
        #endif
}

void SX126x_SetStandby( RadioStandbyModes_t standbyConfig )
{

    SX126xHal_WriteCommand( RADIO_SET_STANDBY, ( uint8_t* )&standbyConfig, 1 );
    if( standbyConfig == STDBY_RC )
    {
        OperatingMode = MODE_STDBY_RC;
    }
    else
    {
        OperatingMode = MODE_STDBY_XOSC;
    }
}

void SX126x_SetDio3AsTcxoCtrl( RadioTcxoCtrlVoltage_t tcxoVoltage, uint32_t timeout )
{
    uint8_t buf[4];

    buf[0] = tcxoVoltage & 0x07;
    buf[1] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[2] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[3] = ( uint8_t )( timeout & 0xFF );

    SX126xHal_WriteCommand( RADIO_SET_TCXOMODE, buf, 4 );
}

void SX126x_Calibrate( CalibrationParams_t calibParam )
{
    SX126xHal_WriteCommand( RADIO_CALIBRATE, &calibParam.Value, 1 );
}

void SX126x_SetDio2AsRfSwitchCtrl( uint8_t enable )
{

    SX126xHal_WriteCommand( RADIO_SET_RFSWITCHMODE, &enable, 1 );
}

void SX126x_SetPacketType( RadioPacketTypes_t packetType )
{


    // Save packet type internally to avoid questioning the radio
    PacketType = packetType;
    SX126xHal_WriteCommand( RADIO_SET_PACKETTYPE, ( uint8_t* )&packetType, 1 );
}

RadioOperatingModes_t SX126x_GetOperatingMode( void )
{
    return OperatingMode;
}

void SX126x_CheckDeviceReady( void )
{
    if( ( SX126x_GetOperatingMode( ) == MODE_SLEEP ) || ( SX126x_GetOperatingMode( ) == MODE_RX_DC ) )
    {
        SX126xHal_Wakeup( );
        // Switch is turned off when device is in sleep mode and turned on is all other modes
        SX126xHal_AntSwOn( );
    }
}

void SX126x_SetPayload( uint8_t *payload, uint8_t size )
{
    uint8_t dummy;
    uint8_t start_buffer = 0x00;
    SX126x_GetRxBufferStatus( &dummy, &start_buffer );
    SX126xHal_WriteBuffer( start_buffer, payload, size );
}

uint8_t SX126x_GetPayload( uint8_t *buffer, uint8_t size,  uint8_t maxSize )
{
    uint8_t start_buffer = 0x00;
    SX126x_GetRxBufferStatus( &size, &start_buffer );
    if( size > maxSize )
    {
        return 1;
    }
    SX126xHal_ReadBuffer( start_buffer, buffer, size );
    return 0;
}

void SX126x_SendPayload( uint8_t *payload, uint8_t size, uint32_t timeout )
{
    SX126x_SetPayload( payload, size );
    SX126x_SetTx( timeout );
}

uint8_t SX126x_SetSyncWord( uint8_t *syncWord )
{
    SX126xHal_WriteRegister( REG_LR_SYNCWORDBASEADDRESS, syncWord, 8 );
    return 0;
}

void SX126x_SetCrcSeed( uint16_t seed )
{
    uint8_t buf[2];

    buf[0] = ( uint8_t )( ( seed >> 8 ) & 0xFF );
    buf[1] = ( uint8_t )( seed & 0xFF );

    switch( SX126x_GetPacketType( ) )
    {
        case PACKET_TYPE_GFSK:
            SX126xHal_WriteRegister( REG_LR_CRCSEEDBASEADDR, buf, 2 );
            break;

        default:
            break;
    }
}

void SX126x_SetCrcPolynomial( uint16_t polynomial )
{
    uint8_t buf[2];

    buf[0] = ( uint8_t )( ( polynomial >> 8 ) & 0xFF );
    buf[1] = ( uint8_t )( polynomial & 0xFF );

    switch( SX126x_GetPacketType( ) )
    {
        case PACKET_TYPE_GFSK:
            SX126xHal_WriteRegister( REG_LR_CRCPOLYBASEADDR, buf, 2 );
            break;

        default:
            break;
    }
}

void SX126x_SetWhiteningSeed( uint16_t seed )
{
    uint8_t regValue = 0;

    switch( SX126x_GetPacketType( ) )
    {
        case PACKET_TYPE_GFSK:
            SX126xHal_ReadReg( REG_LR_WHITSEEDBASEADDR_MSB, &regValue );
			regValue = regValue & 0xFE;
            regValue = ( ( seed >> 8 ) & 0x01 ) | regValue;
            SX126xHal_WriteReg( REG_LR_WHITSEEDBASEADDR_MSB, &regValue ); // only 1 bit.
            SX126xHal_WriteReg( REG_LR_WHITSEEDBASEADDR_LSB, (uint8_t *) &seed );
            break;

        default:
            break;
    }
}

uint32_t SX126x_GetRandom( void )
{
    uint8_t buf[] = { 0, 0, 0, 0 };

    // Set radio in continuous reception
    SX126x_SetRx( 0 );

    wait_ms( 1 );

    SX126xHal_ReadRegister( RANDOM_NUMBER_GENERATORBASEADDR, buf, 4 );

    SX126x_SetStandby( STDBY_RC );

    return ( buf[0] << 24 ) | ( buf[1] << 16 ) | ( buf[2] << 8 ) | buf[3];
}

void SX126x_SetSleep( SleepParams_t sleepConfig )
{


    SX126xHal_AntSwOff( );

    SX126xHal_WriteCommand( RADIO_SET_SLEEP, &sleepConfig.Value, 1 );
    OperatingMode = MODE_SLEEP;
}

void SX126x_SetFs( void )
{

    SX126xHal_WriteCommand( RADIO_SET_FS, 0, 0 );
    OperatingMode = MODE_FS;
}

void SX126x_SetTx( uint32_t timeout )
{
    uint8_t buf[3];

    OperatingMode = MODE_TX;
 


    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    SX126xHal_WriteCommand( RADIO_SET_TX, buf, 3 );
}

void SX126x_SetRxBoosted( uint32_t timeout )
{
    uint8_t buf[3];

    OperatingMode = MODE_RX;


    SX126xHal_WriteReg( REG_RX_GAIN, (uint8_t *)0x96 ); // max LNA gain, increase current by ~2mA for around ~3dB in sensivity

    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    SX126xHal_WriteCommand( RADIO_SET_RX, buf, 3 );
}

void SX126x_SetRx( uint32_t timeout )
{
    uint8_t buf[3];

    OperatingMode = MODE_RX;


    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    SX126xHal_WriteCommand( RADIO_SET_RX, buf, 3 );
}

void SX126x_SetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime )
{
    uint8_t buf[6];

    buf[0] = ( uint8_t )( ( rxTime >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( rxTime >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( rxTime & 0xFF );
    buf[3] = ( uint8_t )( ( sleepTime >> 16 ) & 0xFF );
    buf[4] = ( uint8_t )( ( sleepTime >> 8 ) & 0xFF );
    buf[5] = ( uint8_t )( sleepTime & 0xFF );
    SX126xHal_WriteCommand( RADIO_SET_RXDUTYCYCLE, buf, 6 );
    OperatingMode = MODE_RX_DC;
}

void SX126x_SetCad( void )
{
    SX126xHal_WriteCommand( RADIO_SET_CAD, 0, 0 );
    OperatingMode = MODE_CAD;
}

void SX126x_SetTxContinuousWave( void )
{
    SX126xHal_WriteCommand( RADIO_SET_TXCONTINUOUSWAVE, 0, 0 );
}

void SX126x_SetTxInfinitePreamble( void )
{
    SX126xHal_WriteCommand( RADIO_SET_TXCONTINUOUSPREAMBLE, 0, 0 );
}

void SX126x_SetStopRxTimerOnPreambleDetect( uint8_t enable )
{
    SX126xHal_WriteCommand( RADIO_SET_STOPRXTIMERONPREAMBLE, ( uint8_t* )&enable, 1 );
}

void SX126x_SetLoRaSymbNumTimeout( uint8_t SymbNum )
{
    SX126xHal_WriteCommand( RADIO_SET_LORASYMBTIMEOUT, &SymbNum, 1 );
}

void SX126x_SetRegulatorMode( RadioRegulatorMode_t mode )
{
    SX126xHal_WriteCommand( RADIO_SET_REGULATORMODE, ( uint8_t* )&mode, 1 );
}


void SX126x_CalibrateImage( uint32_t freq )
{
    uint8_t calFreq[2];

    if( freq > 900000000 )
    {
        calFreq[0] = 0xE1;
        calFreq[1] = 0xE9;
    }
    else if( freq > 850000000 )
    {
        calFreq[0] = 0xD7;
        calFreq[1] = 0xD8;
    }
    else if( freq > 770000000 )
    {
        calFreq[0] = 0xC1;
        calFreq[1] = 0xC5;
    }
    else if( freq > 460000000 )
    {
        calFreq[0] = 0x75;
        calFreq[1] = 0x81;
    }
    else if( freq > 425000000 )
    {
        calFreq[0] = 0x6B;
        calFreq[1] = 0x6F;
    }
    SX126xHal_WriteCommand( RADIO_CALIBRATEIMAGE, calFreq, 2 );
}

void SX126x_SetPaConfig( uint8_t paDutyCycle, uint8_t HpMax, uint8_t deviceSel, uint8_t paLUT )
{
    uint8_t buf[4];


    buf[0] = paDutyCycle;
    buf[1] = HpMax;
    buf[2] = deviceSel;
    buf[3] = paLUT;
    SX126xHal_WriteCommand( RADIO_SET_PACONFIG, buf, 4 );
}

void SX126x_SetRxTxFallbackMode( uint8_t fallbackMode )
{
    SX126xHal_WriteCommand( RADIO_SET_TXFALLBACKMODE, &fallbackMode, 1 );
}

void SX126x_SetDioIrqParams( uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask )
{
    uint8_t buf[8];


    buf[0] = ( uint8_t )( ( irqMask >> 8 ) & 0x00FF );
    buf[1] = ( uint8_t )( irqMask & 0x00FF );
    buf[2] = ( uint8_t )( ( dio1Mask >> 8 ) & 0x00FF );
    buf[3] = ( uint8_t )( dio1Mask & 0x00FF );
    buf[4] = ( uint8_t )( ( dio2Mask >> 8 ) & 0x00FF );
    buf[5] = ( uint8_t )( dio2Mask & 0x00FF );
    buf[6] = ( uint8_t )( ( dio3Mask >> 8 ) & 0x00FF );
    buf[7] = ( uint8_t )( dio3Mask & 0x00FF );
    SX126xHal_WriteCommand( RADIO_CFG_DIOIRQ, buf, 8 );
}

uint16_t SX126x_GetIrqStatus( void )
{
    uint8_t irqStatus[2];

    SX126xHal_ReadCommand( RADIO_GET_IRQSTATUS, irqStatus, 2 );
    return ( irqStatus[0] << 8 ) | irqStatus[1];
}



void SX126x_SetRfFrequency( uint32_t frequency )
{
    uint8_t buf[4];
    uint32_t freq = 0;


    if( ImageCalibrated == false )
    {
        SX126x_CalibrateImage( frequency );
        ImageCalibrated = true;
    }

    freq = ( uint32_t )( ( double )frequency / ( double )FREQ_STEP );
    buf[0] = ( uint8_t )( ( freq >> 24 ) & 0xFF );
    buf[1] = ( uint8_t )( ( freq >> 16 ) & 0xFF );
    buf[2] = ( uint8_t )( ( freq >> 8 ) & 0xFF );
    buf[3] = ( uint8_t )( freq & 0xFF );
    SX126xHal_WriteCommand( RADIO_SET_RFFREQUENCY, buf, 4 );
}


RadioPacketTypes_t SX126x_GetPacketType( void )
{
    return PacketType;
}

void SX126x_SetTxParams( int8_t power, RadioRampTimes_t rampTime )
{
    uint8_t buf[2];


    if( SX1261 )
    {
        if( power == 15 )
        {
            SX126x_SetPaConfig( 0x06, 0x00, 0x01, 0x01 );
        }
        else
        {
            SX126x_SetPaConfig( 0x04, 0x00, 0x01, 0x01 );  
        }
        if( power >= 14 )
        {
            power = 14;
        }
        else if( power < -3 )
        {
            power = -3;
        }
        SX126xHal_WriteReg( REG_OCP, (uint8_t *) 0x18 ); // current max is 80 mA for the whole device
    }
    else // sx1262 or sx1268
    {
        SX126x_SetPaConfig( 0x04, 0x07, 0x00, 0x01 );
        if( power > 22 )
        {
            power = 22;
        }
        else if( power < -3 )
        {
            power = -3;
        }
        SX126xHal_WriteReg( REG_OCP, (uint8_t *) 0x38 ); // current max 160mA for the whole device
    }
    buf[0] = power;
    if( XTAL == 0 )
    {
        if( ( uint8_t )rampTime < RADIO_RAMP_200_US )
        {
            buf[1] = RADIO_RAMP_200_US;
        }
        else
        {
            buf[1] = ( uint8_t )rampTime;
        }
    }
    else
    {
        buf[1] = ( uint8_t )rampTime;
    }
    SX126xHal_WriteCommand( RADIO_SET_TXPARAMS, buf, 2 );
}

void SX126x_SetModulationParams( ModulationParams_t *modulationParams )
{
    uint8_t n;
    uint32_t tempVal = 0;
    uint8_t buf[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


    // Check if required configuration corresponds to the stored packet type
    // If not, silently update radio packet type
    if( PacketType != modulationParams->PacketType )
    {
        SX126x_SetPacketType( modulationParams->PacketType );
    }

    switch( modulationParams->PacketType )
    {
    case PACKET_TYPE_GFSK:
        n = 8;
        tempVal = ( uint32_t )( 32 * ( ( double )XTAL_FREQ / ( double )modulationParams->Params.Gfsk.BitRate ) );
        buf[0] = ( tempVal >> 16 ) & 0xFF;
        buf[1] = ( tempVal >> 8 ) & 0xFF;
        buf[2] = tempVal & 0xFF;
        buf[3] = modulationParams->Params.Gfsk.ModulationShaping;
        buf[4] = modulationParams->Params.Gfsk.Bandwidth;
        tempVal = ( uint32_t )( ( double )modulationParams->Params.Gfsk.Fdev / ( double )FREQ_STEP );
        buf[5] = ( tempVal >> 16 ) & 0xFF;
        buf[6] = ( tempVal >> 8 ) & 0xFF;
        buf[7] = ( tempVal& 0xFF );
        break;
    case PACKET_TYPE_LORA:
        n = 4;
        switch( modulationParams->Params.LoRa.Bandwidth )
        {
            case LORA_BW_500:
                 modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                break;
            case LORA_BW_250:
                if( modulationParams->Params.LoRa.SpreadingFactor == 12 )
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                }
                else
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                }
                break;
            case LORA_BW_125:
                if( modulationParams->Params.LoRa.SpreadingFactor >= 11 )
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                }
                else
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                }
                break;
            case LORA_BW_062:
                if( modulationParams->Params.LoRa.SpreadingFactor >= 10 )
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                }
                else
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                }
                break;
            case LORA_BW_041:
                if( modulationParams->Params.LoRa.SpreadingFactor >= 9 )
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                }
                else
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                }
                break;
            case LORA_BW_031:
            case LORA_BW_020:
            case LORA_BW_015:
            case LORA_BW_010:
            case LORA_BW_007:
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                break;
            default:
                break;
        }
        buf[0] = modulationParams->Params.LoRa.SpreadingFactor;
        buf[1] = modulationParams->Params.LoRa.Bandwidth;
        buf[2] = modulationParams->Params.LoRa.CodingRate;
        buf[3] = modulationParams->Params.LoRa.LowDatarateOptimize;
        break;
    default:
    case PACKET_TYPE_NONE:
        return;
    }
    SX126xHal_WriteCommand( RADIO_SET_MODULATIONPARAMS, buf, n );
}

void SX126x_SetPacketParams( PacketParams_t *packetParams )
{
    uint8_t n;
    uint8_t crcVal = 0;
    uint8_t buf[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


    // Check if required configuration corresponds to the stored packet type
    // If not, silently update radio packet type
    if( PacketType != packetParams->PacketType )
    {
        SX126x_SetPacketType( packetParams->PacketType );
    }

    switch( packetParams->PacketType )
    {
    case PACKET_TYPE_GFSK:
        if( packetParams->Params.Gfsk.CrcLength == RADIO_CRC_2_BYTES_IBM )
        {
            SX126x_SetCrcSeed( CRC_IBM_SEED );
            SX126x_SetCrcPolynomial( CRC_POLYNOMIAL_IBM );
            crcVal = RADIO_CRC_2_BYTES;
        }
        else if(  packetParams->Params.Gfsk.CrcLength == RADIO_CRC_2_BYTES_CCIT )
        {
            SX126x_SetCrcSeed( CRC_CCITT_SEED );
            SX126x_SetCrcPolynomial( CRC_POLYNOMIAL_CCITT );
            crcVal = RADIO_CRC_2_BYTES_INV;
        }
        else
        {
            crcVal = packetParams->Params.Gfsk.CrcLength;
        }
        n = 9;
        // convert preamble length from byte to bit
        packetParams->Params.Gfsk.PreambleLength = packetParams->Params.Gfsk.PreambleLength << 3;

        buf[0] = ( packetParams->Params.Gfsk.PreambleLength >> 8 ) & 0xFF;
        buf[1] = packetParams->Params.Gfsk.PreambleLength;
        buf[2] = packetParams->Params.Gfsk.PreambleMinDetect;
        buf[3] = ( packetParams->Params.Gfsk.SyncWordLength << 3 ); // convert from byte to bit
        buf[4] = packetParams->Params.Gfsk.AddrComp;
        buf[5] = packetParams->Params.Gfsk.HeaderType;
        buf[6] = packetParams->Params.Gfsk.PayloadLength;
        buf[7] = crcVal;
        buf[8] = packetParams->Params.Gfsk.DcFree;
        break;
    case PACKET_TYPE_LORA:
        n = 6;
        buf[0] = ( packetParams->Params.LoRa.PreambleLength >> 8 ) & 0xFF;
        buf[1] = packetParams->Params.LoRa.PreambleLength;
        buf[2] = packetParams->Params.LoRa.HeaderType;
        buf[3] = packetParams->Params.LoRa.PayloadLength;
        buf[4] = packetParams->Params.LoRa.CrcMode;
        buf[5] = packetParams->Params.LoRa.InvertIQ;
        break;
    default:
    case PACKET_TYPE_NONE:
        return;
    }
    SX126xHal_WriteCommand( RADIO_SET_PACKETPARAMS, buf, n );
}

void SX126x_SetCadParams( RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, RadioCadExitModes_t cadExitMode, uint32_t cadTimeout )
{
    uint8_t buf[7];

    buf[0] = ( uint8_t )cadSymbolNum;
    buf[1] = cadDetPeak;
    buf[2] = cadDetMin;
    buf[3] = ( uint8_t )cadExitMode;
    buf[4] = ( uint8_t )( ( cadTimeout >> 16 ) & 0xFF );
    buf[5] = ( uint8_t )( ( cadTimeout >> 8 ) & 0xFF );
    buf[6] = ( uint8_t )( cadTimeout & 0xFF );
    SX126xHal_WriteCommand( RADIO_SET_CADPARAMS, buf, 7 );
    OperatingMode = MODE_CAD;
}

void SX126x_SetBufferBaseAddresses( uint8_t txBaseAddress, uint8_t rxBaseAddress )
{
    uint8_t buf[2];


    buf[0] = txBaseAddress;
    buf[1] = rxBaseAddress;
    SX126xHal_WriteCommand( RADIO_SET_BUFFERBASEADDRESS, buf, 2 );
}

RadioStatus_t SX126x_GetStatus( void )
{
    uint8_t stat = 0;
    RadioStatus_t status;

    SX126xHal_ReadCommand( RADIO_GET_STATUS, ( uint8_t * )&stat, 1 );
    status.Value = stat;
    return status;
}

int8_t SX126x_GetRssiInst( void )
{
    uint8_t rssi;

    SX126xHal_ReadCommand( RADIO_GET_RSSIINST, ( uint8_t* )&rssi, 1 );
    return( -( rssi / 2 ) );
}

void SX126x_GetRxBufferStatus( uint8_t *payloadLength, uint8_t *rxStartBufferPointer )
{
    uint8_t status[2];

    SX126xHal_ReadCommand( RADIO_GET_RXBUFFERSTATUS, status, 2 );
	
    /* The registers in this part of code are not in the datasheet*/
	
    // In case of LORA fixed header, the payloadLength is obtained by reading
    // the register REG_LR_PAYLOADLENGTH
    uint8_t buffer_stat_temp = 0;
	SX126xHal_ReadReg( REG_LR_PACKETPARAMS,  &buffer_stat_temp);
    if( ( SX126x_GetPacketType( ) == PACKET_TYPE_LORA ) && ( buffer_stat_temp >> 7 == 1 ) )
    {
        SX126xHal_ReadReg( REG_LR_PAYLOADLENGTH, payloadLength );
    }
    else
    {
        *payloadLength = status[0];
    }

    //*payloadLength = status[0];
    *rxStartBufferPointer = status[1];
}

void SX126x_GetPacketStatus( PacketStatus_t *pktStatus )
{
    uint8_t status[3];

    SX126xHal_ReadCommand( RADIO_GET_PACKETSTATUS, status, 3 );

    pktStatus->packetType = SX126x_GetPacketType( );
    switch( pktStatus->packetType )
    {
        case PACKET_TYPE_GFSK:
            pktStatus->Params.Gfsk.RxStatus = status[0];
            pktStatus->Params.Gfsk.RssiSync = -status[1] / 2;
            pktStatus->Params.Gfsk.RssiAvg = -status[2] / 2;
            pktStatus->Params.Gfsk.FreqError = 0;
            break;

        case PACKET_TYPE_LORA:
            pktStatus->Params.LoRa.RssiPkt = -status[0] / 2;
            ( status[1] < 128 ) ? ( pktStatus->Params.LoRa.SnrPkt = status[1] / 4 ) : ( pktStatus->Params.LoRa.SnrPkt = ( ( status[1] - 256 ) /4 ) );
            pktStatus->Params.LoRa.SignalRssiPkt = -status[2] / 2;
            pktStatus->Params.LoRa.FreqError = FrequencyError;
            break;

        default:
        case PACKET_TYPE_NONE:
            // In that specific case, we set everything in the pktStatus to zeros
            // and reset the packet type accordingly
            memset( pktStatus, 0, sizeof( PacketStatus_t ) );
            pktStatus->packetType = PACKET_TYPE_NONE;
            break;
    }
}

RadioError_t SX126x_GetDeviceErrors( void )
{
    RadioError_t error;

    SX126xHal_ReadCommand( RADIO_GET_ERROR, ( uint8_t * )&error, 2 );
    return error;
}

void SX126x_ClearIrqStatus( uint16_t irq )
{
    uint8_t buf[2];
    buf[0] = ( uint8_t )( ( ( uint16_t )irq >> 8 ) & 0x00FF );
    buf[1] = ( uint8_t )( ( uint16_t )irq & 0x00FF );
    SX126xHal_WriteCommand( RADIO_CLR_IRQSTATUS, buf, 2 );
}

void SX126x_ProcessIrqs( void )
{
    uint16_t irqRegs = SX126x_GetIrqStatus( );
    SX126x_ClearIrqStatus( IRQ_RADIO_ALL );


    if( ( irqRegs & IRQ_HEADER_VALID ) == IRQ_HEADER_VALID )
    {
		uint8_t irq_temp;
        // LoRa Only
		SX126xHal_ReadReg( REG_FREQUENCY_ERRORBASEADDR, &irq_temp );
        FrequencyError = 0x000000 | ( ( 0x0F & irq_temp ) << 16 );
		SX126xHal_ReadReg( REG_FREQUENCY_ERRORBASEADDR + 1, &irq_temp );
        FrequencyError = FrequencyError | ( irq_temp << 8 );
		SX126xHal_ReadReg( REG_FREQUENCY_ERRORBASEADDR + 2, &irq_temp );
        FrequencyError = FrequencyError | ( irq_temp );
    }

    if( ( irqRegs & IRQ_TX_DONE ) == IRQ_TX_DONE )
    {
       // Do something: Tx done
    }

    if( ( irqRegs & IRQ_RX_DONE ) == IRQ_RX_DONE )
    {
        if( ( irqRegs & IRQ_CRC_ERROR ) == IRQ_CRC_ERROR )
        {
            // Do something: Rx error
        }
        else
        {
           // Do something: Rx succesful
        }
    }

    if( ( irqRegs & IRQ_CAD_DONE ) == IRQ_CAD_DONE )
    {
        // Do something: cad done
    }

    // Timeout IRQ
    if( ( irqRegs & IRQ_RX_TX_TIMEOUT ) == IRQ_RX_TX_TIMEOUT )
    {
        if( OperatingMode == MODE_TX )
        {
            // Do something: Tx timeout;
        }
        else if( OperatingMode == MODE_RX )
        {
            // Do something: Rx timeout;
        }
        else
        {
            //Fail
        }
    }
    
/*
    //IRQ_PREAMBLE_DETECTED                   = 0x0004,
    if( irqRegs & IRQ_PREAMBLE_DETECTED )
    {
        if( rxPblSyncWordHeader != NULL )
        {
            rxPblSyncWordHeader( IRQ_PBL_DETECT_CODE);
            
        }
    }

    //IRQ_SYNCWORD_VALID                      = 0x0008,
    if( irqRegs & IRQ_SYNCWORD_VALID )
    {
        if( rxPblSyncWordHeader != NULL )
        {
            rxPblSyncWordHeader( IRQ_SYNCWORD_VALID_CODE  );
        }
    }

    //IRQ_HEADER_VALID                        = 0x0010,
    if ( irqRegs & IRQ_HEADER_VALID ) 
    {
        if( rxPblSyncWordHeader != NULL )
        {
            rxPblSyncWordHeader( IRQ_HEADER_VALID_CODE );
        }
    } 

    //IRQ_HEADER_ERROR                        = 0x0020,
    if( irqRegs & IRQ_HEADER_ERROR )
    {
        if( )
        {
                        // Do something: Header error;
        }
    }  
    */

}

// HELPER FUNCTIONS TO START TX AND RX

void set_rx( uint32_t freq, RadioLoRaBandwidths_t bw, RadioLoRaSpreadingFactors_t sf, RadioLoRaCodingRates_t cd, RadioLoRaPacketLengthsMode_t ht, uint8_t pck_len ){
    SX126x_SetPacketType(PACKET_TYPE_LORA);

    SX126x_SetRfFrequency(freq);

    SX126x_SetBufferBaseAddresses(10, 10);


    ModulationParams_t ModParams;
    ModParams.PacketType = PACKET_TYPE_LORA;

    struct LoRa_modul lora_modul;
    lora_modul.SpreadingFactor = sf;
    lora_modul.Bandwidth = bw;
    lora_modul.CodingRate = cd;
    lora_modul.LowDatarateOptimize = 0;

    struct Params_modul params_modul;

    memcpy(&params_modul.LoRa, &lora_modul, sizeof(lora_modul));

    memcpy(&ModParams.Params, &params_modul, sizeof(params_modul));

    SX126x_SetModulationParams(&ModParams);


    PacketParams_t PckParam;
    PckParam.PacketType = PACKET_TYPE_LORA;
    struct Params_pckt param_pckt;
    struct LoRa_pckt lora_pckt;

    lora_pckt.PreambleLength = 8;
    lora_pckt.HeaderType = LORA_PACKET_VARIABLE_LENGTH;
    lora_pckt.PayloadLength = pck_len;
    lora_pckt.CrcMode = LORA_CRC_OFF;
    lora_pckt.InvertIQ = LORA_IQ_NORMAL;

    memcpy(&param_pckt.LoRa, &lora_pckt, sizeof(lora_pckt));
    memcpy(&PckParam.Params, &param_pckt, sizeof(param_pckt));

    SX126x_SetPacketParams(&PckParam);
}

void set_tx( uint32_t freq, RadioLoRaBandwidths_t bw, RadioLoRaSpreadingFactors_t sf, RadioLoRaCodingRates_t cd, RadioLoRaPacketLengthsMode_t ht, uint8_t pck_len, int8_t power, RadioRampTimes_t rt ){
    // Same considerations as RX
    set_rx( freq, bw, sf, cd, ht, pck_len );
    // Plus some specific TX
    SX126x_SetPaConfig( 0x04, 0x07, 0x00, 0x01 );
    SX126x_SetTxParams(power, rt);
}
