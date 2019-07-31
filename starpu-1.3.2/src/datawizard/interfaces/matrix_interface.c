/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2008-2019                                Université de Bordeaux
 * Copyright (C) 2011,2012,2017                           Inria
 * Copyright (C) 2010-2017,2019                           CNRS
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

#ifdef STARPU_USE_CUDA
/* At least CUDA 4.2 still didn't have working memcpy3D */
#if CUDART_VERSION < 5000
#define BUGGED_MEMCPY3D
#endif

static int copy_ram_to_cuda(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED);
static int copy_cuda_to_ram(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED);
static int copy_cuda_to_cuda(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED);
static int copy_ram_to_cuda_async(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, cudaStream_t stream);
static int copy_cuda_to_ram_async(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, cudaStream_t stream);
#ifndef BUGGED_MEMCPY3D
static int copy_cuda_to_cuda_async(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, cudaStream_t stream);
#endif
#endif
#ifdef STARPU_USE_OPENCL
static int copy_ram_to_opencl(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED);
static int copy_opencl_to_ram(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED);
static int copy_opencl_to_opencl(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED);
static int copy_ram_to_opencl_async(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, cl_event *event);
static int copy_opencl_to_ram_async(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, cl_event *event);
static int copy_opencl_to_opencl_async(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, cl_event *event);
#endif
static int copy_any_to_any(void *src_interface, unsigned src_node, void *dst_interface, unsigned dst_node, void *async_data);

static const struct starpu_data_copy_methods matrix_copy_data_methods_s =
{
#ifdef STARPU_USE_CUDA
	.ram_to_cuda = copy_ram_to_cuda,
	.cuda_to_ram = copy_cuda_to_ram,
	.ram_to_cuda_async = copy_ram_to_cuda_async,
	.cuda_to_ram_async = copy_cuda_to_ram_async,
	.cuda_to_cuda = copy_cuda_to_cuda,
#ifndef BUGGED_MEMCPY3D
	.cuda_to_cuda_async = copy_cuda_to_cuda_async,
#endif
#else
#ifdef STARPU_SIMGRID
#ifndef BUGGED_MEMCPY3D
	/* Enable GPU-GPU transfers in simgrid */
	.cuda_to_cuda_async = (void *)1,
#endif
#endif
#endif
#ifdef STARPU_USE_OPENCL
	.ram_to_opencl = copy_ram_to_opencl,
	.opencl_to_ram = copy_opencl_to_ram,
	.opencl_to_opencl = copy_opencl_to_opencl,
        .ram_to_opencl_async = copy_ram_to_opencl_async,
	.opencl_to_ram_async = copy_opencl_to_ram_async,
	.opencl_to_opencl_async = copy_opencl_to_opencl_async,
#endif
	.any_to_any = copy_any_to_any,
};

static void matrix_init(void *data_interface);
static void register_matrix_handle(starpu_data_handle_t handle, unsigned home_node, void *data_interface);
static void *matrix_to_pointer(void *data_interface, unsigned node);
static int matrix_pointer_is_inside(void *data_interface, unsigned node, void *ptr);
static starpu_ssize_t allocate_matrix_buffer_on_node(void *data_interface_, unsigned dst_node);
static void free_matrix_buffer_on_node(void *data_interface, unsigned node);
static size_t matrix_interface_get_size(starpu_data_handle_t handle);
static size_t matrix_interface_get_alloc_size(starpu_data_handle_t handle);
static uint32_t footprint_matrix_interface_crc32(starpu_data_handle_t handle);
static uint32_t alloc_footprint_matrix_interface_crc32(starpu_data_handle_t handle);
static int matrix_compare(void *data_interface_a, void *data_interface_b);
static int matrix_alloc_compare(void *data_interface_a, void *data_interface_b);
static void display_matrix_interface(starpu_data_handle_t handle, FILE *f);
static int pack_matrix_handle(starpu_data_handle_t handle, unsigned node, void **ptr, starpu_ssize_t *count);
static int unpack_matrix_handle(starpu_data_handle_t handle, unsigned node, void *ptr, size_t count);
static starpu_ssize_t describe(void *data_interface, char *buf, size_t size);

