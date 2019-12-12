#include "stm32f7xx.h"
#include "DriversHW/inc/dma2d.h"
#include "DriversHW/inc/lcdt.h"


DMA2D_HandleTypeDef hdma2d ;

void dma2dInit(void)
{
	__HAL_RCC_DMA2D_CLK_ENABLE();


	hdma2d.Instance = DMA2D;
	hdma2d.Init.Mode=DMA2D_M2M;
	hdma2d.Init.ColorMode=DMA2D_OUTPUT_RGB565;
	hdma2d.Init.OutputOffset=0;
	hdma2d.LayerCfg[0].InputOffset=0;
	hdma2d.LayerCfg[0].InputColorMode=DMA2D_INPUT_RGB565;
	hdma2d.LayerCfg[0].AlphaMode=DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[0].InputAlpha=0;

	if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
	  {
	    asm("bkpt #1");
	  }

	  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
	  {
	    asm("bkpt #1");
	  }
} 

dma2dStatus_t dma2dStartTransmision(uint32_t src, uint32_t dst, uint32_t Width, uint32_t Height)
{
	HAL_StatusTypeDef status=0;

	status=HAL_DMA2D_Start(&hdma2d, src, dst, Width, Height);
	return (dma2dStatus_t)status;

}

dma2dStatus_t dma2dPollTransfer(uint32_t Timeout)
{
	HAL_StatusTypeDef status=0;

	HAL_DMA2D_PollForTransfer(&hdma2d, Timeout); 
	return (dma2dStatus_t)status;
}
