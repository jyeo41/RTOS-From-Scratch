#include "stm32f407xx.h"
#include "led.h"
#include "systick.h"
#include "kernel.h"

uint32_t blinky1_stack[40];
tcb_type blinky1;
void main_blinky1(void)
{
	while (1) {
		uint32_t i;
		for (i = 0; i < 100000; i++) {
			led_red_toggle();
		}
		kernel_tcb_block(1500U);
	}
}

uint32_t blinky2_stack[40];
tcb_type blinky2;
void main_blinky2(void)
{
	while (1) {
		uint32_t i;
		for (i = 0; i < 220000; i++) {
			led_orange_toggle();
		}
		kernel_tcb_block(4700U);
	}
}

uint32_t blinky3_stack[40];
tcb_type blinky3;
void main_blinky3(void)
{
	while (1) {
		uint32_t i;
		for (i = 0; i < 500000; i++) {
			led_blue_toggle();
		}
		kernel_tcb_block(8200U);
	}
}

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

	kernel_tcb_start(
		&blinky3,
		1U,
		&main_blinky3,
		blinky3_stack,
		sizeof(blinky3_stack));

	/* This start function replaces the redundant superloop */
	kernel_run();
}
