/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2010,2011,2014,2016                      Université de Bordeaux
 * Copyright (C) 2011,2017                                CNRS
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
#include <debug/starpu_debug_helpers.h>

/*
 * Display the sizes of various StarPU data structures
 */

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	_starpu_debug_display_structures_size(stderr);

	return EXIT_SUCCESS;
}