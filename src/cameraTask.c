#include <stdio.h>
#include "main.h"
#include "DriversHW/inc/i2c.h"
#include "DriversHW/inc/dcmi.h"
#include "DriversHW/inc/dma2d.h"
#include "DriversHW/inc/lcdt.h"
#include "ov5640.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "printf.h"

extern QueueHandle_t xQueueDebugCam;
extern SemaphoreHandle_t xSemaphoreSynchroCameraTask;
uint32_t sdramAddress=0xC0000000;


static void rgbMode(void);
static void jpegMode(void);



void vTaskCAMERA( void * pvParameters )
{
	uint32_t queueData=0;
	uint16_t xPos=0;
	uint16_t yPos=0;
	bool status=true;
	uint8_t i=0;

	initI2C();
	dcmiInit();
	dma2dInit();
	
	rgbMode();

	// OV5640_changeCameraMode(JPEG);
	// jpegMode();

	while(1)
	{
        xQueueReceive(xQueueDebugCam, &queueData, portMAX_DELAY);
        switch((queueData>>24)&0x0F)
        {
        	//RGB Function:
        	case 0x01: // on/off autofocus window
        	{
        		queueData=0x01;
        		queueData<<=24;
        		if(OV5640_getAutoFocusWindowState() == 0x01)
        		{
        			OV5640_autoFocusWindowOnOff(false); 
        		}
        		else 
        		{
        			OV5640_autoFocusWindowOnOff(true); 
        		}

        		queueData |= OV5640_getAutoFocusWindowState();
        		xSemaphoreGive( xSemaphoreSynchroCameraTask );
        		xQueueSend( xQueueDebugCam, &queueData, 10 );
        		vTaskSuspend(NULL);
        		break;
        	}
        	case 0x02: // move autofocus window
        	{
        		OV5640_getAutoFocusWindowPosition(&xPos, &yPos);

        		if((queueData & 0x0F) == 0x01) // move right autofocus window
        		{
        			xPos+=2;
        		}
        		else if((queueData & 0x0F) == 0x02)// move left autofocus window
        		{
        			xPos-=2;
        		}
        		else if((queueData & 0x0F) == 0x03)// move down autofocus window
        		{
        			yPos+=2;
        		}
        		else if((queueData & 0x0F) == 0x04)// move up autofocus window
        		{
        			yPos-=2;
        		}
        			
        		OV5640_setAutoFocusWindowPosition(xPos, yPos);
        		vTaskSuspend(NULL);
        		break;
        	}
        	case 0x03: // focus
        	{
        		OV5640_focusZone(30,30);
        		status = OV5640_Get_Focus_Zone();
        		vTaskSuspend(NULL);
        		break;
        	}
        	case 0x04:
        	{
        		status = OV5640_Get_Focus_Zone();
        		vTaskSuspend(NULL);
        		break;
        	}
        	// Trig: 
        	case 0x05:
        	{
            static int8_t bright=0;

        		OV5640_Brightness(bright);
            printf("brightness: %d\n", bright);
              bright++;
              if(bright>8)
              {
                bright = 0;
              }
        		vTaskSuspend(NULL);
        		break;
        	}
        	case 0x06: // take a photo
        	{
        		stopReadDataFromCamera(); 
        		OV5640_changeCameraMode(JPEG);
        		osDelay(100);
        		readDataFromCamera();
        		osDelay(100);
        		OV5640_changeCameraMode(RGB);
        		readDataFromCamera();
        		vTaskSuspend(NULL);
        		break;
        	}

        	case 0x07:
        	{
            static uint8_t sat = 0;
        		OV5640_Color_Saturation(sat);
            printf("Saturation: %d\n", sat);
            sat++;
            if (sat>6) { sat = 0; }
            vTaskSuspend(NULL);
        		break;
        	}
          case 0x08:
          {
            static uint8_t exposure = 0;
            OV5640_Exposure(exposure);
            printf("exposure: %d\n", exposure);
            exposure++;
            if(exposure>6) { exposure = 0; }
            vTaskSuspend(NULL);
            break;
          }
          case 0x09:
          {
            static uint8_t mode=0;
            OV5640_Light_Mode(mode);
            printf("mode: %d\n", mode);
            mode++;
            if(mode > 4) { mode = 0; }
            vTaskSuspend(NULL);
            break;
          }
          case 0x0a:
          {
            static uint8_t sharp=0;
            OV5640_Sharpness(sharp);
            printf("sharp: %d\n", sharp);
            sharp++;
            if(sharp > 33) { sharp = 0; }
            vTaskSuspend(NULL);
            break;
          }
          case 0x0b:
          {
            static uint8_t eft=0;
            OV5640_Special_Effects(eft);
            printf("eft: %d\n", eft);
            eft++;
            if(eft > 6) { eft = 0; }
            vTaskSuspend(NULL);
            break;
          }
        }
	}
	// osDelay(10000);
}

static void rgbMode(void)
{
	cameraRGB();
	readDataFromCamera();
	vTaskSuspend(NULL);
}

static void jpegMode(void)
{
	cameraJPEG();
	readDataFromCamera();
	vTaskSuspend(NULL);
}
