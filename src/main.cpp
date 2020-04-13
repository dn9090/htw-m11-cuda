#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <time.h>
#include <opencv2/opencv.hpp>
#include "shared.hpp"
#include "img_operations.hpp"


#define OPT(value, option) strcmp(value, option) == 0

using namespace std;
using namespace cv;

uint32_t* mat_to_flat_array(Mat& mat)
{
	uint32_t *im = (uint32_t *)malloc(sizeof(uint32_t) * (mat.rows * mat.cols));

	for (int32_t row = 0; row < mat.rows; ++row)
	{
		for (int32_t col = 0; col < mat.cols; ++col)
		{
			uint32_t channel = (row * mat.cols + col) * mat.channels();
			uint32_t index = (row * mat.cols) + col;

			uint8_t red = mat.data[channel];
			uint8_t green = mat.data[channel + 1];
			uint8_t blue = mat.data[channel + 2];
			uint8_t alpha = 0xFF;

			if(mat.channels() > 3)
				alpha = mat.data[channel + 3];

			im[index] = RGBA32(red, green, blue, alpha);
		}
	}

	return im;
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

	Mat mat_in = imread(argv[2], CV_LOAD_IMAGE_UNCHANGED);
	
	if(mat_in.empty())
	{
		printf("The file %s could not be loaded.\n", argv[2]);
		return EXIT_FAILURE;
	}

	printf("Loaded file %s with %d rows, %d columns and %d channels.\n", argv[2], mat_in.rows, mat_in.cols, mat_in.channels());

	uint32_t *im = mat_to_flat_array(mat_in);
	Mat mat_out;

	clock_t clock_start, clock_end;
	
	ios_base::sync_with_stdio(false); 

	if(OPT(argv[1], "grey"))
	{
		clock_start = clock();
		op_grey(mat_in.cols, mat_in.rows, im);
		clock_end = clock();

		mat_out = Mat(mat_in.rows, mat_in.cols, CV_8UC4, im);
	} else if(OPT(argv[1], "emboss")) {
		clock_start = clock();
		op_emboss(mat_in.cols, mat_in.rows, im);
		clock_end = clock();

		mat_out = Mat(mat_in.rows, mat_in.cols, CV_8UC4, im);
	} else if(OPT(argv[1], "blur")) {
		clock_start = clock();
		op_blur(mat_in.cols, mat_in.rows, im);
		clock_end = clock();

		mat_out = Mat(mat_in.rows, mat_in.cols, CV_8UC4, im);
	} else if(OPT(argv[1], "hsv")) {
		clock_start = clock();
		op_hsv(mat_in.cols, mat_in.rows, im);
		clock_end = clock();

		mat_out = Mat(mat_in.rows, mat_in.cols, CV_8UC3, im);
		Mat tmp = Mat(mat_in.rows, mat_in.cols, CV_8UC3);
		cvtColor(mat_out, tmp, COLOR_HSV2RGB);
		mat_out = tmp;
	}

	ios_base::sync_with_stdio(true);

	double cpu_time_taken = double(clock_end - clock_start) / CLOCKS_PER_SEC; 
	printf("The operation completed successfully in %f sec.\n", cpu_time_taken);

	std::vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);

	imwrite(argv[3], mat_out, compression_params);

	printf("Exported the result to %s.\n", argv[3]);

	return EXIT_SUCCESS;
}