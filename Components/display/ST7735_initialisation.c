#include "ST7735_initialisation.h"
#include <stdint.h>
#include "ST7735_registers.h"

#define NULL (void*)0  ///< NULL address

static const uint8_t framerateControl_args[3] = {
    ONELINEPERIOD_1,
    FRONTPORCH_DEFAULT,
    BACKPORCH_DEFAULT,
};

static const uint8_t framerateControlPartial_args[6] = {
    ONELINEPERIOD_1, FRONTPORCH_DEFAULT, BACKPORCH_DEFAULT, ONELINEPERIOD_1, FRONTPORCH_DEFAULT, BACKPORCH_DEFAULT,
};

static const uint8_t inversionControl_arg = ALL_MODES_NO_INVERSION;

static const uint8_t powerControl1_args[3] = {
    AVDD_5V | GVDD_4_6V,
    GVCL_NEG_4_6V,
    POWER_MODE_AUTO,
};

static const uint8_t powerControl2_arg = VGH25_2_4C | VGL_10 | VGH_3ADD;

static const uint8_t powerControl3_args[2] = {
    BOOST_MAX | OPAMP_HIGH_SMALL_CUR | OPAMP_LOW_MEDLOW_CUR,
    BOOST_MAX,
};

static const uint8_t powerControl4_args[2] = {
    (BOOST_BCLK_2 << 6U) | OPAMP_HIGH_SMALL_CUR | OPAMP_LOW_MEDLOW_CUR,
    BOOST_LSB_BCLK_2,
};

static const uint8_t powerControl5_args[2] = {
    (BOOST_BCLK_2 << 6U) | OPAMP_HIGH_SMALL_CUR | OPAMP_LOW_MEDLOW_CUR,
    BOOST_LSB_IDLE_MODE,
};

static const uint8_t vmCtr1_arg = VCOM_NEG_0_775V;

// NOLINTNEXTLINE(misc-redundant-expression)
static const uint8_t madCtrl_arg = COL_ROW_ADDRESS | REFRESH_TOP_BOTTOM | REFRESH_LEFT_RIGHT | COLORS_ORDER_RGB;

static const uint8_t colorMode_arg = COLOUR_16BITS;

/**
 * @brief Configuration commands list
 */
const st7735_command_t st7735configurationScript[ST7735_NB_COMMANDS] = {
    {SWRESET, 0,                         NULL}, //software reset
    { SLPOUT, 0,                         NULL}, //sleep out
    {FRMCTR1, 3,        framerateControl_args}, //set frame rate in normal mode
    {FRMCTR2, 3,        framerateControl_args}, //set frame rate in idle mode
    {FRMCTR3, 3, framerateControlPartial_args}, //set frame rate in partial mode
    { INVCTR, 1,        &inversionControl_arg}, //disable inversion
    { PWCTR1, 3,           powerControl1_args}, //set power control 1
    { PWCTR2, 1,           &powerControl2_arg}, //set power control 2
    { PWCTR3, 2,           powerControl3_args}, //set power control 3 in normal mode
    { PWCTR4, 2,           powerControl4_args}, //set power control 4 in idle mode
    { PWCTR5, 2,           powerControl5_args}, //set power control 5 in partial mode
    { VMCTR1, 1,                  &vmCtr1_arg}, //set VCOM voltage
    { INVOFF, 0,                         NULL}, //no display inversion
    { MADCTL, 1,                 &madCtrl_arg}, //address control
    { COLMOD, 1,               &colorMode_arg}, //color mode
};
