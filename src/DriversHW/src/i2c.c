#include <stdio.h>
#include "stm32f7xx.h"
#include "stm32f7xx_hal_def.h"
#include "DriversHW/inc/i2c.h"


I2C_HandleTypeDef hi2c;
static void I2Cx_MspInit(I2C_HandleTypeDef *i2c_handler);
	uint8_t I2cFound[50]={0};

void initI2C(void)
{
	uint8_t adresI2c=0, i=0;


	hi2c.Instance = I2C1;
    hi2c.Init.Timing           = 0x40912732;
    hi2c.Init.OwnAddress1      = 0;
    hi2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    hi2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    hi2c.Init.OwnAddress2      = 0;
    hi2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    hi2c.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    /* Init the I2C */
    I2Cx_MspInit(&hi2c);
    HAL_I2C_Init(&hi2c);

    for(adresI2c=0; adresI2c<0xff; adresI2c++)
    {
    	if(HAL_I2C_IsDeviceReady(&hi2c, (uint16_t)adresI2c, 10, 10)==HAL_OK)
    	{
    		I2cFound[i]=adresI2c;
    		i++;
    	}
    }
    i++;
}

static void I2Cx_MspInit(I2C_HandleTypeDef *i2c_handler)
{
	GPIO_InitTypeDef  gpio_init_structure;

	if(HAL_I2C_GetState(i2c_handler) == HAL_I2C_STATE_RESET)
  	{
  		__HAL_RCC_GPIOB_CLK_ENABLE();

  		gpio_init_structure.Pin = GPIO_PIN_8;
    	gpio_init_structure.Mode = GPIO_MODE_AF_OD;
	    gpio_init_structure.Pull = GPIO_NOPULL;
	    gpio_init_structure.Speed = GPIO_SPEED_FAST;
	    gpio_init_structure.Alternate = GPIO_AF4_I2C1;
	    HAL_GPIO_Init(GPIOB, &gpio_init_structure);

	    /* Configure I2C Rx as alternate function */
	    gpio_init_structure.Pin = GPIO_PIN_9;
	    HAL_GPIO_Init(GPIOB, &gpio_init_structure);

	    __HAL_RCC_I2C1_CLK_ENABLE();

	    /* Force the I2C peripheral clock reset */
	    __HAL_RCC_I2C1_FORCE_RESET();

	    /* Release the I2C peripheral clock reset */
	    __HAL_RCC_I2C1_RELEASE_RESET();

	    /* Enable and set I2Cx Interrupt to a lower priority */
	    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0x0F, 0);
	    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);

	    /* Enable and set I2Cx Interrupt to a lower priority */
	    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0x0F, 0);
	    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  	}
}

uint8_t OV5640_RD_Reg(uint16_t reg)
{
	uint8_t data=0;
	HAL_I2C_Mem_Read(&hi2c, 0X78, reg, 2, &data, 1, 100);
	return data;
}

uint8_t OV5640_WR_Reg(uint16_t reg,uint8_t data)
{
	HAL_I2C_Mem_Write(&hi2c, 0X78, reg, 2, &data, 1, 100);
	return 1;
}