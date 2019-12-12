#ifndef __DMA2D_HANDLE
#define __DMA2D_HANDLE

typedef enum dma2dStatus_e
{
	DMA2D_OK = 0,
	DMA2D_ERROR_TE = 0x01,
	DMA2D_ERROR_CE = 0x02,
	DMA2D_ERROR_CAE = 0x04,
	DMA2D_ERROR_TIMEOUT = 0x20,
}dma2dStatus_t;

void dma2dInit(void);
dma2dStatus_t dma2dStartTransmision(uint32_t src, uint32_t dst, uint32_t Width, uint32_t Height);
dma2dStatus_t dma2dPollTransfer(uint32_t Timeout);

#endif