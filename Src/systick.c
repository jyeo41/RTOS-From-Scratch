#include <stdint.h>
#include "stm32f407xx.h"
#include "systick.h"
#include "kernel.h"
#include "led.h"

#define SYSTEM_CLOCK 		16000000	/* 16 MHz */
#define TRIGGER_EVERY_MS	(SYSTEM_CLOCK / 1000U)

static uint32_t get_tick_counter(void);

uint32_t tick_counter_global;

void systick_initialize(void)
{
	/* Disable Systick module during configuration */
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

	/* Set Clocksource to internal 16 MHz clock */
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	/* Set the Reload value to trigger the Systick handler every 1 ms */
	SysTick->LOAD = TRIGGER_EVERY_MS - 1U;

	/* Clear the value in CURRENT register */
	SysTick->VAL = 0;

	/* Enable Interrupts */
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

	/* Enable Systick module */
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
	tick_counter_global++;

	/* Remember the scheduler needs to be called inside of a critical section to avoid race conditions */
	__disable_irq();
	kernel_scheduler_round_robin();
	__enable_irq();
}

void systick_delay_ms(uint32_t delay)
{
	/* Start variable is used as the starting reference point to calculate the correct delay.
	 * The current tick_counter is repeatedly checked minus "start" variable to know when the delay
	 * 	has finished.
	 */
	uint32_t start = get_tick_counter();
	while (get_tick_counter() - start <= delay){}
}


static uint32_t get_tick_counter(void)
{
	uint32_t tick_counter_local;
	/* Return the tick counter variable inside of a critical section.
	 * This is so the systick interrupt cannot preempt this function and modify the value while it is being read.
	 */
	__disable_irq();
	tick_counter_local = tick_counter_global;
	__enable_irq();

	return tick_counter_local;
}
