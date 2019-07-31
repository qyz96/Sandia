/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2016,2017                                CNRS
 * Copyright (C) 2012-2018                                Université de Bordeaux
 * Copyright (C) 2016,2017                                Inria
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
#include <core/simgrid.h>
#include <common/config.h>

#ifdef STARPU_SIMGRID
#ifdef STARPU_HAVE_SIMGRID_MSG_H
#include <simgrid/msg.h>
#else
#include <msg/msg.h>
#endif
#if SIMGRID_VERSION >= 32190
#include <simgrid/simix.hpp>
#else
#include <simgrid/simix.h>
#endif
#ifdef STARPU_HAVE_SIMGRID_HOST_H
#include <simgrid/host.h>
#endif

/* thread_create function which implements inheritence of MPI privatization */
/* See https://github.com/simgrid/simgrid/issues/139 */

typedef struct
{
	void_f_pvoid_t code;
	void *userparam;
	void *father_data;
} thread_data_t;

#if SIMGRID_VERSION >= 32190
static void _starpu_simgrid_xbt_thread_create_wrapper(void)
#else
static int _starpu_simgrid_xbt_thread_create_wrapper(int argc STARPU_ATTRIBUTE_UNUSED, char *argv[] STARPU_ATTRIBUTE_UNUSED)
#endif
{
	/* FIXME: Ugly work-around for bug in simgrid: the MPI context is not properly set at MSG process startup */
	MSG_process_sleep(0.000001);

#ifdef HAVE_SMX_ACTOR_T
	smx_actor_t
#else
	smx_process_t
#endif
	self = SIMIX_process_self();
#if SIMGRID_VERSION < 31300
	thread_data_t *t = (thread_data_t *) SIMIX_process_self_get_data(self);
#else
	thread_data_t *t = (thread_data_t *) SIMIX_process_self_get_data();
#endif
	simcall_process_set_data(self, t->father_data);
	t->code(t->userparam);
	simcall_process_set_data(self, NULL);
	free(t);

#if SIMGRID_VERSION < 32190
	return 0;
#endif
}

void _starpu_simgrid_xbt_thread_create(const char *name, void_f_pvoid_t code, void *param)
{
#if SIMGRID_VERSION >= 32190 || defined(HAVE_SIMCALL_PROCESS_CREATE) || defined(simcall_process_create)
#ifdef HAVE_SMX_ACTOR_T
	smx_actor_t process STARPU_ATTRIBUTE_UNUSED;
#else
	smx_process_t process STARPU_ATTRIBUTE_UNUSED;
#endif
	thread_data_t *res = (thread_data_t *) malloc(sizeof(thread_data_t));
	res->userparam = param;
	res->code = code;
#if SIMGRID_VERSION < 31300
	res->father_data = SIMIX_process_self_get_data(SIMIX_process_self());
#else
	res->father_data = SIMIX_process_self_get_data();
#endif

#if SIMGRID_VERSION < 31200
	simcall_process_create(&process,
#else
	process = simcall_process_create(
#endif
	                         name,
	                         _starpu_simgrid_xbt_thread_create_wrapper, res,
#if SIMGRID_VERSION < 31400
	                         SIMIX_host_self_get_name(),
#else
#  if defined(HAVE_SG_HOST_SELF) || defined(sg_host_self)
	                         sg_host_self(),
#  else
	                         SIMIX_host_self(),
#  endif
#endif
#if SIMGRID_VERSION < 31500 || SIMGRID_VERSION == 31559
				 -1.0,
#endif
#if SIMGRID_VERSION < 32190
				 0, NULL,
#endif
	                         /*props */ NULL
#if SIMGRID_VERSION < 31500 || SIMGRID_VERSION == 31559
				 , 0
#endif
				 );
#else
	STARPU_ABORT_MSG("Can't run StarPU-Simgrid-MPI with a Simgrid version which does not provide simcall_process_create and does not fix https://github.com/simgrid/simgrid/issues/139 , sorry.");
#endif
}

#endif
