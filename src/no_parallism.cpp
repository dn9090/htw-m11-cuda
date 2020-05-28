#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <cmath>
#include "shared.hpp"

/* Get the position of a two dimensional flat array. */
#define ARRAY2_IDX(a,b,size) (a * size) + b

using namespace std;

/*
 * Greyscales the colors of the image.
 */
int op_grey(uint32_t width, uint32_t height, uint32_t *data)
{
	for(int32_t row = height - 1; row >= 0; --row)
	{
		for(int32_t col = width - 1; col >= 0; --col)
		{
			uint32_t index = ARRAY2_IDX(row, col, width);
			
			uint8_t color = 
				  (0.21 * RED8(data[index]))
				+ (0.72 * GREEN8(data[index]))
				+ (0.07 * BLUE8(data[index]));

			uint8_t alpha = ALPHA8(data[index]);

			data[index] = RGBA32(color, color, color, alpha);
		}
	}

	return EXIT_SUCCESS;
}

/*
 * Converts the colorspace from rgba to hsv.
 */
int op_hsv(uint32_t width, uint32_t height, uint32_t *data)
{
	for(int32_t row = 0; row < height; ++row)
	{
		for(int32_t col = 0; col < width; ++col)
		{
			uint32_t index = ARRAY2_IDX(row, col, width);
			
			int32_t next = index * 3; // hsv has only 3 channels

			/* Normalize color values except alpha. */
			uint8_t red = RED8(data[index]);
			uint8_t green = GREEN8(data[index]);
			uint8_t blue = BLUE8(data[index]);

			/* Calulate conversion parameters */
			uint8_t cmax = MAXRGB(red, green, blue);
			uint8_t cmin = MINRGB(red, green, blue);
			uint8_t diff = cmax - cmin;

			/* Calculate hue. */
			uint8_t hue = 0;

			if(diff != 0)
			{
				if(cmax == red)
					hue = 43 * ((green - blue) / diff);
				else if(cmax == green)
					hue = 85 + 43 * ((blue - red) / diff);
				else
					hue = 200 + 171 + 43 * ((red - green) / diff); /* 200 is temporary. It works for some reason. */
			}

			/* Calculate saturation. */
			uint8_t saturation = cmax == 0 ? 0 : 255 * diff / cmax;
			
			/* Calculate value. */
			uint8_t value = cmax;

			/* Write only three channels. */
			((uint8_t *)data)[next++] = hue;
			((uint8_t *)data)[next++] = saturation;
			((uint8_t *)data)[next++] = value;
		}
	}

	return EXIT_SUCCESS;
}

/*
 * Applies a emboss filter to the image.
 */
int op_emboss(uint32_t width, uint32_t height, uint32_t *data)
{
	int32_t diff_max = 0, diff_tmp = 0;

	for(int32_t row = height - 1; row >= 0; --row)
	{
		for(int32_t col = width - 1; col >= 0; --col)
		{
			/* Save the highest difference. */
			if(diff_max > diff_tmp)
				diff_tmp = diff_max;
			else
				diff_max = diff_tmp;

			uint32_t top_left_index = ARRAY2_IDX(max(row - 1, 0), max(col - 1, 0), width);
			uint32_t index = ARRAY2_IDX(row, col, width);

			/* Calculate difference between color values. */
			int32_t diff_red = RED8(data[index]) - RED8(data[top_left_index]);
			int32_t diff_green = GREEN8(data[index]) - GREEN8(data[top_left_index]);
			int32_t diff_blue = BLUE8(data[index]) - BLUE8(data[top_left_index]);

			/* Find the maximum difference. */
			diff_max = max(diff_red, max(diff_green, diff_blue));

			/* Use the maximum difference as color value. */
			uint32_t color = 128 + diff_max;

			data[index] = RGBA32(
				(uint8_t)color,
				(uint8_t)color,
				(uint8_t)color,
				ALPHA8(data[index])
			);
		}
	}

	return EXIT_SUCCESS;
}

/*
 * Applies a gaussian blur filter to the image.
 * Works with different filters.
 * Credits: https://lodev.org/cgtutor/filtering.html 
 */
int op_blur(uint32_t width, uint32_t height, uint32_t *data)
{
	int32_t filter_size = 5;
	int32_t filter_pivot = filter_size / 2;

	/*
	double filter[filter_size][filter_size] =
	{
		{0.077847, 0.123317, 0.077847},
		{0.123317, 0.195346, 0.123317},
		{0.077847, 0.123317, 0.077847}
	};
	*/

	float filter[filter_size][filter_size] =
	{
		{1.0f,  4.0f,  6.0f,  4.0f,  1.0f},
		{4.0f, 16.0f, 24.0f, 16.0f,  4.0f},
		{6.0f, 24.0f, 36.0f, 24.0f,  6.0f},
		{4.0f, 16.0f, 24.0f, 16.0f,  4.0f},
		{1.0f,  4.0f,  6.0f,  4.0f,  1.0f}
	};

	float filter_factor = 1.0f / 256.0f;
	float filter_bias = 0.0f;

	for(int32_t row = height - 1; row >= 0; --row)
	{
		for(int32_t col = width - 1; col >= 0; --col)
		{
			uint32_t index = ARRAY2_IDX(row, col, width);

			float red = 0, green = 0, blue = 0, alpha = 0;

			for(int32_t filter_y = 0; filter_y < filter_size; ++filter_y)
			{
				int32_t filter_y_idx = row - filter_pivot + filter_y;

				if(filter_y_idx < 0 || filter_y_idx >= height)
					continue;

				for(int32_t filter_x = 0; filter_x < filter_size; ++filter_x)
				{
					int32_t filter_x_idx = col - filter_pivot + filter_x;

					if(filter_x_idx < 0 || filter_x_idx >= width)
						continue;

					int32_t filter_idx = ARRAY2_IDX(filter_y_idx, filter_x_idx, width);

					red += filter[filter_y][filter_x] * ((float)RED8(data[filter_idx]));
					green += filter[filter_y][filter_x] * ((float)GREEN8(data[filter_idx]));
					blue += filter[filter_y][filter_x] * ((float)BLUE8(data[filter_idx]));
					alpha += filter[filter_y][filter_x] * ((float)ALPHA8(data[filter_idx]));
				}
			}

			red = TRUNCATE_CHANNEL(red, filter_factor, filter_bias);
			green = TRUNCATE_CHANNEL(green, filter_factor, filter_bias);
			blue = TRUNCATE_CHANNEL(blue, filter_factor, filter_bias);
			alpha = TRUNCATE_CHANNEL(alpha, filter_factor, filter_bias);

			data[index] = RGBA32((uint8_t)red, (uint8_t)green, (uint8_t)blue, (uint8_t)alpha);
		}
	}

	return EXIT_SUCCESS;
}