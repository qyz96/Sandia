/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2017                                     CNRS
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

#define __COMMON_UTILS_H__
#define _STARPU_MALLOC(p, s) do {p = malloc(s);} while (0)
#define _STARPU_CALLOC(p, n, s) do {p = calloc(n, s);} while (0)
#define _STARPU_REALLOC(p, s) do {p = realloc(p, s);} while (0)

#define STARPU_DEBUG_PREFIX "[starpu]"
#ifdef STARPU_VERBOSE
#  define _STARPU_DEBUG(fmt, ...) do { if (!_starpu_silent) {fprintf(stderr, STARPU_DEBUG_PREFIX"[%s] " fmt ,__starpu_func__ ,## __VA_ARGS__); fflush(stderr); }} while(0)
#else
#  define _STARPU_DEBUG(fmt, ...) do { } while (0)
#endif

#define STARPU_UYIELD() ((void)0)

#ifndef NOCONFIG
#include <common/config.h>
#else
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
// Assuming recent simgrid
#define STARPU_HAVE_SIMGRID_MSG_H
#define STARPU_HAVE_XBT_SYNCHRO_H
#endif
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <common/barrier.h>
#ifdef STARPU_HAVE_SIMGRID_MSG_H
#include <simgrid/msg.h>
#else
#include <msg/msg.h>
#endif
#include <simgrid/modelchecker.h>
#ifdef STARPU_HAVE_XBT_SYNCHRO_H
#include <xbt/synchro.h>
#else
#include <xbt/synchro_core.h>
#endif

int
_starpu_simgrid_thread_start(int argc, char *argv[])
{
	return 0;
}

static void _starpu_clock_gettime(struct timespec *ts)
{
	double now = MSG_get_clock();
	ts->tv_sec = floor(now);
	ts->tv_nsec = floor((now - ts->tv_sec) * 1000000000);
}

#include <common/barrier.c>
#include <common/thread.c>

#ifndef NTHREADS
#define NTHREADS 2
#endif

#ifndef NITERS
#define NITERS 1
#endif

struct _starpu_barrier barrier;

int worker(int argc, char *argv[])
{
	unsigned iter;

	for (iter = 0; iter < NITERS; iter++)
	{
		MC_assert(barrier.count <= NTHREADS);
		_starpu_barrier_wait(&barrier);
	}

	return 0;
}

int master(int argc, char *argv[])
{
	unsigned i;

	_starpu_barrier_init(&barrier, NTHREADS);

	for (i = 0; i < NTHREADS; i++)
	{
		char *s;
		asprintf(&s, "%d\n", i);
		char **args = malloc(sizeof(char*)*2);
		args[0] = s;
		args[1] = NULL;
		MSG_process_create_with_arguments("test", worker, NULL, MSG_host_self(), 1, args);
	}

	return 0;
}

#undef main
int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr,"usage: %s platform.xml host\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	srand48(0);
	MSG_init(&argc, argv);
#if SIMGRID_VERSION_MAJOR < 3 || (SIMGRID_VERSION_MAJOR == 3 && SIMGRID_VERSION_MINOR < 13)
	extern xbt_cfg_t _sg_cfg_set;
	xbt_cfg_set_int(_sg_cfg_set, "contexts/stack-size", 128);
#else
	xbt_cfg_set_int("contexts/stack-size", 128);
#endif
	MSG_create_environment(argv[1]);
	MSG_process_create("master", master, NULL, MSG_get_host_by_name(argv[2]));
	MSG_main();
	return 0;
}
