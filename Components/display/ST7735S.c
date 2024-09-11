/**
 * @file ST7735S.c
 * @brief Implement the functioning of the ST7735S TFT screen via SPI and DMA
 * @author Gilles Henrard
 * @date 09/09/2024
 *
 * @note Datasheet : https://cdn-shop.adafruit.com/datasheets/ST7735R_V0.2.pdf
 */
#include "ST7735S.h"
#include <stdint.h>
#include "ST7735_initialisation.h"
#include "ST7735_registers.h"
#include "errorstack.h"
#include "main.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_spi.h"
#include "systick.h"

enum {
    DISPLAY_WIDTH     = 160U,  ///< Number of pixels in width
    DISPLAY_HEIGHT    = 128U,  ///< Number of pixels in height
    RESET_DELAY_MS    = 150U,  ///< Number of milliseconds to wait after reset
    SLEEPOUT_DELAY_MS = 255U,  ///< Number of milliseconds to wait sleep out
    SPI_TIMEOUT_MS    = 10U,   ///< Number of milliseconds beyond which SPI is in timeout
    BUFFER_SIZE       = 900U,  ///< Size of the frame buffer in bytes
};

/**
 * @brief Enumeration of the function IDs of the SSD1306
 */
typedef enum {
    INIT = 0,         ///< st7735sInitialise()
    SEND_CMD,         ///< sendCommand()
    ORIENT,           ///< st7735sSetOrientation()
    RESETTING,        ///< stateResetting()
    WAKING,           ///< stateExitingSleep()
    CONFIG,           ///< stateConfiguring()
    WAITING_DMA_RDY,  ///< stateWaitingForTXdone()
} SSD1306functionCodes_e;

/**
 * @brief SPI Data/command pin status enumeration
 */
typedef enum {
    COMMAND = 0,  ///< Command is to be sent
    DATA,         ///< Data is to be sent
} DCgpio_e;

/**
 * @brief Screen state machine state prototype
 *
 * @return Return code of the state
 */
typedef errorCode_u (*screenState)(void);

/**
 * @brief Pixel type definition
 */
typedef uint16_t pixel_t;

//utility functions
static inline void setDataCommandGPIO(DCgpio_e function);
static errorCode_u sendCommand(ST7735register_e regNumber, const uint8_t parameters[], uint8_t nbParameters);

//state machine
static errorCode_u stateResetting(void);
static errorCode_u stateConfiguring(void);
static errorCode_u stateWaitingForReset(void);
static errorCode_u stateExitingSleep(void);
static errorCode_u stateWaitingForSleepOut(void);
static errorCode_u stateSendingTestPixels(void);
static errorCode_u stateIdle(void);
static errorCode_u stateWaitingForTXdone(void);
static errorCode_u stateError(void);

//State variables
static SPI_TypeDef*  spiHandle      = (void*)0;            ///< SPI handle used with the SSD1306
static DMA_TypeDef*  dmaHandle      = (void*)0;            ///< DMA handle used with the SSD1306
static uint32_t      dmaChannelUsed = 0x00000000U;         ///< DMA channel used
static screenState   state          = stateResetting;      ///< State machine current state
static pixel_t       displayBuffer[BUFFER_SIZE];           ///< Buffer used to send data to the display
static systick_t     previousTick_ms = 0;                  ///< Latest system tick value saved (in ms)
static errorCode_u   result;                               ///< Buffer used to store function return codes
static uint16_t      displayHeight      = 0;               ///< Current height of the display (depending on orientation)
static uint16_t      displayWidth       = 0;               ///< Current width of the display (depending on orientation)
static orientation_e currentOrientation = NB_ORIENTATION;  ///< Current display orientation

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
 * brief Set the Data/Command pin
 * 
 * @param function Value of the data/command pin
 */
static inline void setDataCommandGPIO(DCgpio_e function) {
    if(function == COMMAND) {
        LL_GPIO_ResetOutputPin(ST7735S_DC_GPIO_Port, ST7735S_DC_Pin);
    } else {
        LL_GPIO_SetOutputPin(ST7735S_DC_GPIO_Port, ST7735S_DC_Pin);
    }
}

/**
 * @brief Send a command with parameters
 *
 * @param regNumber Register number
 * @param parameters Parameters to write
 * @param nbParameters Number of parameters to write
 * @return Success
 * @retval 1	No SPI handle declared
 * @retval 2	No parameters array provided with a non-zero number of parameters
 * @retval 3	Number of parameters above maximum
 * @retval 4	Timeout while sending the command
 */