struct starpu_data_interface_ops starpu_interface_matrix_ops =
{
	.init = matrix_init,
	.register_data_handle = register_matrix_handle,
	.allocate_data_on_node = allocate_matrix_buffer_on_node,
	.to_pointer = matrix_to_pointer,
	.pointer_is_inside = matrix_pointer_is_inside,
	.free_data_on_node = free_matrix_buffer_on_node,
	.copy_methods = &matrix_copy_data_methods_s,
	.get_size = matrix_interface_get_size,
	.get_alloc_size = matrix_interface_get_alloc_size,
	.footprint = footprint_matrix_interface_crc32,
	.alloc_footprint = alloc_footprint_matrix_interface_crc32,
	.compare = matrix_compare,
	.alloc_compare = matrix_alloc_compare,
	.interfaceid = STARPU_MATRIX_INTERFACE_ID,
	.interface_size = sizeof(struct starpu_matrix_interface),
	.display = display_matrix_interface,
	.pack_data = pack_matrix_handle,
	.unpack_data = unpack_matrix_handle,
	.describe = describe,
	.name = "STARPU_MATRIX_INTERFACE"
};

static void matrix_init(void *data_interface)
{
	struct starpu_matrix_interface *matrix_interface = data_interface;
	matrix_interface->allocsize = -1;
}

static void register_matrix_handle(starpu_data_handle_t handle, unsigned home_node, void *data_interface)
{
	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *) data_interface;

	unsigned node;
	for (node = 0; node < STARPU_MAXNODES; node++)
	{
		struct starpu_matrix_interface *local_interface = (struct starpu_matrix_interface *)
			starpu_data_get_interface_on_node(handle, node);

		if (node == home_node)
		{
			local_interface->ptr = matrix_interface->ptr;
                        local_interface->dev_handle = matrix_interface->dev_handle;
                        local_interface->offset = matrix_interface->offset;
			local_interface->ld  = matrix_interface->ld;
		}
		else
		{
			local_interface->ptr = 0;
			local_interface->dev_handle = 0;
			local_interface->offset = 0;
			local_interface->ld  = 0;
		}

		local_interface->id = matrix_interface->id;
		local_interface->nx = matrix_interface->nx;
		local_interface->ny = matrix_interface->ny;
		local_interface->elemsize = matrix_interface->elemsize;
		local_interface->allocsize  = matrix_interface->allocsize;
	}
}

static void *matrix_to_pointer(void *data_interface, unsigned node)
{
	(void) node;
	struct starpu_matrix_interface *matrix_interface = data_interface;

	return (void*) matrix_interface->ptr;
}

static int matrix_pointer_is_inside(void *data_interface, unsigned node, void *ptr)
{
	(void) node;
	struct starpu_matrix_interface *matrix_interface = data_interface;

	return (char*) ptr >= (char*) matrix_interface->ptr &&
		(char*) ptr < (char*) matrix_interface->ptr + matrix_interface->nx*matrix_interface->ny*matrix_interface->elemsize;
}


/* declare a new data with the matrix interface */
void starpu_matrix_data_register_allocsize(starpu_data_handle_t *handleptr, int home_node,
					   uintptr_t ptr, uint32_t ld, uint32_t nx,
					   uint32_t ny, size_t elemsize, size_t allocsize)
{
	struct starpu_matrix_interface matrix_interface =
	{
		.id = STARPU_MATRIX_INTERFACE_ID,
		.ptr = ptr,
		.ld = ld,
		.nx = nx,
		.ny = ny,
		.elemsize = elemsize,
                .dev_handle = ptr,
                .offset = 0,
		.allocsize = allocsize,
	};
#ifndef STARPU_SIMGRID
	if (home_node >= 0 && starpu_node_get_kind(home_node) == STARPU_CPU_RAM)
	{
		STARPU_ASSERT_ACCESSIBLE(ptr);
		STARPU_ASSERT_ACCESSIBLE(ptr + (ny-1)*ld*elemsize + nx*elemsize - 1);
	}
#endif

	starpu_data_register(handleptr, home_node, &matrix_interface, &starpu_interface_matrix_ops);
}

