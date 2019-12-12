#ifndef MAIN_H_
#define MAIN_H_


typedef struct memQueue_s
{
	uint32_t address;
	uint32_t *addressData;
	uint32_t data;
	uint32_t size;
}memQueue_t;

void startTaskTime(void);
uint32_t getTaskLive(void);

#endif