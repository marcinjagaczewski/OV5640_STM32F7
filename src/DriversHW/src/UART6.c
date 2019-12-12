#include <stdio.h>
#include "stm32f7xx.h"
#include "stm32f7xx_hal_def.h"
#include "DriversHW/inc/UART6.h"
#include "DriversHW/inc/VCP.h"


static void uart6GpioInit(void);
static void uart6DmaInit(void);

GPIO_InitTypeDef uartGpio;
UART_HandleTypeDef uart6h;
DMA_HandleTypeDef DMA2_usart6TX;
DMA_HandleTypeDef DMA2_usart6RX;
extern uint8_t rxChar;


static void uart6GpioInit(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();

	//RX
	uartGpio.Pin=GPIO_PIN_7;
	uartGpio.Mode=GPIO_MODE_AF_PP;
	uartGpio.Pull=GPIO_PULLUP;
	uartGpio.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	uartGpio.Alternate=GPIO_AF8_USART6;
	HAL_GPIO_Init(GPIOC, &uartGpio);
	//TX
	uartGpio.Pin=GPIO_PIN_6;
	uartGpio.Mode=GPIO_MODE_AF_PP;
	uartGpio.Pull=GPIO_PULLUP;
	uartGpio.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	uartGpio.Alternate=GPIO_AF8_USART6;
	HAL_GPIO_Init(GPIOC, &uartGpio);
}

static void uart6DmaInit(void)
{
	__HAL_RCC_DMA2_CLK_ENABLE();

	DMA2_usart6TX.Instance = DMA2_Stream6;
	DMA2_usart6TX.Init.Channel = DMA_CHANNEL_5;
	DMA2_usart6TX.Init.Direction = DMA_MEMORY_TO_PERIPH;
	DMA2_usart6TX.Init.PeriphInc = DMA_PINC_DISABLE;
	DMA2_usart6TX.Init.MemInc = DMA_MINC_ENABLE;
	DMA2_usart6TX.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	DMA2_usart6TX.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	DMA2_usart6TX.Init.Mode = DMA_NORMAL;
	DMA2_usart6TX.Init.Priority = DMA_PRIORITY_LOW;
	DMA2_usart6TX.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	DMA2_usart6TX.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;
	DMA2_usart6TX.Init.MemBurst=DMA_MBURST_SINGLE;
	DMA2_usart6TX.Init.PeriphBurst=DMA_PBURST_SINGLE;
	
	DMA2_usart6RX.Instance = DMA2_Stream2;
	DMA2_usart6RX.Init.Channel = DMA_CHANNEL_5;
	DMA2_usart6RX.Init.Direction = DMA_PERIPH_TO_MEMORY;
    DMA2_usart6RX.Init.PeriphInc = DMA_PINC_DISABLE;
    DMA2_usart6RX.Init.MemInc = DMA_MINC_DISABLE;
    DMA2_usart6RX.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    DMA2_usart6RX.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    DMA2_usart6RX.Init.Mode = DMA_NORMAL;
    DMA2_usart6RX.Init.Priority = DMA_PRIORITY_LOW;
    DMA2_usart6RX.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    DMA2_usart6RX.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;
    DMA2_usart6RX.Init.MemBurst=DMA_MBURST_SINGLE;
	DMA2_usart6RX.Init.PeriphBurst=DMA_PBURST_SINGLE;

	HAL_DMA_Init(&DMA2_usart6TX);
	__HAL_LINKDMA(&uart6h,hdmatx,DMA2_usart6TX);
	HAL_DMA_Init(&DMA2_usart6RX);
	__HAL_LINKDMA(&uart6h,hdmarx,DMA2_usart6RX);
}

void uart6Init(void)
{
	__HAL_RCC_USART6_CLK_ENABLE();
	uart6GpioInit();

	uart6h.Instance=USART6;
	uart6h.Init.BaudRate=1000000;
	uart6h.Init.WordLength=UART_WORDLENGTH_8B;
	uart6h.Init.StopBits=UART_STOPBITS_1;
	uart6h.Init.Parity=UART_PARITY_NONE;
	uart6h.Init.Mode=UART_MODE_TX_RX;
	uart6h.Init.HwFlowCtl=UART_HWCONTROL_NONE;
	uart6h.Init.OverSampling=UART_OVERSAMPLING_16;
	uart6h.Init.OneBitSampling=UART_ONE_BIT_SAMPLE_DISABLE;
	HAL_UART_Init(&uart6h);

	uart6DmaInit();

	__HAL_UART_ENABLE_IT(&uart6h,UART_IT_TC);
	__HAL_UART_ENABLE_IT(&uart6h,UART_IT_RXNE);

	HAL_NVIC_SetPriority(USART6_IRQn, 7, 0);
	HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 6, 0);
	HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 6, 0);

	HAL_NVIC_EnableIRQ(USART6_IRQn);
	HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
  	HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

  	while(!__HAL_UART_GET_FLAG(&uart6h, UART_FLAG_TC));
  	HAL_UART_Transmit_DMA(&uart6h, "ok\n\r", strlen("ok\n\r"));
  	while(!__HAL_UART_GET_FLAG(&uart6h, UART_FLAG_TC));
	HAL_UART_Transmit_DMA(&uart6h, "dziala\n\r", strlen("dziala\n\r"));
	

	HAL_UART_Receive_DMA(&uart6h, &rxChar, 1);
}