void starpu_matrix_data_register(starpu_data_handle_t *handleptr, int home_node,
				 uintptr_t ptr, uint32_t ld, uint32_t nx,
				 uint32_t ny, size_t elemsize)
{
	starpu_matrix_data_register_allocsize(handleptr, home_node, ptr, ld, nx, ny, elemsize, nx * ny * elemsize);
}

void starpu_matrix_ptr_register(starpu_data_handle_t handle, unsigned node,
				uintptr_t ptr, uintptr_t dev_handle, size_t offset, uint32_t ld)
{
	struct starpu_matrix_interface *matrix_interface = starpu_data_get_interface_on_node(handle, node);
	starpu_data_ptr_register(handle, node);
	matrix_interface->ptr = ptr;
	matrix_interface->dev_handle = dev_handle;
	matrix_interface->offset = offset;
	matrix_interface->ld = ld;
}

static uint32_t footprint_matrix_interface_crc32(starpu_data_handle_t handle)
{
	return starpu_hash_crc32c_be(starpu_matrix_get_nx(handle), starpu_matrix_get_ny(handle));
}

static uint32_t alloc_footprint_matrix_interface_crc32(starpu_data_handle_t handle)
{
	return starpu_hash_crc32c_be(starpu_matrix_get_allocsize(handle), 0);
}

static int matrix_compare(void *data_interface_a, void *data_interface_b)
{
	struct starpu_matrix_interface *matrix_a = (struct starpu_matrix_interface *) data_interface_a;
	struct starpu_matrix_interface *matrix_b = (struct starpu_matrix_interface *) data_interface_b;

	/* Two matricess are considered compatible if they have the same size */
	return (matrix_a->nx == matrix_b->nx)
		&& (matrix_a->ny == matrix_b->ny)
		&& (matrix_a->elemsize == matrix_b->elemsize);
}

static int matrix_alloc_compare(void *data_interface_a, void *data_interface_b)
{
	struct starpu_matrix_interface *matrix_a = (struct starpu_matrix_interface *) data_interface_a;
	struct starpu_matrix_interface *matrix_b = (struct starpu_matrix_interface *) data_interface_b;

	/* Two matricess are considered compatible if they have the same size */
	return (matrix_a->allocsize == matrix_b->allocsize);
}

static void display_matrix_interface(starpu_data_handle_t handle, FILE *f)
{
	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, STARPU_MAIN_RAM);

	fprintf(f, "%u\t%u\t", matrix_interface->nx, matrix_interface->ny);
}

static int pack_matrix_handle(starpu_data_handle_t handle, unsigned node, void **ptr, starpu_ssize_t *count)
{
	STARPU_ASSERT(starpu_data_test_if_allocated_on_node(handle, node));

	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, node);

	*count = matrix_interface->nx*matrix_interface->ny*matrix_interface->elemsize;

	if (ptr != NULL)
	{
		uint32_t y;
		char *matrix = (void *)matrix_interface->ptr;

		*ptr = (void *)starpu_malloc_on_node_flags(node, *count, 0);

		char *cur = *ptr;
		for(y=0 ; y<matrix_interface->ny ; y++)
		{
			memcpy(cur, matrix, matrix_interface->nx*matrix_interface->elemsize);
			cur += matrix_interface->nx*matrix_interface->elemsize;
			matrix += matrix_interface->ld * matrix_interface->elemsize;
		}
	}

	return 0;
}

