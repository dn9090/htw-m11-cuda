#pragma once

/*
 * Common bit operations for the rgba family.
 * The pixel format is specified as uint32_t 
 * with the color format as uint8_t.
 *
 * Position |    24 |   16 |     8 |   0 | 
 * Bit      |     8 |   8  |     8 |   8 |
 * Value    | alpha | blue | green | red |
 */

#define COLOR8_MASK 0xFF

/* Retrieves the 1 byte alpha channel. */
#define ALPHA8(p) (((p) >> 24) & COLOR8_MASK)

/* Retrieves the 1 byte blue channel. */
#define BLUE8(p) (((p) >> 16) & COLOR8_MASK)

/* Retrieves the 1 byte green channel. */
#define GREEN8(p) (((p) >> 8) & COLOR8_MASK)

/* Retrieves the 1 byte red channel. */
#define RED8(p) ((p) & COLOR8_MASK)

/* Combines the rgba channels to a 4 byte integer. */
#define RGBA32(r,g,b,a) ((((a) << 24)) | (((b) << 16)) | (((g) << 8)) | ((r)))

/* Truncate channel value to 1 byte. */ 
#define TRUNCATE_CHANNEL(value,factor,bias) std::min(std::max(factor * value + bias, 0.0f), 255.0f)
