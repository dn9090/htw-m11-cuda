#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <time.h>
#define cimg_display 0
#include "CImg.h"
#include "shared.hpp"
#include "img_operations.hpp"


#define OPT(value, option) strcmp(value, option) == 0

using namespace std;
using namespace cimg_library;

uint32_t* img_to_flat_array(CImg<unsigned char>& img)
{
	uint32_t *im = (uint32_t *)malloc(sizeof(uint32_t) * (img.height() * img.width()));

	for (int32_t row = 0; row < img.height(); ++row)
	{
		for (int32_t col = 0; col < img.width(); ++col)
		{
			uint32_t index = (row * img.width()) + col;

			uint8_t red = (uint8_t)img(col, row, 0, 0);
			uint8_t green = (uint8_t)img(col, row, 0, 1);
			uint8_t blue = (uint8_t)img(col, row, 0, 2);
			uint8_t alpha = 0xFF;

			if(img.spectrum() > 3)
				alpha = (uint8_t)img(col, row, 0, 3);

			im[index] = RGBA32(red, green, blue, alpha);
		}
	}

	return im;
}

int replace_img_with_flat_array(CImg<unsigned char>& img, uint32_t *data)
{
	for (int32_t row = 0; row < img.height(); ++row)
	{
		for (int32_t col = 0; col < img.width(); ++col)
		{
			uint32_t index = (row * img.width()) + col;

			uint8_t red = RED8(data[index]);
			uint8_t green = GREEN8(data[index]);
			uint8_t blue = BLUE8(data[index]);
			uint8_t alpha = ALPHA8(data[index]);

			img(col, row, 0, 0) = red;
			img(col, row, 0, 1) = green;
			img(col, row, 0, 2) = blue;

			if(img.spectrum() > 3)
				img(col, row, 0, 3) = alpha;
		}
	}

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	if(argc < 4)
	{
		printf("usage: %s <grey|emboss|blur|hsv> <input file> <output file>\n\n", argv[0]);
		printf("convert image colors\n\tgrey\tconverts the colors to greyscale\n\thsv\tconverts the rgba to the hsv colorspace\n\n");
		printf("apply filter to image\n\temboss\tapplies the emboss filter\n\tblur\tblurs the image via a gaussian blur filter\n\n");
		return 0;
	}

	CImg<unsigned char> img(argv[2]);

	uint32_t width = img.width();
	uint32_t height = img.height();
	uint32_t channels = img.spectrum();

	printf("Loaded file %s with %d rows, %d columns and %d channels.\n", argv[2], height, width, channels);

	uint32_t *im = img_to_flat_array(img);

	clock_t clock_start, clock_end;

	int success = EXIT_FAILURE;

	if(OPT(argv[1], "grey"))
	{
		clock_start = clock();
		success = op_grey(width, height, im);
		clock_end = clock();
	} else if(OPT(argv[1], "emboss")) {
		clock_start = clock();
		success = op_emboss(width, height, im);
		clock_end = clock();
	} else if(OPT(argv[1], "blur")) {
		clock_start = clock();
		success = op_blur(width, height, im);
		clock_end = clock();
	} else if(OPT(argv[1], "hsv")) {
		clock_start = clock();
		success = op_hsv(width, height, im);
		clock_end = clock();
	} else {
		printf("The operation %s is not available.\n", argv[1]);
		return EXIT_FAILURE;
	}

	replace_img_with_flat_array(img, im);
	
	if(success != EXIT_SUCCESS)
	{
		printf("The operation failed.\n");
		return EXIT_FAILURE;
	}

	double cpu_time_taken = double(clock_end - clock_start) / CLOCKS_PER_SEC; 
	printf("The operation completed successfully in %f sec.\n", cpu_time_taken);

	img.save(argv[3]);

	printf("Exported the result to %s.\n", argv[3]);

	return EXIT_SUCCESS;
}