static int unpack_matrix_handle(starpu_data_handle_t handle, unsigned node, void *ptr, size_t count)
{
	STARPU_ASSERT(starpu_data_test_if_allocated_on_node(handle, node));

	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, node);

	STARPU_ASSERT(count == matrix_interface->elemsize * matrix_interface->nx * matrix_interface->ny);

	uint32_t y;
	char *cur = ptr;
	char *matrix = (void *)matrix_interface->ptr;
	for(y=0 ; y<matrix_interface->ny ; y++)
	{
		memcpy(matrix, cur, matrix_interface->nx*matrix_interface->elemsize);
		cur += matrix_interface->nx*matrix_interface->elemsize;
		matrix += matrix_interface->ld * matrix_interface->elemsize;
	}

	starpu_free_on_node_flags(node, (uintptr_t)ptr, count, 0);

	return 0;
}

static size_t matrix_interface_get_size(starpu_data_handle_t handle)
{
	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, STARPU_MAIN_RAM);

#ifdef STARPU_DEBUG
	STARPU_ASSERT_MSG(matrix_interface->id == STARPU_MATRIX_INTERFACE_ID, "Error. The given data is not a matrix.");
#endif

	return matrix_interface->nx * matrix_interface->ny * matrix_interface->elemsize;
}

static size_t matrix_interface_get_alloc_size(starpu_data_handle_t handle)
{
	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, STARPU_MAIN_RAM);

#ifdef STARPU_DEBUG
	STARPU_ASSERT_MSG(matrix_interface->id == STARPU_MATRIX_INTERFACE_ID, "Error. The given data is not a matrix.");
#endif

	STARPU_ASSERT_MSG(matrix_interface->allocsize != (size_t)-1, "The matrix allocation size needs to be defined");

	return matrix_interface->allocsize;
}

/* offer an access to the data parameters */
uint32_t starpu_matrix_get_nx(starpu_data_handle_t handle)
{
	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, STARPU_MAIN_RAM);

#ifdef STARPU_DEBUG
	STARPU_ASSERT_MSG(matrix_interface->id == STARPU_MATRIX_INTERFACE_ID, "Error. The given data is not a matrix.");
#endif

	return matrix_interface->nx;
}

uint32_t starpu_matrix_get_ny(starpu_data_handle_t handle)
{
	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, STARPU_MAIN_RAM);

#ifdef STARPU_DEBUG
	STARPU_ASSERT_MSG(matrix_interface->id == STARPU_MATRIX_INTERFACE_ID, "Error. The given data is not a matrix.");
#endif

	return matrix_interface->ny;
}

uint32_t starpu_matrix_get_local_ld(starpu_data_handle_t handle)
{
	unsigned node;
	node = starpu_worker_get_local_memory_node();

	STARPU_ASSERT(starpu_data_test_if_allocated_on_node(handle, node));

	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, node);

#ifdef STARPU_DEBUG
	STARPU_ASSERT_MSG(matrix_interface->id == STARPU_MATRIX_INTERFACE_ID, "Error. The given data is not a matrix.");
#endif

	return matrix_interface->ld;
}

uintptr_t starpu_matrix_get_local_ptr(starpu_data_handle_t handle)
{
	unsigned node;
	node = starpu_worker_get_local_memory_node();

	STARPU_ASSERT(starpu_data_test_if_allocated_on_node(handle, node));

	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, node);

#ifdef STARPU_DEBUG
	STARPU_ASSERT_MSG(matrix_interface->id == STARPU_MATRIX_INTERFACE_ID, "Error. The given data is not a matrix.");
#endif

	return matrix_interface->ptr;
}

size_t starpu_matrix_get_elemsize(starpu_data_handle_t handle)
{
	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, STARPU_MAIN_RAM);

#ifdef STARPU_DEBUG
	STARPU_ASSERT_MSG(matrix_interface->id == STARPU_MATRIX_INTERFACE_ID, "Error. The given data is not a matrix.");
