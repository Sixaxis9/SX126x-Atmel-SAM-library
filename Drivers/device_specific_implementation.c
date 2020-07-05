/*
__/\\\\____________/\\\\_____/\\\\\\\\\\\\_        
 _\/\\\\\\________/\\\\\\___/\\\//////////__       
  _\/\\\//\\\____/\\\//\\\__/\\\_____________      
   _\/\\\\///\\\/\\\/_\/\\\_\/\\\____/\\\\\\\_     
    _\/\\\__\///\\\/___\/\\\_\/\\\___\/////\\\_    
     _\/\\\____\///_____\/\\\_\/\\\_______\/\\\_   
      _\/\\\_____________\/\\\_\/\\\_______\/\\\_  
       _\/\\\_____________\/\\\_\//\\\\\\\\\\\\/__ 
        _\///______________\///___\////////////____

Author: Marco Giordano
*/

// Pin assignemnts
// Sobstitute here the your specific pin assignment

#include "device_specific_implementation.h"
#include <hpl_spi.h>

// Device-specific implementations
// Sobstitute here the functions related to your specific microcontroller

extern struct spi_m_sync_descriptor SPI_0;
struct io_descriptor *spi;

uint8_t read_pin(const uint8_t pin){
    return gpio_get_pin_level(pin);
}

void write_pin(const uint8_t pin, const uint8_t status){
    gpio_set_pin_level(pin, status);
}

void SPI_init(void)
{
	spi_m_sync_get_io_descriptor(&SPI_0, &spi);
	spi_m_sync_enable(&SPI_0);
}

void SendSpi(uint8_t *data, uint8_t len){	
		io_write(spi, data, len);
}

void *ReadSpi(uint8_t *rx_data, uint8_t len){	
    struct spi_xfer *temp;

    uint8_t noop[len];
    for(int i=0; i<len; i++){
        noop[i] = 0x00; // Fill with noop, 0x00
    }

    temp->txbuf = noop;
    temp->rxbuf = rx_data;
    temp->size  = len;
    spi_m_sync_transfer(spi, temp);
}

void IRQ_Init(void)
{
	ext_irq_register(PIN_PC00, DIO1_IRQ);
    // Possibility to add DIO2 and DIO3 interrupts
}

static void DIO1_IRQ(void)
{
}