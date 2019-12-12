#include <stdio.h>
#include <stdint.h>
#include "debugTask.h"
#include <stdio.h>
#include "stdio.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "printf.h"
#include "DriversHW/inc/VCP.h"
#include "ov5640.h"


extern QueueHandle_t xQueueDebugCam;
extern SemaphoreHandle_t xSemaphoreSynchroCameraTask;

bufforCycle_t bufforCycle;
uint32_t rxSdramAddress=0;
static void bufferCycle_Init(void);
static uint16_t hexToAscii(uint8_t hex);
uint32_t tmp=0;
uint8_t testData=0;
uint8_t *tmpMem=0;
uint32_t i=0;
bool testDCMI = true;
uint16_t *asciiPhoto;
uint16_t tmpAscii=0;

// queueData = 0bCCCCCCCC XXXXXXXX XXXXXXXX XXXXXXXX 
//C - Command
//X - data
uint32_t queueData=0;


/****** Autofocus ******/
static bool autofocusWindowOnOff = true;
static uint16_t autofocusWindowX = 0;
static uint16_t autofocusWindowY = 0;


void vTaskDEBUG( void * pvParameters )
{
  TaskStatus_t xTaskDetails;
  char stateFreeRTOS[][12]={ {"Ready"},
                              {"Running"},
                              {"Blocked"},
                              {"Suspended"},
                              {"Deleted"}};

  bufferCycle_Init();
  vTaskSuspend(xTaskGetHandle("DEBUG_TASK"));
  while(1)
  {
    if(getVcpState() == TX)
    {
       bufferCycle_SendByVCP();
       // setVcpState(NONE);
    }
    else if (getVcpState() == RX)
    {
      switch(getKeyboardChar())
      {
        case 't':
          printf("--------------------------------------------\n");
          printf("|TASK NUM|    TASK    | PRIOR. |    STATE   | \n");
          vTaskGetInfo( xTaskGetHandle("DEBUG_TASK"), &xTaskDetails, pdTRUE, eInvalid );
          printf("|   %2d   | %.10s  |   %2d   | [%1d]%.9s |\n", (int)xTaskDetails.xTaskNumber, (char*)xTaskDetails.pcTaskName, (int)xTaskDetails.uxBasePriority, xTaskDetails.eCurrentState, (char*)stateFreeRTOS[xTaskDetails.eCurrentState]);
          printf("--------------------------------------------\n");
          vTaskGetInfo( xTaskGetHandle("SDRAM_TASK"), &xTaskDetails, pdTRUE, eInvalid );
          printf("|   %2d   | %.10s  |   %2d   | [%1d]%.9s |\n", (int)xTaskDetails.xTaskNumber, (char*)xTaskDetails.pcTaskName, (int)xTaskDetails.uxBasePriority, xTaskDetails.eCurrentState, (char*)stateFreeRTOS[xTaskDetails.eCurrentState]);
          printf("--------------------------------------------\n");
          vTaskGetInfo( xTaskGetHandle("LCD_TASK"), &xTaskDetails, pdTRUE, eInvalid );
          printf("|   %2d   | %.10s   |   %2d   | [%1d]%.9s |\n", (int)xTaskDetails.xTaskNumber, (char*)xTaskDetails.pcTaskName, (int)xTaskDetails.uxBasePriority, xTaskDetails.eCurrentState, (char*)stateFreeRTOS[xTaskDetails.eCurrentState]);
          printf("--------------------------------------------\n");
          vTaskGetInfo( xTaskGetHandle("CAMERA_TASK"), &xTaskDetails, pdTRUE, eInvalid );
          printf("|   %2d   | %.10s   |   %2d   | [%1d]%.9s |\n", (int)xTaskDetails.xTaskNumber, (char*)xTaskDetails.pcTaskName, (int)xTaskDetails.uxBasePriority, xTaskDetails.eCurrentState, (char*)stateFreeRTOS[xTaskDetails.eCurrentState]);
          printf("--------------------------------------------\n");
          break;
        case 'h':
        case 'H':
          printf("--------------------------------------------\n");
          printf("t - get information about task\n");
          printf("q - take a photo\n");
          printf("p - send photo to computer\n");
          printf("f - focus\n");
          printf("o - ON/OFF focus window\n");
          printf("       set focus window position: \n");
          printf("            i - ^ \n");
          printf("        j - <       > - k\n");
          printf("            m - v \n");
          printf("--------------------------------------------\n");
          break;
        case 'p':
        case 'P':
        {
          uint32_t i=0, j=0;
          uint32_t jpeglen=0, jpgstart=0, jpegstop=0;
          uint8_t  head=0, endJpegFrame=0;
          uint8_t *memAddress;
          memAddress=0xC003FC10;
          stopReadDataFromCamera(); 
          for(i=0;i<700*1024; i++)//search for 0XFF 0XD8 and 0XFF 0XD9, get size of JPG 
          {
              if((*(memAddress + i)==0XFF)&&(*(memAddress + i+1)==0XD8))
              {
                      jpgstart=i;
                      head=1; // Already found  FF D8
              }
              if((*(memAddress + i)==0XFF)&&(*(memAddress + i+1)==0XD9)&&head) //search for FF D9
              {
                jpegstop=i;
                  jpeglen=i-jpgstart+2;
                  endJpegFrame = 1;
                  break;
              }
          }
          rxSdramAddress = 0xC003FC10;
          if( (head == 1) && (endJpegFrame == 1))
          { 
            sendPhoto((uint8_t*)rxSdramAddress+jpgstart, jpeglen);
          } 
          else
          {
            printf("Not found start or end JPEG frame\n");
          }
        }
          // rxSdramAddress = 0xC0000000;
          // sendPhoto((uint8_t*)rxSdramAddress, XSIZE*YSIZE*2);
          // vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          // xSemaphoreTake( xSemaphoreSynchroCameraTask, portMAX_DELAY );
          // xQueueReceive( xQueueDebugCam, &rxSdramAddress, portMAX_DELAY );
          // osDelay(1000);

          // sendPhoto((uint8_t*)rxSdramAddress, XSIZE*YSIZE*2);
          // xSemaphoreGive( xSemaphoreSynchroCameraTask );
          break;

        case 'a':
        case 'A':
          rxSdramAddress=0xC0000000;
          uint32_t i=0;

          
        //   stopReadDataFromCamera();
        //   for(tmp=0; tmp<XSIZE*YSIZE*2; tmp++)
        //   {
        //     *(uint8_t *)(rxSdramAddress+tmp) = 'z';
        //     testData++;
        //     if(testData>0x7F)
        //       testData = 0;
        //     i++;
        //     if(i>=10) { i=0; }
        //   }
          
        //   sendPhoto((uint8_t*)rxSdramAddress, XSIZE*YSIZE*2);
        //   readDataFromCamera();

          break;
        // case 's':
        // case 'S':
        //   rxSdramAddress=0xC0000000;
        //   asciiPhoto = 0xC0000000 + 0x1c2000;
        //   stopReadDataFromCamera();
        //   for(tmp=0; tmp<XSIZE*YSIZE*2; tmp++)
        //   {
        //     *(uint16_t *)(asciiPhoto+tmp) = hexToAscii(*(uint8_t *)(rxSdramAddress+tmp));
        //   }
        //   sendPhoto(asciiPhoto, XSIZE*YSIZE*4);

          
        //   // tmp+=2;
        //   // *asciiPhoto+tmp = hexToAscii(0x3A); //0x3341
        //   // tmp+=2;
        //   // *asciiPhoto+tmp = hexToAscii(0x7F); //0x3746
        //   // tmp+=2;
        //   // *asciiPhoto+tmp = hexToAscii(0xF1); //0x4631

        //   // for(tmp=0; tmp<XSIZE*YSIZE*4; tmp++)
        //   // {
        //   //   *asciiPhoto+tmp=hexToAscii(*(uint8_t *)(rxSdramAddress+tmp));
        //   // }
        //   break;

        case 'd':
        case 'D':
          if(testDCMI == true)
          {
            stopReadDataFromCamera();
            testDCMI = false;
          }
          else
          {
            readDataFromCamera();
            testDCMI = true;
          }
          break;
        case 'o':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x01; //get auto focus window status
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          xSemaphoreTake( xSemaphoreSynchroCameraTask, portMAX_DELAY );
          xQueueReceive(xQueueDebugCam, &queueData, portMAX_DELAY);
          if(((queueData >> 24) & 0x0F ) == 0x01)
          {
            if((queueData&0x0F) == true) { printf(">>Autofocus window: ON"); }
            else { printf(">>Autofocus window: OFF"); }
          }
          break;
        }
        case 'k':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x02; //move auto focus window
          queueData<<=24;
          queueData|=0x01; //move right auto focus window
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 'j':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x02; //move auto focus window
          queueData<<=24;
          queueData|=0x02; //move left auto focus window
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 'm':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x02; //move auto focus window
          queueData<<=24;
          queueData|=0x03; //move down auto focus window
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 'i':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x02; //move auto focus window
          queueData<<=24;
          queueData|=0x04; //move up auto focus window
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 'f':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x03; //focus picture 
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 'g':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x04; 
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 'v':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x05; 
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 'q':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x06;
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 's':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x07; 
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 'e':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x08; 
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
        case 'l':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x09; 
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }

        case 'r':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x0A; 
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }

        case 'y':
        {
          vTaskResume(xTaskGetHandle("CAMERA_TASK"));
          queueData=0x0B; 
          queueData<<=24;
          xQueueSend( xQueueDebugCam, &queueData, 10 );
          break;
        }
      }
      if (getVcpState() != TX)
      {
        setVcpState(IDLE);
      }
    }
    else
    {
      vTaskSuspend(xTaskGetHandle("DEBUG_TASK"));
    }
  }
}

