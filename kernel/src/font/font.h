#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct font_desc_t {
    const char* data;
    uint16_t width;
	uint16_t height;
	uint16_t charCount;
} font_desc;

extern const font_desc defaultMonospace;

#endif