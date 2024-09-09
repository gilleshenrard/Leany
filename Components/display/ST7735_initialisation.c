#include "ST7735_initialisation.h"
#include "ST7735_registers.h"

#define NULL (void*)0  ///< NULL address

/**
 * @brief Configuration commands list
 */
const st7735_command_t st7735configurationScript[ST7735_NB_COMMANDS] = {
    {SWRESET, 0, NULL}, //software reset
    { SLPOUT, 0, NULL}, //sleep out
};
