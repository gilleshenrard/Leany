/**
 * @file ST7735S.c
 * @brief Implement the functioning of the ST7735S TFT screen via SPI and DMA
 * @author Gilles Henrard
 * @date 08/09/2024
 *
 * @note Datasheet : https://www.displayfuture.com/Display/datasheet/controller/ST7735.pdf
 */
#include "ST7735S.h"
#include "errorstack.h"

/**
 * @brief Initialise the ST7735S display
 *
 * @param handle        SPI handle used
 * @param dma           DMA handle used
 * @param dmaChannel    DMA channel used to send data to the ST7735S
 * @return Success
 */
errorCode_u st7735sInitialise(SPI_TypeDef* handle, DMA_TypeDef* dma, uint32_t dmaChannel) {
    (void)handle;
    (void)dma;
    (void)dmaChannel;

    return (ERR_SUCCESS);
}

/**
 * @brief Run the state machine
 *
 * @return Return code of the current state
 */
errorCode_u st7735sUpdate(void) {
    return (ERR_SUCCESS);
}
