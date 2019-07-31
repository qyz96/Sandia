/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2016                                     Inria
 * Copyright (C) 2017                                     CNRS
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

#ifndef __DATA_MOVEMENTS_INTERFACE_H
#define __DATA_MOVEMENTS_INTERFACE_H

/* interface for data_movements */
struct data_movements_interface
{
	/* Data tags table */
	int *tags;
	/* Ranks table (where to move the corresponding data) */
	int *ranks;
	/* Size of the tables */
	int size;
};

void data_movements_data_register(starpu_data_handle_t *handle, unsigned home_node, int *ranks, int *tags, int size);

int **data_movements_get_ref_tags_table(starpu_data_handle_t handle);
int **data_movements_get_ref_ranks_table(starpu_data_handle_t handle);
int data_movements_reallocate_tables(starpu_data_handle_t handle, int size);

int *data_movements_get_tags_table(starpu_data_handle_t handle);
int *data_movements_get_ranks_table(starpu_data_handle_t handle);
int data_movements_get_size_tables(starpu_data_handle_t handle);

#define DATA_MOVEMENTS_GET_SIZE_TABLES(interface)	(((struct data_movements_interface *)(interface))->size)
#define DATA_MOVEMENTS_GET_TAGS_TABLE(interface)	(((struct data_movements_interface *)(interface))->tags)
#define DATA_MOVEMENTS_GET_RANKS_TABLE(interface)	(((struct data_movements_interface *)(interface))->ranks)

#endif /* __DATA_MOVEMENTS_INTERFACE_H */
