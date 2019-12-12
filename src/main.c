/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include <stdio.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32f7xx.h"
#include "stm32746g_discovery.h"
#include "DriversHW/inc/VCP.h"
#include "printf.h"
#include "lcdTask.h"
#include "sdramTask.h"
#include "debugTask.h"
#include "cameraTask.h"
#include "DriversHW/inc/gpio.h"
#include "DriversHW/inc/UART6.h"
#include "stm32746g_discovery_ts.h"


SemaphoreHandle_t xSemaphoreSynchroCameraTask=NULL;
QueueHandle_t xQueueDebugCam=NULL;
xTaskHandle xHandleLCD=NULL;
xTaskHandle xHandleSDRAM=NULL;
xTaskHandle xHandleCAMERA=NULL;
xTaskHandle xHandleDEBUG=NULL;
uint32_t taskTime=0;


void tasksCreate(void);
static void SystemClock_Config(void);
uint32_t sysClk=0;

void startTaskTime(void)
{
	taskTime=HAL_GetTick();
}

uint32_t getTaskLive(void)
{
	return HAL_GetTick() - taskTime;
}

int main(void)
{
  uint8_t i=0;
  HAL_Init();
  SystemClock_Config();
  VCP_gpioInit();
  uart6Init();
  gpioLedInit();
  sysClk=HAL_RCC_GetSysClockFreq();
  for(i=0;i<5;i++)
  {
    HAL_Delay(1000);
    toggleLed();
  }

  tasksCreate();
  vTaskStartScheduler();
  while(1);
}

void tasksCreate(void)
{
  BaseType_t xReturned;
  xSemaphoreSynchroCameraTask = xSemaphoreCreateBinary();
  xQueueDebugCam = xQueueCreate( 1, sizeof( uint32_t ) );
  xReturned = xTaskCreate( vTaskDEBUG, "DEBUG_TASK",  1024,  NULL, 4, &xHandleDEBUG );
  xReturned = xTaskCreate( vTaskSDRAM, "SDRAM_TASK",  512,  NULL, 3, &xHandleSDRAM );
  xReturned = xTaskCreate( vTaskLCD, "LCD_TASK",  1024,  xHandleCAMERA, 3, &xHandleLCD );
  xReturned = xTaskCreate( vTaskCAMERA, "CAMERA_TASK",  512,  NULL, 3, &xHandleCAMERA );
}

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){}


  if (HAL_PWREx_EnableOverDrive() != HAL_OK){}

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK){}

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {}

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