#endif

	return matrix_interface->elemsize;
}

size_t starpu_matrix_get_allocsize(starpu_data_handle_t handle)
{
	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *)
		starpu_data_get_interface_on_node(handle, STARPU_MAIN_RAM);

#ifdef STARPU_DEBUG
	STARPU_ASSERT_MSG(matrix_interface->id == STARPU_MATRIX_INTERFACE_ID, "Error. The given data is not a matrix.");
#endif

	return matrix_interface->allocsize;
}

/* memory allocation/deallocation primitives for the matrix interface */

/* returns the size of the allocated area */
static starpu_ssize_t allocate_matrix_buffer_on_node(void *data_interface_, unsigned dst_node)
{
	uintptr_t addr = 0, handle;

	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *) data_interface_;

	uint32_t ld = matrix_interface->nx; // by default

	starpu_ssize_t allocated_memory = matrix_interface->allocsize;
	handle = starpu_malloc_on_node(dst_node, allocated_memory);

	if (!handle)
		return -ENOMEM;

	if (starpu_node_get_kind(dst_node) != STARPU_OPENCL_RAM)
		addr = handle;

	/* update the data properly in consequence */
	matrix_interface->ptr = addr;
	matrix_interface->dev_handle = handle;
	matrix_interface->offset = 0;
	matrix_interface->ld = ld;

	return allocated_memory;
}

static void free_matrix_buffer_on_node(void *data_interface, unsigned node)
{
	struct starpu_matrix_interface *matrix_interface = (struct starpu_matrix_interface *) data_interface;

	starpu_free_on_node(node, matrix_interface->dev_handle, matrix_interface->allocsize);
}

#ifdef STARPU_USE_CUDA
static int copy_cuda_common(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, enum cudaMemcpyKind kind, int is_async, cudaStream_t stream)
{
	struct starpu_matrix_interface *src_matrix = src_interface;
	struct starpu_matrix_interface *dst_matrix = dst_interface;

	size_t elemsize = src_matrix->elemsize;
	cudaError_t cures;

	if (is_async)
	{
		double start;
		starpu_interface_start_driver_copy_async(src_node, dst_node, &start);
		cures = cudaMemcpy2DAsync((char *)dst_matrix->ptr, dst_matrix->ld*elemsize,
			(char *)src_matrix->ptr, src_matrix->ld*elemsize,
			src_matrix->nx*elemsize, src_matrix->ny, kind, stream);
		starpu_interface_end_driver_copy_async(src_node, dst_node, start);
		if (!cures)
			return -EAGAIN;
	}

	cures = cudaMemcpy2D((char *)dst_matrix->ptr, dst_matrix->ld*elemsize,
		(char *)src_matrix->ptr, src_matrix->ld*elemsize,
		src_matrix->nx*elemsize, src_matrix->ny, kind);
	if (!cures)
		cures = cudaThreadSynchronize();
	if (STARPU_UNLIKELY(cures))
	{
		int ret = copy_any_to_any(src_interface, src_node, dst_interface, dst_node, (void*)(uintptr_t)is_async);
		if (ret == -EAGAIN) return ret;
		if (ret) STARPU_CUDA_REPORT_ERROR(cures);
	}

	starpu_interface_data_copy(src_node, dst_node, (size_t)src_matrix->nx*src_matrix->ny*src_matrix->elemsize);

	return 0;
}

