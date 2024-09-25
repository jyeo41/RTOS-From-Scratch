#include "stm32f407xx.h"
#include "led.h"

/* User Manual UM1472 states:
 * User Green LED:	I/O PD12
 * User Orange LED: I/O PD13
 * User Red LED:	I/O PD14
 * User Blue LED:	I/O PD15
 */

void led_initialize(void)
{
	/* Need to enable the clock connected to GPIO Port D */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

	/* Set Direction bit LED pins to output pin.
	 * First clear the bit-field.
	 * Then set the bits to 0b01 for GPIO mode
	 */

	/* Output Direction for Green LED */
	GPIOD->MODER &= ~GPIO_MODER_MODER12_Msk;
	GPIOD->MODER |= GPIO_MODER_MODER12_0;

	/* Output Direction for Orange LED */
	GPIOD->MODER &= ~GPIO_MODER_MODER13_Msk;
	GPIOD->MODER |= GPIO_MODER_MODER13_0;

	/* Output Direction for Red LED */
	GPIOD->MODER &= ~GPIO_MODER_MODER14_Msk;
	GPIOD->MODER |= GPIO_MODER_MODER14_0;

	/* Output Direction for Blue LED */
	GPIOD->MODER &= ~GPIO_MODER_MODER15_Msk;
	GPIOD->MODER |= GPIO_MODER_MODER15_0;
}

/* PD 12 */
void led_green_toggle(void)
{
	if (GPIOD->ODR & GPIO_ODR_OD12) {
		GPIOD->BSRR = GPIO_BSRR_BR12;
	} else {
		GPIOD->BSRR = GPIO_BSRR_BS12;
	}
}

void led_green_off(void)
{
	if (GPIOD->ODR & GPIO_ODR_OD12) {
		GPIOD->BSRR = GPIO_BSRR_BR12;
	}
}

/* Orange LED PD 13 */
void led_orange_toggle(void)
{
	/* Reference Manual
	 * Page 287, Section 8.4.7
	 * Check if Orange LED is ON in the Data Register.
	 * If its set, then atomically clear the bit by writing 1 to corresponding bit in upper 16 BSRR register.
	 * 	The BR for Bit Reset definition is used.
	 * Else if the bit is not set, then atomically set it by writing a 1 to corresponding bit in lower 16 BSRR register.
	 * 	The BS for Bit Set definition is used.
	 */
	if (GPIOD->ODR & GPIO_ODR_OD13) {
		GPIOD->BSRR = GPIO_BSRR_BR13;
	} else {
		GPIOD->BSRR = GPIO_BSRR_BS13;
	}
}

void led_red_toggle(void)
{
	if (GPIOD->ODR & GPIO_ODR_OD14) {
		GPIOD->BSRR = GPIO_BSRR_BR14;
	} else {
		GPIOD->BSRR = GPIO_BSRR_BS14;
	}
}

void led_blue_off(void)
{
	if (GPIOD->ODR & GPIO_ODR_OD15) {
		GPIOD->BSRR = GPIO_BSRR_BR15;
	}
}

void led_blue_toggle(void)
{
	if (GPIOD->ODR & GPIO_ODR_OD15) {
		GPIOD->BSRR = GPIO_BSRR_BR15;
	} else {
		GPIOD->BSRR = GPIO_BSRR_BS15;
	}
}
