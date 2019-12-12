#include <stdio.h>
#include "lcdTask.h"
#include "main.h"
#include "stm32f7xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"


/* Task to be created. */
void vTaskLCD( void * pvParameters )
{
	uint16_t x=0;
	uint32_t mem = SDRAM_DEVICE_ADDR;
  TS_StateTypeDef touchScreen;


  BSP_LCD_Init();
  BSP_LCD_LayerRgb565Init(1, SDRAM_DEVICE_ADDR);


  for(x=0; x<480; x++)
  {
 		*(uint16_t*)(mem+(x*2))=0x00FF;
  }

  vTaskSuspend(NULL);
  

  while(1)
  {
  }
}

