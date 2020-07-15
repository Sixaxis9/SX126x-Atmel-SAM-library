#include <atmel_start.h>

#include "./SX1262 Drivers/sx126x_commands.h"
#include "./SX1262 Drivers/sx126x_hal.h"

extern struct usart_sync_descriptor USART_0;
struct io_descriptor *usart;

extern struct timer_descriptor TIMER_0;
struct timer_task TIMER_0_task1;
static void TIMER_0_task1_cb(const struct timer_task *const timer_task);

void USART_init(void);
void Timer_init(void);

uint8_t welcome_USART[13] = "Hello World!\n";
RadioCommands_t commands;


int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	USART_init();
	Timer_init();
	
	io_write(usart, welcome_USART, 13);

	SX126x_Init();

	// SET THIS FOR THE RX
	set_rx(868000000, LORA_BW_500, LORA_SF7, LORA_CR_4_5, LORA_PACKET_VARIABLE_LENGTH, 0x20);
    SX126x_SetDioIrqParams(2, 2, 0, 0);
	SX126x_SetRx(0);
	
	//SET THE FOLLING FOR THE TX
	//set_tx(868000000, LORA_BW_500, LORA_SF7, LORA_CR_4_5, LORA_PACKET_VARIABLE_LENGTH, 0x04, 14, RADIO_RAMP_200_US);


	while (1) {
	//SET THE FOLLING FOR THE TX
	//uint8_t payload[4] = {'P', 'O', 'N', 'G'};
	//SX126x_SendPayload(payload, 4, 0); // Be careful timeout
	//gpio_toggle_pin_level(LED);
	
	//SX126x_GetStatus();
	
	//delay_ms(500);
	
	}
}

void USART_init(void){
	usart_sync_get_io_descriptor(&USART_0, &usart);
	usart_sync_enable(&USART_0);
}

void Timer_init(void){
	TIMER_0_task1.interval = 100;
	TIMER_0_task1.cb = TIMER_0_task1_cb;
	TIMER_0_task1.mode = TIMER_TASK_REPEAT;
	timer_add_task(&TIMER_0, &TIMER_0_task1);
	timer_start(&TIMER_0);
}

static void TIMER_0_task1_cb(const struct timer_task *const timer_task)
{
	//io_write(usart, welcome_USART, 13);
}