#include "stm32f7xx.h"
#include "stm32746g_discovery.h"
#include "DriversHW/inc/gpio.h"

GPIO_InitTypeDef gpioLed;

void gpioLedInit(void)
{

	__HAL_RCC_GPIOI_CLK_ENABLE();

	gpioLed.Pin=GPIO_PIN_1;
	gpioLed.Mode=GPIO_MODE_OUTPUT_PP;
	gpioLed.Pull=GPIO_NOPULL;
	gpioLed.Speed=GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOI, &gpioLed);
}

void toggleLed(void)
{
	HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_1);
}