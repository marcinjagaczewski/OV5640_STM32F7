/*
 * VCP.c
 *
 *  Created on: 15.01.2018
 *      Author: Marcin Jagaczewski
 */

#include <stdio.h>
#include "stm32f7xx.h"
#include "stm32f7xx_hal_def.h"
#include "DriversHW/inc/VCP.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "DriversHW/inc/gpio.h"

extern xTaskHandle xHandleDEBUG;
extern UART_HandleTypeDef uart6h;
GPIO_InitTypeDef VCP_gpio;
UART_HandleTypeDef VCP_usart1;
DMA_HandleTypeDef DMA2_usart1TX;
DMA_HandleTypeDef DMA2_usart1RX;
vcp_t vcpState=NONE;
uint8_t rxChar=0;

UART_HandleTypeDef *uartTmpHandler;

static void VCP_DMAInit(void);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	setVcpState(IDLE);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		HAL_UART_Receive_DMA(huart, &rxChar, 1);
	}
	else if(huart->Instance == USART6)
	{
		HAL_UART_Receive_DMA(huart, &rxChar, 1);
	}
	uartTmpHandler=huart;
	setVcpState(RX);
	if(xTaskResumeFromISR(xHandleDEBUG) == pdTRUE)
    {
      portYIELD_FROM_ISR(pdTRUE);
    }
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
}

static void VCP_DMAInit(void)
{
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* USART1_TX Init */
    DMA2_usart1TX.Instance = DMA2_Stream7;
    DMA2_usart1TX.Init.Channel = DMA_CHANNEL_4;
    DMA2_usart1TX.Init.Direction = DMA_MEMORY_TO_PERIPH;
    DMA2_usart1TX.Init.PeriphInc = DMA_PINC_DISABLE;
    DMA2_usart1TX.Init.MemInc = DMA_MINC_ENABLE;
    DMA2_usart1TX.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    DMA2_usart1TX.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    DMA2_usart1TX.Init.Mode = DMA_NORMAL;
    DMA2_usart1TX.Init.Priority = DMA_PRIORITY_LOW;
    DMA2_usart1TX.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    DMA2_usart1TX.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;
    DMA2_usart1TX.Init.MemBurst=DMA_MBURST_SINGLE;
    DMA2_usart1TX.Init.PeriphBurst=DMA_PBURST_SINGLE;

    DMA2_usart1RX.Instance = DMA2_Stream5;
    DMA2_usart1RX.Init.Channel = DMA_CHANNEL_4;
    DMA2_usart1RX.Init.Direction = DMA_PERIPH_TO_MEMORY;
    DMA2_usart1RX.Init.PeriphInc = DMA_PINC_DISABLE;
    DMA2_usart1RX.Init.MemInc = DMA_MINC_DISABLE;
    DMA2_usart1RX.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    DMA2_usart1RX.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    DMA2_usart1RX.Init.Mode = DMA_NORMAL;
    DMA2_usart1RX.Init.Priority = DMA_PRIORITY_LOW;
    DMA2_usart1RX.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    DMA2_usart1RX.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;
    DMA2_usart1RX.Init.MemBurst=DMA_MBURST_SINGLE;
	DMA2_usart1RX.Init.PeriphBurst=DMA_PBURST_SINGLE;


	HAL_DMA_Init(&DMA2_usart1TX);
	__HAL_LINKDMA(&VCP_usart1,hdmatx,DMA2_usart1TX);
	HAL_DMA_Init(&DMA2_usart1RX);
	__HAL_LINKDMA(&VCP_usart1,hdmarx,DMA2_usart1RX);
}

void VCP_gpioInit(void)
{
	// uint8_t size=sizeof(data);
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE();
// TX
	VCP_gpio.Pin=GPIO_PIN_9;
	VCP_gpio.Mode=GPIO_MODE_AF_PP;
	VCP_gpio.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	VCP_gpio.Pull=GPIO_PULLUP;
	VCP_gpio.Alternate=GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &VCP_gpio);
