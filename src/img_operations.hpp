#pragma once

#include <stdlib.h>
#include <stdint.h>

using namespace std;

extern int op_grey(uint32_t width, uint32_t height, uint32_t *data);

extern int op_hsv(uint32_t width, uint32_t height, uint32_t *data);

extern int op_emboss(uint32_t width, uint32_t height, uint32_t *data);

extern int op_blur(uint32_t width, uint32_t height, uint32_t *data);