#ifndef BUGGED_MEMCPY3D
static int copy_cuda_peer(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, int is_async, cudaStream_t stream)
{
#ifdef STARPU_HAVE_CUDA_MEMCPY_PEER
	struct starpu_matrix_interface *src_matrix = src_interface;
	struct starpu_matrix_interface *dst_matrix = dst_interface;

	size_t elemsize = src_matrix->elemsize;
	cudaError_t cures;

	int src_dev = starpu_memory_node_get_devid(src_node);
	int dst_dev = starpu_memory_node_get_devid(dst_node);

	struct cudaMemcpy3DPeerParms p;
	memset(&p, 0, sizeof(p));

	p.srcDevice = src_dev;
	p.dstDevice = dst_dev;
	p.srcPtr = make_cudaPitchedPtr((char *)src_matrix->ptr, src_matrix->ld * elemsize, src_matrix->nx, src_matrix->ny);
	p.dstPtr = make_cudaPitchedPtr((char *)dst_matrix->ptr, dst_matrix->ld * elemsize, dst_matrix->nx, dst_matrix->ny);
	p.extent = make_cudaExtent(src_matrix->nx * elemsize, src_matrix->ny, 1);

	if (is_async)
	{
		double start;
		starpu_interface_start_driver_copy_async(src_node, dst_node, &start);
		cures = cudaMemcpy3DPeerAsync(&p, stream);
		starpu_interface_end_driver_copy_async(src_node, dst_node, start);
		if (!cures)
			return -EAGAIN;
	}

	cures = cudaMemcpy3DPeer(&p);
	if (!cures)
		cures = cudaThreadSynchronize();
	if (STARPU_UNLIKELY(cures))
		STARPU_CUDA_REPORT_ERROR(cures);

	starpu_interface_data_copy(src_node, dst_node, (size_t)src_matrix->nx*src_matrix->ny*src_matrix->elemsize);

	return 0;
#else
	STARPU_ABORT_MSG("CUDA memcpy 3D peer not available, but core triggered one ?!");
#endif
}
#endif

static int copy_cuda_to_ram(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED)
{
	return copy_cuda_common(src_interface, src_node, dst_interface, dst_node, cudaMemcpyDeviceToHost, 0, 0);
}

static int copy_ram_to_cuda(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED)
{
	return copy_cuda_common(src_interface, src_node, dst_interface, dst_node, cudaMemcpyHostToDevice, 0, 0);
}

static int copy_cuda_to_cuda(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED)
{
	if (src_node == dst_node)
		return copy_cuda_common(src_interface, src_node, dst_interface, dst_node, cudaMemcpyDeviceToDevice, 0, 0);
	else
#ifdef BUGGED_MEMCPY3D
		STARPU_ABORT_MSG("CUDA memcpy 3D peer not available, but core triggered one?!");
#else
		return copy_cuda_peer(src_interface, src_node, dst_interface, dst_node, 0, 0);
#endif
}

static int copy_cuda_to_ram_async(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, cudaStream_t stream)
{
	return copy_cuda_common(src_interface, src_node, dst_interface, dst_node, cudaMemcpyDeviceToHost, 1, stream);
}

static int copy_ram_to_cuda_async(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, cudaStream_t stream)
{
	return copy_cuda_common(src_interface, src_node, dst_interface, dst_node, cudaMemcpyHostToDevice, 1, stream);
}

#ifndef BUGGED_MEMCPY3D
static int copy_cuda_to_cuda_async(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED, cudaStream_t stream)
{
	if (src_node == dst_node)
		return copy_cuda_common(src_interface, src_node, dst_interface, dst_node, cudaMemcpyDeviceToDevice, 1, stream);
	else
		return copy_cuda_peer(src_interface, src_node, dst_interface, dst_node, 1, stream);
}
#endif
#endif // STARPU_USE_CUDA

