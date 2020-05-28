#include <stdint.h>
#include <stdlib.h>
#include "shared.hpp"

/* Get the position of a two dimensional flat array. */
#define ARRAY2_IDX(a,b,size) (a * size) + b

/* Get the index via the CUDA block and thread index. */
#define IDX(bIdx,bDim,tIdx,size) ARRAY2_IDX((bIdx.y * bDim.y + tIdx.y),((bIdx.x * bDim.x) + tIdx.x), size)

/* Return if the idx is out of bounds of the array size. */ 
#define IDX_GUARD(size_x, size_y, idx) if((size_x * size_y) <= idx) return

/* Basic inlined math operations for the rgb format. */
#define MAXRGB8(r,g,b) ((uint8_t) fmaxf(fmaxf((float)r, (float)g), (float)b))
#define MINRGB8(r,g,b) ((uint8_t) fminf(fminf((float)r, (float)g), (float)b))

/*
 * CUDA kernel for the grayscaling operation.
 */
__global__ void op_kernel_grey(uint32_t width, uint32_t height, uint32_t *in, uint32_t *out)
{
	uint32_t idx = IDX(blockIdx, blockDim, threadIdx, width);

	IDX_GUARD(width, height, idx);

	uint8_t color = 
		  (0.21 * RED8(in[idx]))
		+ (0.72 * GREEN8(in[idx]))
		+ (0.07 * BLUE8(in[idx]));

	uint8_t alpha = ALPHA8(in[idx]);

	out[idx] = RGBA32(color, color, color, alpha);
}
#define OP_KERNEL_GREY 1

/*
 * CUDA kernel for converting the rgba to hsv colorspace.
 */
 __global__ void op_kernel_hsv(uint32_t width, uint32_t height, uint32_t *in, uint32_t *out)
{
	uint32_t idx = IDX(blockIdx, blockDim, threadIdx, width);

	IDX_GUARD(width, height, idx);

	int32_t next = idx * 3; // hsv has only 3 channels
	
	/* Normalize color values except alpha. */
	uint8_t red = RED8(in[idx]);
	uint8_t green = GREEN8(in[idx]);
	uint8_t blue = BLUE8(in[idx]);

	/* Calulate conversion parameters */
	uint8_t cmax = MAXRGB8(red, green, blue);
	uint8_t cmin = MINRGB8(red, green, blue);
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
			hue = 171 + 43 * ((red - green) / diff);
	}

	/* Calculate saturation. */
	uint8_t saturation = 0;

	if(cmax != 0)
		saturation = 255 * diff / cmax;
	
	/* Calculate value. */
	uint8_t value = cmax;

	/* Write only three channels. */
	((uint8_t *)out)[next++] = hue;
	((uint8_t *)out)[next++] = saturation;
	((uint8_t *)out)[next++] = value;
}
#define OP_KERNEL_HSV 2

/*
 * CUDA kernel for the gaussian blur operation.
 */
 __global__ void op_kernel_blur(uint32_t width, uint32_t height, uint32_t *in, uint32_t *out)
{
	uint32_t x = (blockIdx.x * blockDim.x) + threadIdx.x;
	uint32_t y = (blockIdx.y * blockDim.y) + threadIdx.y;

	uint32_t idx = y * width + x;

	IDX_GUARD(width, height, idx);

	const int32_t filter_size = 5;
	const int32_t filter_pivot = filter_size / 2;

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

	float red = 0, green = 0, blue = 0, alpha = 0;

	for(int32_t filter_y = 0; filter_y < filter_size; ++filter_y)
	{
		int32_t filter_y_idx = x - filter_pivot + filter_y;

		if(filter_y_idx < 0 || filter_y_idx >= height)
			continue;

		for(int32_t filter_x = 0; filter_x < filter_size; ++filter_x)
		{
			int32_t filter_x_idx = y - filter_pivot + filter_x;

			if(filter_x_idx < 0 || filter_x_idx >= width)
				continue;

			int32_t filter_idx = ARRAY2_IDX(filter_y_idx, filter_x_idx, width);

			red += filter[filter_y][filter_x] * ((float)RED8(in[filter_idx]));
			green += filter[filter_y][filter_x] * ((float)GREEN8(in[filter_idx]));
			blue += filter[filter_y][filter_x] * ((float)BLUE8(in[filter_idx]));
			alpha += filter[filter_y][filter_x] * ((float)ALPHA8(in[filter_idx]));
		}
	}

	red = TRUNCATE_CHANNEL(red, filter_factor, filter_bias);
	green = TRUNCATE_CHANNEL(green, filter_factor, filter_bias);
	blue = TRUNCATE_CHANNEL(blue, filter_factor, filter_bias);
	alpha = TRUNCATE_CHANNEL(alpha, filter_factor, filter_bias);

	out[idx] = RGBA32((uint8_t)red, (uint8_t)green, (uint8_t)blue, (uint8_t)alpha);
}
#define OP_KERNEL_BLUR 3


