#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <time.h>
#include "lodepng/lodepng.h"
#include "shared.hpp"
#include "img_operations.hpp"

#define OPT(value, option) strcmp(value, option) == 0

using namespace std;

int main(int argc, char **argv)
{
	if(argc < 4)
	{
		printf("usage: %s <grey|emboss|blur|hsv> <input file> <output file>\n\n", argv[0]);
		printf("convert image colors\n\tgrey\tconverts the colors to greyscale\n\thsv\tconverts the rgba to the hsv colorspace\n\n");
		printf("apply filter to image\n\temboss\tapplies the emboss filter\n\tblur\tblurs the image via a gaussian blur filter\n\n");
		return 0;
	}

	printf("The implementation uses lodepng.\n");

	std::vector<unsigned char> image;
	unsigned image_width, image_height;

	if(lodepng::decode(image, image_width, image_height, argv[2]))
	{
		printf("The file %s could not be loaded.\n", argv[2]);
		return EXIT_FAILURE;
	}

	uint32_t pixels = image_height * image_width;
	uint32_t *im = (uint32_t *)malloc(sizeof(uint32_t) * (pixels));
	int offset = 0;

	for(int i = 0; i < image.size(); i += 4)
	{
		uint8_t red = (uint8_t)image[i];
		uint8_t green = (uint8_t)image[i+1];
		uint8_t blue = (uint8_t)image[i+2];
		uint8_t alpha = (uint8_t)image[i+3];

		im[offset++] = RGBA32(red, green, blue, alpha);
	}

	uint32_t width = image_width;
	uint32_t height = image_height;
	uint32_t channels = 4;

	printf("Loaded file %s with %d rows, %d columns and %d channels.\n", argv[2], height, width, channels);

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

	if(success != EXIT_SUCCESS)
	{
		printf("The operation failed.\n");
		return EXIT_FAILURE;
	}

	double cpu_time_taken = double(clock_end - clock_start) / CLOCKS_PER_SEC; 
	printf("The operation completed successfully in %f sec.\n", cpu_time_taken);

	std::vector<unsigned char> output;

	for(int i = 0; i < pixels; ++i)
	{
		output.push_back(RED8(im[i]));
		output.push_back(GREEN8(im[i]));
		output.push_back(BLUE8(im[i]));
		output.push_back(ALPHA8(im[i]));
	}

	if(lodepng::encode(argv[3], output, width, height))
	{
		printf("Export failed.\n");
		return EXIT_FAILURE;
	}

	printf("Exported the result to %s.\n", argv[3]);

	return EXIT_SUCCESS;
}