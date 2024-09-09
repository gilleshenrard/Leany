#ifndef INC_HARDWARE_SCREEN_ST7735S_INITIALISATION_H_
#define INC_HARDWARE_SCREEN_ST7735S_INITIALISATION_H_
#include <stdint.h>
#include "ST7735_registers.h"

enum {
    ST7735_NB_COMMANDS    = 2U,   ///< Number of commands to send during initialisation
    ST7735_STRUCT_PADDING = 16U,  ///< Configuration command structure memory alignment size
};

/**
 * @brief Structure describing a command to send during configuration
 */
typedef struct {
    ST7735register_e registerNumber;  ///< Number of the register to send
    uint8_t          nbParameters;    ///< Number of parameters sent after the register number
    const uint8_t*   parameters;      ///< Array of parameters
} __attribute__((aligned(ST7735_STRUCT_PADDING))) st7735_command_t;

#endif