// RX
	VCP_gpio.Pin=GPIO_PIN_7;
	VCP_gpio.Mode=GPIO_MODE_AF_PP;
	VCP_gpio.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	VCP_gpio.Pull=GPIO_PULLUP;
	VCP_gpio.Alternate=GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOB, &VCP_gpio);

	VCP_usart1.Instance=USART1;
	VCP_usart1.Init.BaudRate=115200; //1000000
	VCP_usart1.Init.WordLength=UART_WORDLENGTH_8B;
	VCP_usart1.Init.StopBits=UART_STOPBITS_1;
	VCP_usart1.Init.Parity=UART_PARITY_NONE;
	VCP_usart1.Init.Mode=UART_MODE_TX_RX;
	VCP_usart1.Init.HwFlowCtl=UART_HWCONTROL_NONE;
	VCP_usart1.Init.OverSampling=UART_OVERSAMPLING_16;
	VCP_usart1.Init.OneBitSampling=UART_ONE_BIT_SAMPLE_DISABLE;
	HAL_UART_Init(&VCP_usart1);

	VCP_DMAInit();

	__HAL_UART_ENABLE_IT(&VCP_usart1,UART_IT_TC);
	__HAL_UART_ENABLE_IT(&VCP_usart1,UART_IT_RXNE);

	// HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	HAL_NVIC_SetPriority(USART1_IRQn, 7, 0);
	HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 6, 0);
	HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 5, 0);

	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
	HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

	while(!__HAL_UART_GET_FLAG(&VCP_usart1, UART_FLAG_TC));
	HAL_UART_Transmit_DMA(&VCP_usart1, "ok\n\r", strlen("ok\n\r"));
	while(!__HAL_UART_GET_FLAG(&VCP_usart1, UART_FLAG_TC));
	HAL_UART_Transmit_DMA(&VCP_usart1, "dziala\n\r", strlen("dziala\n\r"));

	HAL_UART_Receive_DMA(&VCP_usart1, &rxChar, 1);
}
#define PACK_SIZE 65535
#define TIMEOUT_SEND_PHOTO 100000

void sendPhoto(uint8_t *data, uint32_t length)
{
	uint32_t tmpLength=length;
	uint32_t len = 0;
	uint32_t lenPackage=0;
	uint32_t offset=0;
	HAL_StatusTypeDef status=HAL_OK;
	uint8_t receivedChecksum=0;
	uint8_t checksum=0;
	uint32_t i=0;

	uint32_t testFullPack = 0;
	uint32_t testChange = 0;
	uint32_t testLoop = 0;

	HAL_UART_DMAStop(uartTmpHandler);
	while(tmpLength>0)
	{
		if( (tmpLength / PACK_SIZE) > 0 ) { tmpLength-= PACK_SIZE; lenPackage = PACK_SIZE; testFullPack++; }
		else { lenPackage = tmpLength % PACK_SIZE; tmpLength -= lenPackage; testChange++; }

		while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
		status = HAL_UART_Transmit_DMA(uartTmpHandler, data+offset, lenPackage); 
		// HAL_UART_Transmit(uartTmpHandler, data+offset, lenPackage, 5000);
		offset+=lenPackage;


		// do
		// {
		// 	while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
		// 	status = HAL_UART_Transmit_DMA(uartTmpHandler, data+offset, lenPackage); 
		// }while(status!=HAL_OK);
		// offset+=lenPackage;
		// 	//wysylanie danych z kamery
		// 	osDelay(100);
		// 	while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
		// 	status = HAL_UART_Transmit_DMA(uartTmpHandler, data+offset, lenPackage); 
		
		// 	// HAL_UART_Receive(uartTmpHandler, &receivedChecksum, 1, TIMEOUT_SEND_PHOTO); // tutaj dać2 continue petli...
		// 	// for(i=offset; i<offset+PACK_SIZE; i++) { checksum^=*(data+i); }
		// 	if(status == HAL_OK)
		// 	{
		// 		offset+=lenPackage;
		// 		len += lenPackage;
		// 	}
			
		// 	//wyslanie wyliczonej sumy kontrolnej
		// 	// osDelay(100);
		// 	// while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
		// 	// status=HAL_UART_Transmit_DMA(uartTmpHandler, &checksum, 1);
		// }while(len < length);
		// // }while(receivedChecksum != checksum);
		// offset+=lenPackage;
	}
	// printf(">>testFullPack: %d\n", testFullPack);
	// printf(">>testChange: %d\n", testChange);
	while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
	HAL_UART_Receive_DMA(uartTmpHandler, &rxChar, 1);
}




