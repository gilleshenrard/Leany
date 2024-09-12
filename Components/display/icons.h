#ifndef INC_HARDWARE_SCREEN_ICONS_H_
#define INC_HARDWARE_SCREEN_ICONS_H_
#include <stdint.h>

enum {
    TEST_PATTERN_SIZE = 32U,
    ICON_LINE_SIZE    = 32U * 2U,
};

extern const uint32_t testPattern[TEST_PATTERN_SIZE];

#endif
