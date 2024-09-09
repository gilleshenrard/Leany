#ifndef INC_HARDWARE_SCREEN_ST7735S_REGISTERS_H_
#define INC_HARDWARE_SCREEN_ST7735S_REGISTERS_H_

/**
 * @brief ST7735 System Function command List
 */
typedef enum {
    NOP       = 0x00U,  ///< No Operation
    SWRESET   = 0x01U,  ///< Software reset
    RDDID     = 0x04U,  ///< Read Display ID
    RDDST     = 0x09U,  ///< Read Display Status
    RDDPM     = 0x0AU,  ///< Read Display Power
    RDDMADCTL = 0x0BU,  ///< Read Display
    RDDCOLMOD = 0x0CU,  ///< Read Display Pixel
    RDDIM     = 0x0DU,  ///< Read Display Image
    RDDSM     = 0x0EU,  ///< Read Display Signal
    SLPIN     = 0x10U,  ///< Sleep in & booster off
    SLPOUT    = 0x11U,  ///< Sleep out & booster on
    PTLON     = 0x12U,  ///< Partial mode on
    NORON     = 0x13U,  ///< Partial off (Normal)
    INVOFF    = 0x20U,  ///< Display inversion off
    INVON     = 0x21U,  ///< Display inversion on
    GAMSET    = 0x26U,  ///< Gamma curve select
    DISPOFF   = 0x28U,  ///< Display off
    DISPON    = 0x29U,  ///< Display on
    CASET     = 0x2AU,  ///< Column address set
    RASET     = 0x2BU,  ///< Row address set
    RAMWR     = 0x2CU,  ///< Memory write
    RAMRD     = 0x2EU,  ///< Memory read
    PTLAR     = 0x30U,  ///< Partial start/end address set
    TEOFF     = 0x34U,  ///< Tearing effect line off
    TEON      = 0x35U,  ///< Tearing effect mode set & on
    MADCTL    = 0x36U,  ///< Memory data access control
    IDMOFF    = 0x38U,  ///< Idle mode off
    IDMON     = 0x39U,  ///< Idle mode on
    COLMOD    = 0x3AU,  ///< Interface pixel format
    RDID1     = 0xDAU,  ///< Read ID1
    RDID2     = 0xDBU,  ///< Read ID2
    RDID3     = 0xDCU,  ///< Read ID3
} ST7735register_e;

#endif
