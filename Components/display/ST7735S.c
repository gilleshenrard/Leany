/**
 * @file ST7735S.c
 * @brief Implement the functioning of the ST7735S TFT screen via SPI and DMA
 * @author Gilles Henrard
 * @date 16/09/2024
 *
 * @note Datasheet : https://cdn-shop.adafruit.com/datasheets/ST7735R_V0.2.pdf
 */
#include "ST7735S.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "ST7735_initialisation.h"
#include "ST7735_registers.h"
#include "errorstack.h"
#include "icons.h"
#include "main.h"
#include "projdefs.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_spi.h"
#include "task.h"

enum {
    STACK_SIZE        = 128U,  ///< Amount of words in the task stack
    TASK_LOW_PRIORITY = 8U,    ///< FreeRTOS number for a low priority task
    DISPLAY_WIDTH     = 160U,  ///< Number of pixels in width
    DISPLAY_HEIGHT    = 128U,  ///< Number of pixels in height
    SPI_TIMEOUT_MS    = 10U,   ///< Number of milliseconds beyond which SPI is in timeout
    FRAME_BUFFER_SIZE =
        (DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(pixel_t)) / 10U,  ///< Size of the frame buffer in bytes
};

/**
 * @brief Enumeration of the function IDs of the SSD1306
 */
