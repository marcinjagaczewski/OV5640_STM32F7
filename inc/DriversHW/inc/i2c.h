#ifndef __I2C_HANDLER
#define __I2C_HANDLER


void initI2C(void);
uint8_t OV5640_RD_Reg(uint16_t reg);
uint8_t OV5640_WR_Reg(uint16_t reg,uint8_t data);

#endif