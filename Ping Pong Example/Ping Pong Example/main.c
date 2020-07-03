#include <atmel_start.h>

extern struct usart_async_descriptor USART_0;
struct io_descriptor *usart;

extern struct timer_descriptor TIMER_0;
struct timer_task TIMER_0_task1;
static void TIMER_0_task1_cb(const struct timer_task *const timer_task);

static void tx_cb_USART_0(const struct usart_async_descriptor *const io_descr);

void USART_init(void);
void Timer_init(void);

uint8_t welome_USART[13] = "Hello World!\n";

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	USART_init();
	Timer_init();
 
	while (1) {
		//delay_ms(100);
	}
}

void USART_init(void){
	usart_async_register_callback(&USART_0, USART_ASYNC_TXC_CB, tx_cb_USART_0);
	//usart_async_register_callback(&USART_0, USART_ASYNC_RXC_CB, rx_cb);
	//usart_async_register_callback(&USART_0, USART_ASYNC_ERROR_CB, err_cb);
	usart_async_get_io_descriptor(&USART_0, &usart);
	usart_async_enable(&USART_0);
}

static void tx_cb_USART_0(const struct usart_async_descriptor *const io_descr){
	gpio_toggle_pin_level(LED);
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
	io_write(usart, welome_USART, 13);
}