typedef enum {
    INIT = 0,         ///< st7735sInitialise()
    SEND_CMD,         ///< sendCommand()
    FILL_BACKGROUND,  ///< printBackground()
    PRINT_CHAR,       ///< printCharacter()
    ORIENT,           ///< st7735sSetOrientation()
    STARTUP,          ///< stateStartup()
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

//utility functions
static void               taskST7735S(void* argument);
static inline void        setDataCommandGPIO(DCgpio_e function);
static inline errorCode_u setWindow(uint8_t Xstart, uint8_t Ystart, uint8_t width, uint8_t height);
static inline void        turnBacklightON(void);
static errorCode_u        sendCommand(ST7735register_e regNumber, const uint8_t parameters[], uint8_t nbParameters);

//state machine
static errorCode_u stateStartup(void);
static errorCode_u stateConfiguring(void);
static errorCode_u stateIdle(void);
static errorCode_u stateError(void);

//State variables
static volatile TaskHandle_t taskHandle     = NULL;             ///< handle of the FreeRTOS task
static SPI_TypeDef*          spiHandle      = (void*)0;         ///< SPI handle used with the SSD1306
static DMA_TypeDef*          dmaHandle      = (void*)0;         ///< DMA handle used with the SSD1306
static uint32_t              dmaChannelUsed = 0x00000000U;      ///< DMA channel used
static screenState           state          = stateStartup;     ///< State machine current state
static registerValue_t       displayBuffer[FRAME_BUFFER_SIZE];  ///< Buffer used to send data to the display
static errorCode_u           result;                            ///< Buffer used to store function return codes
static uint8_t               displayHeight      = 0;  ///< Current height of the display (depending on orientation)
static uint8_t               displayWidth       = 0;  ///< Current width of the display (depending on orientation)
static orientation_e         currentOrientation = NB_ORIENTATION;  ///< Current display orientation
static uint32_t              dataTXRemaining    = 0;

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

void st7735sDMAinterruptHandler(void) {
    BaseType_t hasWoken = 0;
    vTaskNotifyGiveFromISR(taskHandle, &hasWoken);
    portYIELD_FROM_ISR(hasWoken);
}

/**
 * @brief Create the ST7735S FreeRTOS task
 *
 * @param handle        SPI handle used
 * @param dma           DMA handle used
 * @param dmaChannel    DMA channel used to send data to the ST7735S
 * @return Success
 */
errorCode_u createST7735Stask(SPI_TypeDef* handle, DMA_TypeDef* dma, uint32_t dmaChannel) {
    static StackType_t  taskStack[STACK_SIZE] = {0};  ///< Buffer used as the task stack
    static StaticTask_t taskState             = {0};  ///< Task state variables

    //save the SPI handle and DMA channel used by the ST7735S
    spiHandle      = handle;
    dmaHandle      = dma;
    dmaChannelUsed = dmaChannel;

    //make sure to disable ST7735S SPI communication
    LL_SPI_Disable(spiHandle);
    LL_DMA_DisableChannel(dmaHandle, dmaChannelUsed);
    LL_DMA_EnableIT_TC(dmaHandle, dmaChannelUsed);

    //set the DMA source and destination addresses (will always use the same ones)
    LL_DMA_ConfigAddresses(dmaHandle, dmaChannelUsed, (uint32_t)&displayBuffer, LL_SPI_DMA_GetRegAddr(spiHandle),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    //create the static task
    taskHandle =
        xTaskCreateStatic(taskST7735S, "ST7735S task", STACK_SIZE, NULL, TASK_LOW_PRIORITY, taskStack, &taskState);
    if(!taskHandle) {
        Error_Handler();
    }

    return (ERR_SUCCESS);
}

/**
 * @brief Run the state machine
 *
 * @return Return code of the current state
 */
static void taskST7735S(void* argument) {
    UNUSED(argument);

    while(1) {
        result = (*state)();
        if(isError(result)) {
            Error_Handler();
        }
    }
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

//NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
static inline errorCode_u setWindow(uint8_t Xstart, uint8_t Ystart, uint8_t width, uint8_t height) {
    //set the data window columns count
    uint8_t columns[4] = {0, Xstart, 0, Xstart + width};
    result             = sendCommand(CASET, columns, 4);
    if(isError(result)) {
        return pushErrorCode(result, FILL_BACKGROUND, 1);
    }

    //set the data window rows count
    uint8_t rows[4] = {0, Ystart, 0, Ystart + height};
    result          = sendCommand(RASET, rows, 4);
    if(isError(result)) {
        return pushErrorCode(result, FILL_BACKGROUND, 2);
    }

    return ERR_SUCCESS;
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
    uint32_t SPItick = HAL_GetTick();
    setDataCommandGPIO(COMMAND);
    LL_SPI_Enable(spiHandle);

    //send the command byte and wait for the transaction to be done
    LL_SPI_TransmitData8(spiHandle, (uint8_t)regNumber);
    while(!LL_SPI_IsActiveFlag_TXE(spiHandle) && ((HAL_GetTick() - SPItick) < SPI_TIMEOUT_MS)) {}

    //send the parameters
    setDataCommandGPIO(DATA);
    uint8_t* iterator = (uint8_t*)parameters;
    while(nbParameters && ((HAL_GetTick() - SPItick) < SPI_TIMEOUT_MS)) {
        //wait for the previous byte to be done, then send the next one
        while(!LL_SPI_IsActiveFlag_TXE(spiHandle) && ((HAL_GetTick() - SPItick) < SPI_TIMEOUT_MS)) {}
        if(!LL_SPI_IsActiveFlag_TXE(spiHandle)) {
            LL_SPI_TransmitData8(spiHandle, *iterator);
        }

        iterator++;
        nbParameters--;
    }

    //wait for transaction to be finished and clear Overrun flag
    while(LL_SPI_IsActiveFlag_BSY(spiHandle) && ((HAL_GetTick() - SPItick) < SPI_TIMEOUT_MS)) {}
    LL_SPI_ClearFlag_OVR(spiHandle);

    //disable SPI and return status
    LL_SPI_Disable(spiHandle);

    //if timeout, error
    if(((HAL_GetTick() - SPItick) >= SPI_TIMEOUT_MS)) {
        return (createErrorCode(SEND_CMD, 4, ERR_WARNING));
    }

    return (ERR_SUCCESS);
}

//NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
static errorCode_u sendData(uint32_t* dataRemaining) {
    if(!dataRemaining) {
        return ERR_SUCCESS;
    }

    //set command pin and enable SPI
    uint32_t SPItick = HAL_GetTick();
    setDataCommandGPIO(COMMAND);
    LL_SPI_Enable(spiHandle);

    //send the command byte and wait for the transaction to be done
    LL_SPI_TransmitData8(spiHandle, (uint8_t)RAMWR);
    while(!LL_SPI_IsActiveFlag_TXE(spiHandle) && ((HAL_GetTick() - SPItick) < SPI_TIMEOUT_MS)) {}
    if(!LL_SPI_IsActiveFlag_TXE(spiHandle)) {
        return pushErrorCode(result, FILL_BACKGROUND, 3);
    }

    //clamp the data to send to max. the frameBuffer
    uint32_t dataToSend = (*dataRemaining > FRAME_BUFFER_SIZE ? FRAME_BUFFER_SIZE : *dataRemaining);

    //set data GPIO and enable SPI
    setDataCommandGPIO(DATA);

    //configure the DMA transaction
    LL_DMA_DisableChannel(dmaHandle, dmaChannelUsed);
    LL_DMA_ClearFlag_GI5(dmaHandle);
    LL_DMA_SetDataLength(dmaHandle, dmaChannelUsed, dataToSend);  //must be reset every time
    LL_DMA_EnableChannel(dmaHandle, dmaChannelUsed);
    LL_SPI_EnableDMAReq_TX(spiHandle);

    //wait for measurements to be ready
    if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(SPI_TIMEOUT_MS)) == pdFALSE) {
        state = stateError;
        return (createErrorCode(FILL_BACKGROUND, 1, ERR_CRITICAL));
    }

    //if DMA error, stop DMA and error
    if(LL_DMA_IsActiveFlag_TE5(dmaHandle)) {
        LL_DMA_DisableChannel(dmaHandle, dmaChannelUsed);
        LL_SPI_Disable(spiHandle);
        state = stateError;
        return createErrorCode(WAITING_DMA_RDY, 2, ERR_ERROR);
    }

    *dataRemaining -= dataToSend;
    if(!dataRemaining) {
        LL_DMA_DisableChannel(dmaHandle, dmaChannelUsed);
        LL_SPI_Disable(spiHandle);
    }

    return (ERR_SUCCESS);
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
 * @brief Turn the display TFT backlight ON
 */
static inline void turnBacklightON(void) {
    LL_GPIO_SetOutputPin(ST7735S_BL_GPIO_Port, ST7735S_BL_Pin);
}

/********************************************************************************************************************************************/
/********************************************************************************************************************************************/

/**
 * @brief State in which a software reset and a sleepout are requested
 * 
 * @return Success
 * @retval 1 Error while transmitting the software reset command
 * @retval 2 Error while transmitting the sleep out command
 */
static errorCode_u stateStartup(void) {
    static const uint8_t RESET_DELAY_MS    = 150U;  ///< Number of milliseconds to wait after reset
    static const uint8_t SLEEPOUT_DELAY_MS = 255U;  ///< Number of milliseconds to wait sleep out

    //send the reset command and, if error, exit
    result = sendCommand(SWRESET, NULL, 0);
    if(isError(result)) {
        state = stateError;
        return pushErrorCode(result, STARTUP, 1);
    }

    //wait for a while after software reset
    vTaskDelay(pdMS_TO_TICKS(RESET_DELAY_MS));

    //send the reset command and, if error, exit
    result = sendCommand(SLPOUT, NULL, 0);
    if(isError(result)) {
        state = stateError;
        return pushErrorCode(result, STARTUP, 2);
    }

    //wait for a while after sleepout
    vTaskDelay(pdMS_TO_TICKS(SLEEPOUT_DELAY_MS));

    //save the current systick and get to waiting state
    state = stateConfiguring;
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
    const uint8_t BYTE_DOWNSHIFT = 8U;
    const uint8_t BYTE_MASK      = 0xFFU;
    uint8_t*      iterator       = displayBuffer;

    //execute all configuration commands
    for(uint8_t command = 0; command < (uint8_t)ST7735_NB_COMMANDS; command++) {
        result =
            sendCommand(st7735configurationScript[command].registerNumber,
                        st7735configurationScript[command].parameters, st7735configurationScript[command].nbParameters);
        if(isError(result)) {
            state = stateError;
            return pushErrorCode(result, CONFIG, 1);
        }
    }

    //set screen orientation
    result = st7735sSetOrientation(LANDSCAPE_180);
    if(isError(result)) {
        state = stateError;
        return pushErrorCode(result, CONFIG, 2);
    }

    //turn on backlight
    turnBacklightON();

    //fill the frame buffer with background pixels
    for(uint16_t pixel = 0; pixel < (uint16_t)FRAME_BUFFER_SIZE; pixel++) {
        *(iterator++) = (registerValue_t)((pixel_t)DARK_CHARCOAL >> BYTE_DOWNSHIFT);
        *(iterator++) = (registerValue_t)((pixel_t)DARK_CHARCOAL & BYTE_MASK);
    }

    result = setWindow(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if(isError(result)) {
        state = stateError;
        return pushErrorCode(result, CONFIG, 3);
    }

    dataTXRemaining = (DISPLAY_HEIGHT + 2) * (DISPLAY_WIDTH + 1) * sizeof(pixel_t);
    do {
        result = sendData(&dataTXRemaining);
    } while(dataTXRemaining && !isError(result));

    state = stateIdle;
    return (ERR_SUCCESS);
}

/**
 * @brief State in which the screen waits for instructions
 * 
 * @return Success
 */
static errorCode_u stateIdle(void) {
    static uint8_t nbChar = 1;

    if(nbChar) {
        nbChar--;

        setWindow(2U, 2U, VERDANA_NB_COLUMNS + 1U, VERDANA_NB_ROWS + 2U);

        //fill the frame buffer with background pixels
        uint8_t* iterator = displayBuffer;
        for(uint8_t row = 0; row < (uint8_t)VERDANA_NB_ROWS; row++) {
            uncompressIconLine(iterator, VERDANA_1, row);
            iterator += ((uint8_t)VERDANA_NB_COLUMNS << 1U);
        }

        uint32_t characterSize = VERDANA_NB_COLUMNS * VERDANA_NB_ROWS * sizeof(pixel_t);
        result                 = sendData(&characterSize);
        if(isError(result)) {
            state = stateError;
        }
    }

    return (ERR_SUCCESS);
}

/**
 * @brief State in which the screen is in error
 * 
 * @return Success
 */
static errorCode_u stateError(void) {
    return (ERR_SUCCESS);
}
