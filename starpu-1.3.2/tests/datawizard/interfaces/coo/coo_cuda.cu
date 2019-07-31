/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2012                                     Inria
 * Copyright (C) 2012,2015,2017                           CNRS
 *
 * StarPU is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * StarPU is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU Lesser General Public License in COPYING.LGPL for more details.
 */
#include <starpu.h>
#include "../test_interfaces.h"

extern struct test_config coo_config;

__global__ void coo_cuda(int *val, uint32_t n, int *err, int factor)
{
        unsigned i =  blockIdx.x*blockDim.x + threadIdx.x;

	if (i >= n)
		return;

	if (val[i] != i * factor)
		*err = 1;
	else
		val[i] *= -1;
}

extern "C" void test_coo_cuda_func(void *buffers[], void *args)
{
	int factor;
	int *ret;
	int *val;
	cudaError_t error;
	uint32_t nvalues = STARPU_COO_GET_NVALUES(buffers[0]);
	unsigned threads_per_block = 64;
	unsigned nblocks = (nvalues + threads_per_block-1) / threads_per_block;

	factor = *(int *) args;
	val = (int *) STARPU_COO_GET_VALUES(buffers[0]);

	error = cudaMalloc(&ret, sizeof(int));
	if (error != cudaSuccess)
		STARPU_CUDA_REPORT_ERROR(error);

	error = cudaMemcpyAsync(ret,
			   &coo_config.copy_failed,
			   sizeof(int),
			   cudaMemcpyHostToDevice, starpu_cuda_get_local_stream());
	if (error != cudaSuccess)
		STARPU_CUDA_REPORT_ERROR(error);

        coo_cuda<<<nblocks,threads_per_block,2,starpu_cuda_get_local_stream()>>>
		(val, nvalues, ret, factor);

	error = cudaMemcpyAsync(&coo_config.copy_failed,
			   ret,
			   sizeof(int),
			   cudaMemcpyDeviceToHost, starpu_cuda_get_local_stream());
	if (error != cudaSuccess)
		STARPU_CUDA_REPORT_ERROR(error);

	cudaFree(ret);
	cudaStreamSynchronize(starpu_cuda_get_local_stream());
}