static errorCode_u sendCommand(ST7735register_e regNumber, const uint8_t parameters[], uint8_t nbParameters) {
    const uint8_t MAX_PARAMETERS = 16U;  ///< Maximum number of parameters a command can have

    //if no handle declared
    if(!spiHandle) {
        return (createErrorCode(SEND_CMD, 1, ERR_WARNING));
    }

    //if nb of params non-zero and parameters array NULL, error
    if(!parameters && nbParameters) {
        return (createErrorCode(SEND_CMD, 2, ERR_WARNING));
    }

    //if too many parameters, error
    if(nbParameters > MAX_PARAMETERS) {
        return (createErrorCode(SEND_CMD, 3, ERR_WARNING));
    }

    //set command pin and enable SPI
    systick_t tickAtStart_ms = getSystick();
    setDataCommandGPIO(COMMAND);
    LL_SPI_Enable(spiHandle);

    //send the command byte and wait for the transaction to be done
    LL_SPI_TransmitData8(spiHandle, (uint8_t)regNumber);
    while(!LL_SPI_IsActiveFlag_TXE(spiHandle) && !isTimeElapsed(tickAtStart_ms, SPI_TIMEOUT_MS)) {}

    //send the parameters
    setDataCommandGPIO(DATA);
    uint8_t* iterator = (uint8_t*)parameters;
    while(nbParameters && !isTimeElapsed(tickAtStart_ms, SPI_TIMEOUT_MS)) {
        //wait for the previous byte to be done, then send the next one
        while(!LL_SPI_IsActiveFlag_TXE(spiHandle) && !isTimeElapsed(tickAtStart_ms, SPI_TIMEOUT_MS)) {}
        if(!isTimeElapsed(tickAtStart_ms, SPI_TIMEOUT_MS)) {
            LL_SPI_TransmitData8(spiHandle, *iterator);
        }

        iterator++;
        nbParameters--;
    }

    //wait for transaction to be finished and clear Overrun flag
    while(LL_SPI_IsActiveFlag_BSY(spiHandle) && !isTimeElapsed(tickAtStart_ms, SPI_TIMEOUT_MS)) {}
    LL_SPI_ClearFlag_OVR(spiHandle);

    //disable SPI and return status
    LL_SPI_Disable(spiHandle);

    //if timeout, error
    if(isTimeElapsed(tickAtStart_ms, SPI_TIMEOUT_MS)) {
        return (createErrorCode(SEND_CMD, 4, ERR_WARNING));
    }

    return (result);
}

/**
 * @brief Set the display orientation
 * 
 * @param orientation New display orientation
 * @return Success
 * @retval 1 Invalid orientation code provided
 * @retval 2 Error while sending the command to the screen
 */
