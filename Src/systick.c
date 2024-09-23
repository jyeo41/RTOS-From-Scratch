#include "stm32f407xx.h"
#include "systick.h"
#include "led.h"

#define SYSTEM_CLOCK 		16000000	/* 16 MHz */
#define TRIGGER_EVERY_MS	(SYSTEM_CLOCK / 1000U)

uint32_t tick_counter;

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
	led_orange_toggle();
}

void systick_delay_ms(uint32_t delay);


/*static uint32_t get_tick_counter(void);*/
