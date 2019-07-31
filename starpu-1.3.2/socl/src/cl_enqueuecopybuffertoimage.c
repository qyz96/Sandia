/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2012,2017                                CNRS
 * Copyright (C) 2010,2011,2018                           Université de Bordeaux
 * Copyright (C) 2011                                     Inria
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

#include "socl.h"

CL_API_SUFFIX__VERSION_1_0
CL_API_ENTRY cl_int CL_API_CALL
soclEnqueueCopyBufferToImage(cl_command_queue UNUSED(command_queue),
			     cl_mem           UNUSED(src_buffer),
			     cl_mem           UNUSED(dst_image),
			     size_t           UNUSED(src_offset),
			     const size_t *   UNUSED(dst_origin),
			     const size_t *   UNUSED(region),
			     cl_uint          UNUSED(num_events_in_wait_list),
			     const cl_event * UNUSED(event_wait_list),
			     cl_event *       UNUSED(event))
{
	return CL_INVALID_OPERATION;
}