// void sendPhoto(uint8_t *data, uint32_t length)
// {
// 	uint32_t tmpLength=length;
// 	uint32_t offset=0;
// 	HAL_StatusTypeDef status=HAL_OK;
// 	uint8_t receivedChecksum=0;
// 	uint8_t checksum=0;
// 	uint32_t i=0;

// 	HAL_UART_DMAStop(uartTmpHandler);
// 	while(tmpLength!=0)
// 	{
// 		if(tmpLength>PACK_SIZE)
// 		{
// 			do
// 			{
// 				receivedChecksum=0;
// 				checksum=0;
// 				while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
// 				status=HAL_UART_Transmit_DMA(uartTmpHandler, data+offset, PACK_SIZE);
// 				for(i=offset; i<offset+PACK_SIZE; i++)
// 					{ checksum^=*(data+i); }
// 				while(HAL_UART_Receive(uartTmpHandler, &receivedChecksum, 1, TIMEOUT_SEND_PHOTO) != HAL_OK);
// 				while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
// 				status=HAL_UART_Transmit_DMA(uartTmpHandler, &checksum, 1);
// 			}while(receivedChecksum != checksum);
// 			offset+=PACK_SIZE;
// 			tmpLength-=PACK_SIZE;
// 		}
// 		else
// 		{
// 			if(tmpLength>0)
// 			{
// 				do
// 				{
// 					receivedChecksum=0;
// 					checksum=0;
// 					while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
// 					status=HAL_UART_Transmit_DMA(uartTmpHandler, data+offset, tmpLength);
// 					for(i=offset; i<offset+tmpLength; i++)
// 						{ checksum^=*(data+i); }
// 					while(HAL_UART_Receive(uartTmpHandler, &receivedChecksum, 1, TIMEOUT_SEND_PHOTO) != HAL_OK);
// 					while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
// 					status=HAL_UART_Transmit_DMA(uartTmpHandler, &checksum, 1);
// 				}while(receivedChecksum != checksum);
// 				tmpLength=0;
// 			}
// 		}
// 	}
// 	while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
// 	HAL_UART_Receive_DMA(uartTmpHandler, &rxChar, 1);
// }


void TransmitDataVCP(uint8_t *data, uint32_t length)
{
	uint32_t tmpLength=length;
	uint32_t offset=0;
	HAL_StatusTypeDef status=HAL_OK;

	HAL_UART_DMAStop(uartTmpHandler);
	while(tmpLength!=0)
	{
		if(tmpLength>65535)
		{
			while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
			status=HAL_UART_Transmit_DMA(uartTmpHandler, data+offset, 65535);
			offset+=65535;
			tmpLength-=65535;
			osDelay(100);
		}
		else
		{
			while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
			status=HAL_UART_Transmit_DMA(uartTmpHandler, data+offset, tmpLength);
			tmpLength=0;
		}
	}
	while(HAL_UART_GetState(uartTmpHandler) != HAL_UART_STATE_READY);
	HAL_UART_Receive_DMA(uartTmpHandler, &rxChar, 1);
}

void TransmitVCP(char charUsart)
{
	while(!__HAL_UART_GET_FLAG(uartTmpHandler, UART_FLAG_TC));
	HAL_UART_Transmit_DMA(uartTmpHandler, (uint8_t*)&charUsart, 1);
	// HAL_UART_Transmit_DMA(&uart6h, "dziala\n", 7);
}

void ReceiveVCP(void)
{
	HAL_UART_Receive_DMA(&VCP_usart1, &rxChar, 1);
}

void setVcpState(vcp_t state)
{
	vcpState=state;
}

vcp_t getVcpState(void)
{
	return vcpState;
}

uint8_t getKeyboardChar(void)
{
	return rxChar;
}
