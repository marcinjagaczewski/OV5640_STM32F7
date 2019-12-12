#include <stdio.h>
#include "stm32746g_discovery_sdram.h"
#include "sdramTask.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "printf.h"



void vTaskSDRAM( void * pvParameters )
{
  // uint8_t initLCD=0, sdramLoad=0;
  uint32_t sdramAddress=SDRAM_DEVICE_ADDR;

  BSP_SDRAM_Init();
  for(sdramAddress=SDRAM_DEVICE_ADDR; sdramAddress<SDRAM_DEVICE_ADDR+SDRAM_DEVICE_SIZE; sdramAddress+=4) { *(uint32_t *)sdramAddress=0xFFFFFFFF; }
  vTaskSuspend(NULL);

  
  while(1)
  {
  }
}
