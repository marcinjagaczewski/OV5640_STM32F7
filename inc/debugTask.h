#ifndef DEBUGTASK_H
#define DEBUGTASK_H

#include <stdint.h>

#define BUFFOR_SIZE 500


typedef struct bufforCycle_s
{
	uint8_t buffor[BUFFOR_SIZE];
	uint32_t count_appendChar;
	uint32_t count_sendChar;
}bufforCycle_t;

void vTaskDEBUG( void * pvParameters );

// Buffer cycle
void bufferCycle_AppendChar(char char1);
uint32_t bufferCycle_GetAppendCharValue(void);
uint32_t bufferCycle_GetSendCharValue(void);
void bufferCycle_IncrementSendCharValue(void);
void bufferCycle_SendByVCP(void);


#endif