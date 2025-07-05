#ifndef HANGUL_FONT_H
#define HANGUL_FONT_H

#include <stdint.h>

#define HANGUL_FONT_WIDTH  16
#define HANGUL_FONT_HEIGHT 16

// Korean characters: 안녕하세요
typedef struct {
    uint16_t unicode;
    uint8_t bitmap[32];  // 16x16 bitmap (2 bytes per row)
} hangul_char_t;

extern const hangul_char_t hangul_chars[];
extern const int hangul_chars_count;

#endif // HANGUL_FONT_H