static void cmdTask (uint8_t cmd, uint8_t data)
{
  // xQueueSend( xQueueDebugCam, , 10 );
}

static uint16_t hexToAscii(uint8_t hex)
{
  uint16_t ascii=0;

  if(((hex>>4) & 0x0F)<0x0A ) { ascii=(((hex>>4) & 0x0F) + 0x30); }
  else { ascii=((((hex>>4) & 0x0F) - 10) + 0x41); }

  ascii<<=8;

  if((hex & 0x0F)<0x0A ) { ascii|=((hex & 0x0F) + 0x30); }
  else { ascii|=(((hex & 0x0F) - 10) + 0x41); }

  return ascii;
}

//
static void bufferCycle_Init(void)
{
  bufforCycle.count_appendChar=0;
  bufforCycle.count_sendChar=0;
}

void bufferCycle_AppendChar(char charUsart)
{
  setVcpState(TX);
  vTaskResume(xTaskGetHandle("DEBUG_TASK"));
  bufforCycle.buffor[bufforCycle.count_appendChar]=charUsart;
  bufforCycle.count_appendChar++;
  if(bufforCycle.count_appendChar >= BUFFOR_SIZE-1) { bufforCycle.count_appendChar=0; }
  if(bufforCycle.count_appendChar == bufforCycle.count_sendChar) { asm("bkpt #1"); /*Buffor cycle is overflowe!!*/ }
}

uint32_t bufferCycle_GetAppendCharValue(void)
{
  return bufforCycle.count_appendChar;
}

uint32_t bufferCycle_GetSendCharValue(void)
{
  return bufforCycle.count_sendChar;
}

void bufferCycle_IncrementSendCharValue(void)
{
  bufforCycle.count_sendChar++;
  if(bufforCycle.count_sendChar >= BUFFOR_SIZE-1) { bufforCycle.count_sendChar = 0; }
}

void bufferCycle_SendByVCP(void)
{
  while(bufferCycle_GetSendCharValue() != bufferCycle_GetAppendCharValue())
  {
    TransmitVCP(bufforCycle.buffor[bufferCycle_GetSendCharValue()]);
    if(bufferCycle_GetSendCharValue() != bufferCycle_GetAppendCharValue())
      bufferCycle_IncrementSendCharValue();
  }
}

//stdout handle for printf function
int __io_putchar(int c)
{
  if (c=='\n') 
    bufferCycle_AppendChar('\r');
  bufferCycle_AppendChar(c);
  return c;
}
