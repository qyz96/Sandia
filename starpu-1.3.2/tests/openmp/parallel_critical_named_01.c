/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2014,2015,2017,2019                      CNRS
 * Copyright (C) 2014,2016                                Inria
 * Copyright (C) 2017                                     Université de Bordeaux
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

#include <pthread.h>
#include <starpu.h>
#include "../helper.h"
#include <stdio.h>

/*
 * Check the OpenMP named critical support.
 */

#if !defined(STARPU_OPENMP)
int main(void)
{
	return STARPU_TEST_SKIPPED;
}
#else
__attribute__((constructor))
static void omp_constructor(void)
{
	int ret = starpu_omp_init();
	if (ret == -EINVAL) exit(STARPU_TEST_SKIPPED);
	STARPU_CHECK_RETURN_VALUE(ret, "starpu_omp_init");
}

__attribute__((destructor))
static void omp_destructor(void)
{
	starpu_omp_shutdown();
}

void critical_g(void *arg)
{
	(void) arg;
	int worker_id;
	pthread_t tid;
	tid = pthread_self();
	worker_id = starpu_worker_get_id();
	printf("[tid %p] task thread = %d -- critical \"g\"\n", (void *)tid, worker_id);
}

void critical_h(void *arg)
{
	(void) arg;
	int worker_id;
	pthread_t tid;
	tid = pthread_self();
	worker_id = starpu_worker_get_id();
	printf("[tid %p] task thread = %d -- critical \"h\"\n", (void *)tid, worker_id);
}

void parallel_region_f(void *buffers[], void *args)
{
	(void) buffers;
	(void) args;
	int worker_id;
	pthread_t tid;
	tid = pthread_self();
	worker_id = starpu_worker_get_id();
	printf("[tid %p] task thread = %d -- parallel -->\n", (void *)tid, worker_id);
	starpu_omp_critical(critical_g, NULL, "g");
	starpu_omp_critical(critical_h, NULL, "h");
	starpu_omp_critical(critical_g, NULL, "g");
	starpu_omp_critical(critical_h, NULL, "h");
	printf("[tid %p] task thread = %d -- parallel <--\n", (void *)tid, worker_id);
}

int
main (void)
{
	struct starpu_omp_parallel_region_attr attr;
	pthread_t tid;
	tid = pthread_self();
	printf("<main>\n");
	memset(&attr, 0, sizeof(attr));
#ifdef STARPU_SIMGRID
	attr.cl.model        = &starpu_perfmodel_nop;
#endif
	attr.cl.flags        = STARPU_CODELET_SIMGRID_EXECUTE;
	attr.cl.cpu_funcs[0] = parallel_region_f;
	attr.cl.where        = STARPU_CPU;
	attr.if_clause       = 1;
	starpu_omp_parallel_region(&attr);
	printf("<main>\n");
	starpu_omp_parallel_region(&attr);
	printf("<main>\n");
	return 0;
}
#endif