errorCode_u st7735sSetOrientation(orientation_e orientation) {
    //if orientation does not change, exit
    if(currentOrientation == orientation) {
        return (ERR_SUCCESS);
    }

    //if incorrect orientation, error
    if(orientation >= NB_ORIENTATION) {
        return createErrorCode(ORIENT, 1, ERR_CRITICAL);
    }

    //send the command to the display
    result = sendCommand(MADCTL, &orientations[orientation], 1);
    if(isError(result)) {
        return pushErrorCode(result, ORIENT, 2);
    }

    //update the current orientation
    currentOrientation = orientation;

    //update the display dimensions
    switch(orientation) {
        case PORTRAIT:
        case PORTRAIT_180:
            displayHeight = DISPLAY_WIDTH;
            displayWidth  = DISPLAY_HEIGHT;
            break;

        case LANDSCAPE:
        case LANDSCAPE_180:
        case NB_ORIENTATION:
        default:
            displayHeight = DISPLAY_HEIGHT;
            displayWidth  = DISPLAY_WIDTH;
            break;
    }

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
 * @retval 1 Error while transmitting the command
 */
static errorCode_u stateResetting(void) {
    //send the reset command and, if error, exit
    result = sendCommand(SWRESET, NULL, 0);
    if(isError(result)) {
        state = stateError;
        return pushErrorCode(result, RESETTING, 1);
    }

    //save the current systick and get to waiting state
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
 * @retval 1 Error while transmitting the command
 */
static errorCode_u stateExitingSleep(void) {
    //send the reset command and, if error, exit
    result = sendCommand(SLPOUT, NULL, 0);
    if(isError(result)) {
        state = stateError;
        return pushErrorCode(result, WAKING, 1);
    }

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
    if(isTimeElapsed(previousTick_ms, SLEEPOUT_DELAY_MS)) {
        state = stateConfiguring;
    }

    return (ERR_SUCCESS);
}

/**
 * @brief State in which the screen is being configured properly
 * 
 * @return Success
 * @retval 1 Error while sending a command
 * @retval 2 Error while setting the screen orientation
 */
static errorCode_u stateConfiguring(void) {
    for(uint8_t command = 0; command < (uint8_t)ST7735_NB_COMMANDS; command++) {
        result =
            sendCommand(st7735configurationScript[command].registerNumber,
                        st7735configurationScript[command].parameters, st7735configurationScript[command].nbParameters);
        if(isError(result)) {
            state = stateError;
            return pushErrorCode(result, CONFIG, 1);
        }
    }

    result = st7735sSetOrientation(LANDSCAPE_180);
    if(isError(result)) {
        state = stateError;
        return pushErrorCode(result, CONFIG, 2);
    }

    previousTick_ms = getSystick();
    state           = stateSendingTestPixels;
    return (ERR_SUCCESS);
}

/**
 * @brief State in which the screen waits for instructions
 * 
 * @return Success
 */
static errorCode_u stateSendingTestPixels(void) {
    const pixel_t RED      = 0xF800U;
    pixel_t*      iterator = displayBuffer;

    if(!isTimeElapsed(previousTick_ms, SLEEPOUT_DELAY_MS)) {
        return ERR_SUCCESS;
    }

    for(size_t pixel = 0; pixel < (size_t)BUFFER_SIZE; pixel++) {
        *(iterator++) = RED;
    }

    static const uint8_t columns[4] = {0, 10U, 0, 39U};
    static const uint8_t rows[4]    = {0, 10U, 0, 39U};
    sendCommand(CASET, columns, 4);
    sendCommand(RASET, rows, 4);

    //set command pin and enable SPI
    systick_t tickAtStart_ms = getSystick();
    setDataCommandGPIO(COMMAND);
    LL_SPI_Enable(spiHandle);

    //send the command byte and wait for the transaction to be done
    LL_SPI_TransmitData8(spiHandle, (uint8_t)RAMWR);
    while(!LL_SPI_IsActiveFlag_TXE(spiHandle) && !isTimeElapsed(tickAtStart_ms, SPI_TIMEOUT_MS)) {}

    //set data GPIO and enable SPI
    setDataCommandGPIO(DATA);

    //configure the DMA transaction
    LL_DMA_DisableChannel(dmaHandle, dmaChannelUsed);
    LL_DMA_ClearFlag_GI5(dmaHandle);
    LL_DMA_SetDataLength(dmaHandle, dmaChannelUsed, BUFFER_SIZE);  //must be reset every time
    LL_DMA_EnableChannel(dmaHandle, dmaChannelUsed);

    //send the data
    previousTick_ms = getSystick();
    LL_SPI_EnableDMAReq_TX(spiHandle);

    //get to next
    state = stateWaitingForTXdone;
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

/**
 * @brief State in which the machine waits for a DMA transmission to end
 *
 * @return Success
 * @retval 1	Timeout while waiting for transmission to end
 * @retval 2	Error interrupt occurred during the DMA transfer
 */
static errorCode_u stateWaitingForTXdone(void) {
    //if timer elapsed, stop DMA and error
    if(isTimeElapsed(previousTick_ms, SPI_TIMEOUT_MS)) {
        result = createErrorCode(WAITING_DMA_RDY, 1, ERR_ERROR);
        goto finalise;
    }

    //if DMA error, error
    if(LL_DMA_IsActiveFlag_TE5(dmaHandle)) {
        result = createErrorCode(WAITING_DMA_RDY, 2, ERR_ERROR);
        goto finalise;
    }

    //if transmission not complete yet, exit
    if(!LL_DMA_IsActiveFlag_TC5(dmaHandle)) {
        return (ERR_SUCCESS);
    }

finalise:
    LL_DMA_DisableChannel(dmaHandle, dmaChannelUsed);
    LL_SPI_Disable(spiHandle);
    state = stateIdle;
    return result;
}

/**
 * @brief State in which the screen is in error
 * 
 * @return Success
 */
static errorCode_u stateError(void) {
    return (ERR_SUCCESS);
}
