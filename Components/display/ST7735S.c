/**
 * @file ST7735S.c
 * @brief Implement the functioning of the ST7735S TFT screen via SPI and DMA
 * @author Gilles Henrard
 * @date 09/09/2024
 *
 * @note Datasheet : https://www.displayfuture.com/Display/datasheet/controller/ST7735.pdf
 */
#include "ST7735S.h"
#include <stdint.h>
#include "errorstack.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_spi.h"
#include "systick.h"

enum {
    DISPLAY_WIDTH  = 160U,  ///< Number of pixels in width
    DISPLAY_HEIGHT = 128U,  ///< Number of pixels in height
    RESET_DELAY_MS = 150U,  ///< Number of milliseconds to wait after reset
};

/**
 * @brief Screen state machine state prototype
 *
 * @return Return code of the state
 */
typedef errorCode_u (*screenState)(void);

//state machine
static errorCode_u stateResetting(void);
static errorCode_u stateConfiguring(void);
static errorCode_u stateWaitingForReset(void);
static errorCode_u stateExitingSleep(void);
static errorCode_u stateWaitingForSleepOut(void);
static errorCode_u stateIdle(void);

//State variables
static SPI_TypeDef* spiHandle      = (void*)0;        ///< SPI handle used with the SSD1306
static DMA_TypeDef* dmaHandle      = (void*)0;        ///< DMA handle used with the SSD1306
static uint32_t     dmaChannelUsed = 0x00000000U;     ///< DMA channel used
static screenState  state          = stateResetting;  ///< State machine current state
static uint8_t      displayBuffer;                    ///< Buffer used to send data to the display
static systick_t    previousTick_ms = 0;              ///< Latest system tick value saved (in ms)

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

/**
 * @brief Initialise the ST7735S display
 *
 * @param handle        SPI handle used
 * @param dma           DMA handle used
 * @param dmaChannel    DMA channel used to send data to the ST7735S
 * @return Success
 */
errorCode_u st7735sInitialise(SPI_TypeDef* handle, DMA_TypeDef* dma, uint32_t dmaChannel) {
    spiHandle      = handle;
    dmaHandle      = dma;
    dmaChannelUsed = dmaChannel;

    //make sure to disable ST7735S SPI communication
    LL_SPI_Disable(spiHandle);
    LL_DMA_DisableChannel(dmaHandle, dmaChannelUsed);

    //set the DMA source and destination addresses (will always use the same ones)
    LL_DMA_ConfigAddresses(dmaHandle, dmaChannelUsed, (uint32_t)&displayBuffer, LL_SPI_DMA_GetRegAddr(spiHandle),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    return (ERR_SUCCESS);
}

/**
 * @brief Run the state machine
 *
 * @return Return code of the current state
 */
errorCode_u st7735sUpdate(void) {
    return ((*state)());
}

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

/**
 * @brief State in which a software reset is requested
 * 
 * @return Success
 */
static errorCode_u stateResetting(void) {
    previousTick_ms = getSystick();
    state           = stateWaitingForReset;
    return (ERR_SUCCESS);
}

/**
 * @brief State in which the screen waits for a while after a software reset
 * 
 * @return Success
 */
static errorCode_u stateWaitingForReset(void) {
    if(isTimeElapsed(previousTick_ms, RESET_DELAY_MS)) {
        state = stateExitingSleep;
    }

    return (ERR_SUCCESS);
}

/**
 * @brief State in which a sleep out is requested
 * 
 * @return Success
 */
static errorCode_u stateExitingSleep(void) {
    previousTick_ms = getSystick();
    state           = stateWaitingForSleepOut;
    return (ERR_SUCCESS);
}

/**
 * @brief State in which the screen waits for a while after a sleep out
 * 
 * @return Success
 */
static errorCode_u stateWaitingForSleepOut(void) {
    if(isTimeElapsed(previousTick_ms, RESET_DELAY_MS)) {
        state = stateConfiguring;
    }

    return (ERR_SUCCESS);
}

/**
 * @brief State in which the screen is being configured properly
 * 
 * @return Success
 */
static errorCode_u stateConfiguring(void) {
    state = stateIdle;
    return (ERR_SUCCESS);
}

/**
 * @brief State in which the screen waits for instructions
 * 
 * @return Success
 */
static errorCode_u stateIdle(void) {
    return (ERR_SUCCESS);
}
