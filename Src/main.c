#include "stm32f407xx.h"
#include "led.h"
#include "systick.h"
#include "kernel.h"

uint32_t blinky1_stack[40];
tcb_type blinky1;
void main_blinky1(void)
{

}

uint32_t blinky2_stack[40];
tcb_type blinky2;
void main_blinky2(void)
{

}

int main (void)
{
	/* Basic Startup Config Build */
	led_initialize();
	systick_initialize();
	kernel_initialize();

	kernel_tcb_start(
		&blinky1,
		&main_blinky1,
		blinky1_stack,
		sizeof(blinky1_stack));

	kernel_tcb_start(
		&blinky2,
		&main_blinky2,
		blinky2_stack,
		sizeof(blinky2_stack));

	while (1) {
		led_red_toggle();
		systick_delay_ms(1000);
	}
}