/*
 * Get the number of CUDA blocks. 
 */
dim3 cuda_blocks(uint32_t x, uint32_t y, dim3 threads)
{
	dim3 blocks((x / threads.x + 1), (y / threads.y + 1));
	return blocks;
}

/*
 * Print CUDA error and return failure status code.
 */
#define CUDA_ERROR_CHECK(call) {\
	cudaError_t error = call;\
	if(error != cudaSuccess)\
	{\
		printf("CUDA error %d in %s line %d: %s", error, __FILE__, __LINE__, cudaGetErrorString(error));\
		return EXIT_FAILURE;\
	}\
}\

/*
 * Executes and distributes the specified kernel.
 */
int execute_cuda_kernel(uint32_t kernel, uint32_t width, uint32_t height, uint32_t *data)
{
	size_t size = sizeof(uint32_t) * width * height;

	uint32_t *in, *out; /* avoid undefined behaviour, see http://www.c-faq.com/ptrs/genericpp.html */

	/* Allocate CUDA buffers. */
	CUDA_ERROR_CHECK(cudaMalloc((void **) &in, size));
	CUDA_ERROR_CHECK(cudaMalloc((void **) &out, size));
	CUDA_ERROR_CHECK(cudaMemcpy(in, data, size, cudaMemcpyHostToDevice));

	/* Define distribution levels. */
	dim3 threads(32,32);
	dim3 blocks = cuda_blocks(width, height, threads);

	/* Execute the specified CUDA kernel. */
	switch(kernel)
	{
		case OP_KERNEL_GREY:
			op_kernel_grey<<<threads, blocks>>>(width, height, in, out);
			break;
		case OP_KERNEL_BLUR:
			op_kernel_blur<<<threads, blocks>>>(width, height, in, out);
			break;
		case OP_KERNEL_HSV:
			op_kernel_hsv<<<threads, blocks>>>(width, height, in, out);
			break;
		default:
			return EXIT_FAILURE;
	}

	/* Copy CUDA buffer back to source array and free allocated buffers. */
	CUDA_ERROR_CHECK(cudaMemcpy(data, out, size, cudaMemcpyDeviceToHost));
	CUDA_ERROR_CHECK(cudaFree(in));
	CUDA_ERROR_CHECK(cudaFree(out));

	return EXIT_SUCCESS;
}

/*
 * Greyscales the colors of the image.
 */
int op_grey(uint32_t width, uint32_t height, uint32_t *data)
{
	return execute_cuda_kernel(OP_KERNEL_GREY, width, height, data);
}

/*
 * Converts the colorspace from rgba to hsv.
 */
int op_hsv(uint32_t width, uint32_t height, uint32_t *data)
{
	return execute_cuda_kernel(OP_KERNEL_HSV, width, height, data);
}

/*
 * Applies a emboss filter to the image.
 */
int op_emboss(uint32_t width, uint32_t height, uint32_t *data)
{
	return execute_cuda_kernel(0, width, height, data);
}

/*
 * Applies a gaussian blur filter to the image.
 */
int op_blur(uint32_t width, uint32_t height, uint32_t *data)
{
	return execute_cuda_kernel(OP_KERNEL_BLUR, width, height, data);
}