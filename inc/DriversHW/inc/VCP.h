/*
 * VCP.h
 *
 *  Created on: 15.01.2018
 *      Author: Marcin Jagaczewski
 */

#ifndef DRIVERSHW_INC_VCP_H_
#define DRIVERSHW_INC_VCP_H_

typedef enum vcp_e
{
	NONE=0,
	RX=1,
	TX=2,
	IDLE=3,
}vcp_t;


void VCP_gpioInit(void);
void sendPhoto(uint8_t *data, uint32_t length);
void TransmitDataVCP(uint8_t *data, uint32_t length);
void TransmitVCP(char char1);
void ReceiveVCP(void);
void ReceiveVCPPulling(uint8_t *pData, uint32_t size);
void setVcpState(vcp_t state);
vcp_t getVcpState(void);
uint8_t getKeyboardChar(void);




#endif /* DRIVERSHW_INC_VCP_H_ */
