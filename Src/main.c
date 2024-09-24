#include "stm32f407xx.h"
#include "led.h"
#include "systick.h"
#include "kernel.h"

uint32_t blinky1_stack[40];
tcb_type blinky1;
void main_blinky1(void)
{
	while (1) {
		uint32_t volatile i;
		for (i = 0U; i < 1500U; i++) {
			led_red_toggle();
			led_red_toggle();
		}
		kernel_tcb_block(20U);	/* Block for 1 tick, 1ms */
	}
}

uint32_t blinky2_stack[40];
tcb_type blinky2;
void main_blinky2(void)
{
	while (1) {
		uint32_t volatile i;
		for (i = 0U; i < 3 * 1500U; i++) {
			led_orange_toggle();
			led_orange_toggle();
		}
		kernel_tcb_block(50U);	 /* Block for 50 ticks, 50ms */

	}
}

/*uint32_t blinky3_stack[40];
tcb_type blinky3;
void main_blinky3(void)
{
	while (1) {
		led_blue_toggle();
		kernel_tcb_block(500);
		led_blue_toggle();
		kernel_tcb_block(600);
	}
}*/

int main (void)
{
	/* Initialize interrupt priorities and the idle thread for efficient blocking */
	kernel_initialize();

	/* Basic Startup Config Build */
	led_initialize();
	systick_initialize();

	kernel_tcb_start(
		&blinky1,
		5U,
		&main_blinky1,
		blinky1_stack,
		sizeof(blinky1_stack));

	kernel_tcb_start(
		&blinky2,
		2U,
		&main_blinky2,
		blinky2_stack,
		sizeof(blinky2_stack));

/*	kernel_tcb_start(
		&blinky3,
		&main_blinky3,
		blinky3_stack,
		sizeof(blinky3_stack));*/

	/* This start function replaces the redundant superloop */
	kernel_run();
}
