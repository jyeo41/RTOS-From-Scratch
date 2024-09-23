#include "stm32f407xx.h"
#include "led.h"

int main (void)
{
	/* Basic Startup Config Build */
	led_initialize();
	int i;

	while (1) {
		led_orange_toggle();
		for (i = 0; i < 100000; i++){}
	}
}
