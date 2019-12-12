#include "stm32f7xx.h"
#include "stm32746g_discovery.h"
#include "DriversHW/inc/dcmi.h"
#include "ov5640.h"



DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdmadcmi;
GPIO_InitTypeDef dcmiGpio;

static void BSP_CAMERA_MspInit(void);
static void cameraPwrUpInit(void);

void dcmiInit(void)
{
  // ustawienia dla RGB
  // hdcmi.Instance = DCMI;
  // hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  // hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  // hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
  // hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  // hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  // hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  // hdcmi.Init.JPEGMode = DCMI_JPEG_ENABLE;
  // hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
  // hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
  // hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
  // hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;

	hdcmi.Instance = DCMI;
  hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
	hdcmi.Init.JPEGMode = DCMI_JPEG_ENABLE;
	hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
  hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
  hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
  hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;

  BSP_CAMERA_MspInit();
	HAL_DCMI_Init(&hdcmi);
}

DCMI_HandleTypeDef dcmiGetHandler(void)
{
	return hdcmi;
}


static void BSP_CAMERA_MspInit(void)
{
  
  /*** Enable peripherals and GPIO clocks ***/
  /* Enable DCMI clock */
  __HAL_RCC_DCMI_CLK_ENABLE();

  /* Enable DMA2 clock */
  __HAL_RCC_DMA2_CLK_ENABLE();
  
  /* Enable GPIO clocks */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*** Configure the GPIO ***/
  /* Configure DCMI GPIO as alternate function */
  dcmiGpio.Pin       = GPIO_PIN_4 | GPIO_PIN_6;
  dcmiGpio.Mode      = GPIO_MODE_AF_PP;
  dcmiGpio.Pull      = GPIO_PULLUP;
  dcmiGpio.Speed     = GPIO_SPEED_HIGH;
  dcmiGpio.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOA, &dcmiGpio);

  dcmiGpio.Pin       = GPIO_PIN_3;
  dcmiGpio.Mode      = GPIO_MODE_AF_PP;
  dcmiGpio.Pull      = GPIO_PULLUP;
  dcmiGpio.Speed     = GPIO_SPEED_HIGH;
  dcmiGpio.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOD, &dcmiGpio);

  dcmiGpio.Pin       = GPIO_PIN_5 | GPIO_PIN_6;
  dcmiGpio.Mode      = GPIO_MODE_AF_PP;
  dcmiGpio.Pull      = GPIO_PULLUP;
  dcmiGpio.Speed     = GPIO_SPEED_HIGH;
  dcmiGpio.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOE, &dcmiGpio);

  dcmiGpio.Pin       = GPIO_PIN_9;
  dcmiGpio.Mode      = GPIO_MODE_AF_PP;
  dcmiGpio.Pull      = GPIO_PULLUP;
  dcmiGpio.Speed     = GPIO_SPEED_HIGH;
  dcmiGpio.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOG, &dcmiGpio);

  dcmiGpio.Pin       = GPIO_PIN_9 | GPIO_PIN_10  | GPIO_PIN_11  |\
                                  GPIO_PIN_12 | GPIO_PIN_14;
  dcmiGpio.Mode      = GPIO_MODE_AF_PP;
  dcmiGpio.Pull      = GPIO_PULLUP;
  dcmiGpio.Speed     = GPIO_SPEED_HIGH;
  dcmiGpio.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOH, &dcmiGpio);

  cameraPwrUpInit();
  //ustaienia dla RGB
  // hdmadcmi.Instance = DMA2_Stream1;
  // hdmadcmi.Init.Channel = DMA_CHANNEL_1;
  // hdmadcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
  // hdmadcmi.Init.PeriphInc = DMA_PINC_DISABLE; // Disable incrementation peripherial address
  // hdmadcmi.Init.MemInc = DMA_MINC_ENABLE; // Enable incrementation memory address
  // hdmadcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD; // peripherial data size
  // hdmadcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD; // memory data size
  // hdmadcmi.Init.Mode = DMA_CIRCULAR; //DMA mode
  // hdmadcmi.Init.Priority = DMA_PRIORITY_VERY_HIGH; // Priority 
  // hdmadcmi.Init.FIFOMode = DMA_FIFOMODE_ENABLE; 
  // hdmadcmi.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;//DMA_FIFO_THRESHOLD_FULL;  rgb = DMA_FIFO_THRESHOLD_HALFFULL
  // hdmadcmi.Init.MemBurst = DMA_MBURST_SINGLE;
  // hdmadcmi.Init.PeriphBurst = DMA_PBURST_SINGLE;

  hdmadcmi.Instance = DMA2_Stream1;
  hdmadcmi.Init.Channel = DMA_CHANNEL_1;
  hdmadcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdmadcmi.Init.PeriphInc = DMA_PINC_DISABLE; // Disable incrementation peripherial address
  hdmadcmi.Init.MemInc = DMA_MINC_ENABLE; // Enable incrementation memory address
  hdmadcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD; // peripherial data size
  hdmadcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD; // memory data size
  hdmadcmi.Init.Mode = DMA_CIRCULAR; //DMA mode
  hdmadcmi.Init.Priority = DMA_PRIORITY_VERY_HIGH; // Priority 
  hdmadcmi.Init.FIFOMode = DMA_FIFOMODE_ENABLE; 
  hdmadcmi.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;//DMA_FIFO_THRESHOLD_FULL;  rgb = DMA_FIFO_THRESHOLD_HALFFULL
  hdmadcmi.Init.MemBurst = DMA_MBURST_SINGLE;
  hdmadcmi.Init.PeriphBurst = DMA_PBURST_SINGLE;

  /* Associate the initialized DMA handle to the DCMI handle */
  __HAL_LINKDMA(&hdcmi, DMA_Handle, hdmadcmi);
  
  /*** Configure the NVIC for DCMI and DMA ***/
  /* NVIC configuration for DCMI transfer complete interrupt */
  HAL_NVIC_SetPriority(DCMI_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(DCMI_IRQn);  
  
  /* NVIC configuration for DMA2D transfer complete interrupt */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  
  /* Configure the DMA stream */
  HAL_DMA_Init(&hdmadcmi);

}

static void cameraPwrUpInit(void)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable GPIO clock */
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*** Configure the GPIO ***/
  /* Configure DCMI GPIO as alternate function */
  gpio_init_structure.Pin       = GPIO_PIN_13;
  gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure);

  /* De-assert the camera POWER_DOWN pin (active high) */

  HAL_Delay(3);     /* POWER_DOWN de-asserted during 3ms */
}

void cameraPwrUp(void)
{	
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_13, GPIO_PIN_RESET);
}

void cameraPwrDown(void)
{	
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_13, GPIO_PIN_SET);
}