#include <stdio.h>
#include <stdlib.h>
#include "shared.hpp"

/* Get the position of a two dimensional flat array. */
#define ARRAY2_IDX(a,b,size) (a * size) + b

/* Get the index via the CUDA block and thread index. */
#define IDX(bIdx,bDim,tIdx,size) ARRAY2_IDX((bIdx.y * bDim.y + tIdx.y),((bIdx.x * bDim.x) + tIdx.x), size)

/* Return if the idx is out of bounds of the array size. */ 
#define IDX_GUARD(size_x, size_y, idx) if((size_x * size_y) <= idx) return


/*
 * Get the number of CUDA threads. 
 */
dim3 cuda_threads()
{
	dim3 threads(32,32);
	return threads;
}

/*
 * Get the number of CUDA blocks. 
 */
dim3 cuda_blocks(uint32_t x, uint32_t y, dim3 threads)
{
	dim3 blocks((x / threads.x + 1), (y / threads.y + 1));
	return blocks;
}

/*
 * Allocate generic CUDA memory.
 */
uint32_t* allocate_cuda_buffer(uint32_t x, uint32_t y)
{
	size_t size = sizeof(uint32_t) * x * y;
	uint32_t *buffer;
	cudaMalloc((void **) &buffer, size);
	return buffer;
}

/*
 * Allocate generic CUDA memory and copy the given data to the memory.
 */
uint32_t* allocate_cuda__input_buffer(uint32_t x, uint32_t y, uint32_t *data)
{
	size_t size = sizeof(uint32_t) * x * y;
	uint32_t *buffer = allocate_cuda_buffer(x, y);
	cudaMemcpy(buffer, data, size, cudaMemcpyHostToDevice);
	return buffer;
}

/*
 * Greyscales the colors of the image.
 */
int op_grey(uint32_t width, uint32_t height, uint32_t *data)
{
	dim3 threads = cuda_threads();
	dim3 blocks = cuda_blocks(width, height, threads);
	uint32_t *in = allocate_cuda__input_buffer(width, height, data);
	uint32_t *out = allocate_cuda_buffer(width, height);

	op_kernel_grey<<<threads, blocks>>>(width, height, in, out);
	return EXIT_SUCCESS;
}

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


/*
 * Required filter information.
 */
#define filter_size 5
#define filter_pivot filter_size / 2
#define filter_factor = 1.0f / 256.0f;
#define filter_bias = 0.0f;
__device__ float filter[filter_size][filter_size];

/*
 * Applies a gaussian blur filter to the image.
 */
int op_blur(uint32_t width, uint32_t height, uint32_t *data)
{
	float filter_data[filter_size][filter_size] = 
	{
		{1.0f,  4.0f,  6.0f,  4.0f,  1.0f},
		{4.0f, 16.0f, 24.0f, 16.0f,  4.0f},
		{6.0f, 24.0f, 36.0f, 24.0f,  6.0f},
		{4.0f, 16.0f, 24.0f, 16.0f,  4.0f},
		{1.0f,  4.0f,  6.0f,  4.0f,  1.0f}
	};

	cudaMemcpyToSymbol(filter, filter_data, filter_size * filter_size * sizeof(float));

	dim3 threads = cuda_threads();
	dim3 blocks = cuda_blocks(width, height, threads);
	uint32_t *in = allocate_cuda__input_buffer(width, height, data);
	uint32_t *out = allocate_cuda_buffer(width, height);

	op_kernel_blur<<<threads, blocks>>>(width, height, in, out);

	return EXIT_SUCCESS;
}

#define TRUNCATE_CHANNEL(value,factor,bias) std::min(std::max(factor * value + bias, 0.0f), 255.0f)

/*
 * CUDA kernel for the gaussian blur operation.
 */
 __global__ void op_kernel_blur(uint32_t width, uint32_t height, uint32_t *in, uint32_t *out)
{
	uint32_t x = (blockIdx.x * blockDim.x) + threadIdx.x;
	uint32_t y = (blockIdx.y * blockDim.y) + threadIdx.y;

	uint32_t idx = y * width + x;

	IDX_GUARD(width, height, idx);

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

	out[index] = RGBA32((uint8_t)red, (uint8_t)green, (uint8_t)blue, (uint8_t)alpha);
}

int op_hsv(uint32_t width, uint32_t height, uint32_t *data)
{
	return EXIT_SUCCESS;
}

int op_emboss(uint32_t width, uint32_t height, uint32_t *data)
{
	return EXIT_SUCCESS;
}