#ifdef STARPU_USE_OPENCL
static int copy_opencl_common(void *src_interface, unsigned src_node, void *dst_interface, unsigned dst_node, cl_event *event)
{
	struct starpu_matrix_interface *src_matrix = src_interface;
	struct starpu_matrix_interface *dst_matrix = dst_interface;
        int ret;

	STARPU_ASSERT_MSG((src_matrix->ld == src_matrix->nx) && (dst_matrix->ld == dst_matrix->nx), "XXX non contiguous buffers are not properly supported in OpenCL yet. (TODO)");

	ret = starpu_opencl_copy_async_sync(src_matrix->dev_handle, src_matrix->offset, src_node,
					    dst_matrix->dev_handle, dst_matrix->offset, dst_node,
					    src_matrix->nx*src_matrix->ny*src_matrix->elemsize,
					    event);

	starpu_interface_data_copy(src_node, dst_node, src_matrix->nx*src_matrix->ny*src_matrix->elemsize);

	return ret;
}

static int copy_ram_to_opencl_async(void *src_interface, unsigned src_node, void *dst_interface, unsigned dst_node, cl_event *event)
{
	return copy_opencl_common(src_interface, src_node, dst_interface, dst_node, event);
}

static int copy_opencl_to_ram_async(void *src_interface, unsigned src_node, void *dst_interface, unsigned dst_node, cl_event *event)
{
	return copy_opencl_common(src_interface, src_node, dst_interface, dst_node, event);
}

static int copy_opencl_to_opencl_async(void *src_interface, unsigned src_node, void *dst_interface, unsigned dst_node, cl_event *event)
{
	return copy_opencl_common(src_interface, src_node, dst_interface, dst_node, event);
}

static int copy_ram_to_opencl(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED)
{
        return copy_ram_to_opencl_async(src_interface, src_node, dst_interface, dst_node, NULL);
}

static int copy_opencl_to_ram(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED)
{
        return copy_opencl_to_ram_async(src_interface, src_node, dst_interface, dst_node, NULL);
}

static int copy_opencl_to_opencl(void *src_interface, unsigned src_node STARPU_ATTRIBUTE_UNUSED, void *dst_interface, unsigned dst_node STARPU_ATTRIBUTE_UNUSED)
{
	return copy_opencl_to_opencl_async(src_interface, src_node, dst_interface, dst_node, NULL);
}

#endif

static int copy_any_to_any(void *src_interface, unsigned src_node, void *dst_interface, unsigned dst_node, void *async_data)
{
	struct starpu_matrix_interface *src_matrix = (struct starpu_matrix_interface *) src_interface;
	struct starpu_matrix_interface *dst_matrix = (struct starpu_matrix_interface *) dst_interface;
	int ret = 0;

	uint32_t nx = dst_matrix->nx;
	uint32_t ny = dst_matrix->ny;
	size_t elemsize = dst_matrix->elemsize;

	uint32_t ld_src = src_matrix->ld;
	uint32_t ld_dst = dst_matrix->ld;

	if (ld_src == nx && ld_dst == nx)
	{
		/* Optimize unpartitioned and y-partitioned cases */
		if (starpu_interface_copy(src_matrix->dev_handle, src_matrix->offset, src_node,
		                          dst_matrix->dev_handle, dst_matrix->offset, dst_node,
		                          nx*ny*elemsize, async_data))
			ret = -EAGAIN;
	}
	else
	{
		unsigned y;
		for (y = 0; y < ny; y++)
		{
			uint32_t src_offset = y*ld_src*elemsize;
			uint32_t dst_offset = y*ld_dst*elemsize;

			if (starpu_interface_copy(src_matrix->dev_handle, src_matrix->offset + src_offset, src_node,
						  dst_matrix->dev_handle, dst_matrix->offset + dst_offset, dst_node,
						  nx*elemsize, async_data))
				ret = -EAGAIN;
		}
	}

	starpu_interface_data_copy(src_node, dst_node, (size_t)nx*ny*elemsize);

	return ret;
}

static starpu_ssize_t describe(void *data_interface, char *buf, size_t size)
{
	struct starpu_matrix_interface *matrix = (struct starpu_matrix_interface *) data_interface;
	return snprintf(buf, size, "M%ux%ux%u",
			(unsigned) matrix->nx,
			(unsigned) matrix->ny,
			(unsigned) matrix->elemsize);
}
