#include "stm32f407xx.h"
#include "led.h"
#include "systick.h"
#include "kernel.h"

uint32_t blinky1_stack[40];
tcb_type blinky1;
void main_blinky1(void)
{
	while (1) {
		led_red_toggle();
		systick_delay_ms(1500);
	}
}

uint32_t blinky2_stack[40];
tcb_type blinky2;
void main_blinky2(void)
{
	while (1) {
		led_orange_toggle();
		systick_delay_ms(700);
	}
}

uint32_t blinky3_stack[40];
tcb_type blinky3;
void main_blinky3(void)
{
	while (1) {
		led_blue_toggle();
		systick_delay_ms(300);
	}
}

int main (void)
{
	/* Basic Startup Config Build */
	led_initialize();
	systick_initialize();

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

	kernel_tcb_start(
		&blinky3,
		&main_blinky3,
		blinky3_stack,
		sizeof(blinky3_stack));

	/* This start function replaces the redundant superloop */
	kernel_start();
}
