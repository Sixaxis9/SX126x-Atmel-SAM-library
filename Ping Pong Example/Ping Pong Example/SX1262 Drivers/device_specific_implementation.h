#include <stdint.h>

#include <hal_gpio.h>
#include <hal_ext_irq.h>
#include <hal_spi_m_sync.h>
#include <hal_atomic.h>

#include <atmel_start_pins.h>

#define USE_CONFIG_PUBLIC_NETOWRK 0
#define XTAL 1
#define ADV_DEBUG 0
#define USE_CONFIG_PUBLIC_NETOWRK 0

#define SX1261 0
#define SX1262 1
#define SX1268 0

//volatile hal_atomic_t __atomic;
//#define CRITICAL_SECTION_ENTER atomic_enter_critical(&__atomic)
//#define CRITICAL_SECTION_LEAVE atomic_leave_critical(&__atomic)
	
#define DIO2 DIO1
#define DIO3 DIO1
/*
#define DIO1 1
#define BUSY busy
#define NSS NSS
#define RADIO_RESET rst_lora
*/

uint8_t read_pin(const uint8_t pin);

void write_pin(const uint8_t pin, const uint8_t status);

int32_t SPI_init(void);

int32_t SendSpi(uint8_t *data, uint8_t len);

int32_t ReadSpi(uint8_t *rx_data, uint8_t len);

void IRQ_Init(void);// Possibility to add DIO2 and DIO3 interrupts

void DIO1_IRQ(void);

// Some macro definitions

#define wait_ms delay_ms

#define RESET_ON write_pin(RST, false);
#define RESET_OFF write_pin(RST, true);

#define WAIT_BUSY while(read_pin(BUSY)){}

#define NSS_ON write_pin(NSS, false);
#define NSS_OFF write_pin(NSS, true);