#ifndef INC_HARDWARE_SCREEN_ICONS_H_
#define INC_HARDWARE_SCREEN_ICONS_H_
#include <stdint.h>

enum {
    VERDANA_NB_ROWS    = 49U,
    VERDANA_NB_COLUMNS = 39U,
};

typedef enum {
    VERDANA_0 = 0,
    VERDANA_1,
    VERDANA_2,
    VERDANA_3,
    VERDANA_4,
    VERDANA_5,
    VERDANA_6,
    VERDANA_7,
    VERDANA_8,
    VERDANA_9,
    VERDANA_PLUS,
    VERDANA_MIN,
    VERDANA_COMMA,
    VERDANA_DEG,
    NB_CHARACTERS,
} verdanaCharacter_e;

extern const uint64_t verdana_48ptBitmaps[NB_CHARACTERS][VERDANA_NB_ROWS];

#endif
