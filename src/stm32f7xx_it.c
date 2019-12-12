/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#include <cmsis_os.h>
#include "stm32f7xx_it.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/
extern SDRAM_HandleTypeDef sdramHandle;
extern DMA_HandleTypeDef hdmadcmi;
extern DCMI_HandleTypeDef hdcmi;
extern UART_HandleTypeDef uart6h;
extern DMA_HandleTypeDef DMA2_usart6TX;
extern DMA_HandleTypeDef DMA2_usart6RX;
extern UART_HandleTypeDef VCP_usart1;
extern DMA_HandleTypeDef DMA2_usart1TX;
extern DMA_HandleTypeDef DMA2_usart1RX;

/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
  osSystickHandler();
}


void FMC_IRQHandler(void)
{
  HAL_SDRAM_IRQHandler(&sdramHandle);
}

void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(sdramHandle.hdma);
}

void DMA2_Stream1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdmadcmi);
}

void DCMI_IRQHandler(void)
{
  HAL_DCMI_IRQHandler(&hdcmi);
}

void USART6_IRQHandler(void)
{
    HAL_UART_IRQHandler(&uart6h);
}

void DMA2_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&DMA2_usart6RX);
}

void DMA2_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&DMA2_usart6TX);
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&VCP_usart1);
}

void DMA2_Stream7_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&DMA2_usart1TX);
}

void DMA2_Stream5_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&DMA2_usart1RX);
}
