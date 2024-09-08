#include "ST7735S.h"
#include <errorstack.h>

errorCode_u st7735sInitialise(SPI_TypeDef* handle, DMA_TypeDef* dma, uint32_t dmaChannel) {
    (void)handle;
    (void)dma;
    (void)dmaChannel;

    return (ERR_SUCCESS);
}

errorCode_u st7735sUpdate(void) {
    return (ERR_SUCCESS);
}
