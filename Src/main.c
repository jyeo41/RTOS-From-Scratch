#include "stm32f407xx.h"
#include "led.h"
#include "systick.h"

int main (void)
{
	/* Basic Startup Config Build */
	led_initialize();
	systick_initialize();

	while (1) {
		led_red_toggle();
		systick_delay_ms(1000);
	}
}
