/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2008-2018                                Université de Bordeaux
 * Copyright (C) 2011,2012,2017                           Inria
 * Copyright (C) 2010-2017, 2019                          CNRS
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

#ifndef __STARPU_HELPER_H__
#define __STARPU_HELPER_H__

#include <stdio.h>
#include <starpu.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
   @defgroup API_Miscellaneous_Helpers Miscellaneous Helpers
   @{
*/

/**
   Return the min of the two parameters.
*/
#define STARPU_MIN(a,b)	((a)<(b)?(a):(b))
/**
   Return the max of the two parameters.
*/
#define STARPU_MAX(a,b)	((a)<(b)?(b):(a))

/**
   Define a value which can be used to mark pointers as invalid
   values.
*/
#define STARPU_POISON_PTR	((void *)0xdeadbeef)

extern int _starpu_silent;

char *starpu_getenv(const char *str);

/**
   Return the integer value of the environment variable named \p str.
   Return 0 otherwise (the variable does not exist or has a
   non-integer value).
*/
static __starpu_inline int starpu_get_env_number(const char *str)
{
	char *strval;

	strval = starpu_getenv(str);
	if (strval)
	{
		/* the env variable was actually set */
		long int val;
		char *pcheck;

		val = strtol(strval, &pcheck, 10);
		if (*pcheck) {
			fprintf(stderr,"The %s environment variable must contain an integer\n", str);
			STARPU_ABORT();
		}

		/* fprintf(stderr, "ENV %s WAS %d\n", str, val); */
		STARPU_ASSERT_MSG(val >= 0, "The value for the environment variable '%s' cannot be negative", str);
		return (int)val;
	}
	else
	{
		/* there is no such env variable */
		/* fprintf("There was no %s ENV\n", str); */
		return -1;
	}
}

static __starpu_inline int starpu_get_env_number_default(const char *str, int defval)
{
	int ret = starpu_get_env_number(str);
	if (ret == -1)
		ret = defval;
	return ret;
}

static __starpu_inline float starpu_get_env_float_default(const char *str, float defval)
{
	char *strval;

	strval = starpu_getenv(str);
	if (strval)
	{
		/* the env variable was actually set */
		float val;
		char *pcheck;

		val = strtof(strval, &pcheck);
		if (*pcheck) {
			fprintf(stderr,"The %s environment variable must contain a float\n", str);
			STARPU_ABORT();
		}

		/* fprintf(stderr, "ENV %s WAS %f\n", str, val); */
		return val;
	}
	else
	{
		/* there is no such env variable */
		/* fprintf("There was no %s ENV\n", str); */
		return defval;
	}
}

/**
   Execute the given function \p func on a subset of workers. When
   calling this method, the offloaded function \p func is executed by
   every StarPU worker that are eligible to execute the function. The
   argument \p arg is passed to the offloaded function. The argument
   \p where specifies on which types of processing units the function
   should be executed.
   Similarly to the field starpu_codelet::where, it is possible to
   specify that the function should be executed on every CUDA device
   and every CPU by passing ::STARPU_CPU|::STARPU_CUDA. This function
   blocks until \p func has been executed on every appropriate
   processing units, and thus may not be called from a callback
   function for instance.
*/
void starpu_execute_on_each_worker(void (*func)(void *), void *arg, uint32_t where);

/**
   Same as starpu_execute_on_each_worker(), except that the task name
   is specified in the argument \p name.
*/
void starpu_execute_on_each_worker_ex(void (*func)(void *), void *arg, uint32_t where, const char *name);

/**
   Call \p func(\p arg) on every worker in the \p workers array. \p
   num_workers indicates the number of workers in this array.  This
   function is synchronous, but the different workers may execute the
   function in parallel.
*/
void starpu_execute_on_specific_workers(void (*func)(void*), void *arg, unsigned num_workers, unsigned *workers, const char *name);

/**
   Return the current date in micro-seconds.
*/
double starpu_timing_now(void);

/**
   Copy the content of \p src_handle into \p dst_handle. The parameter \p
   asynchronous indicates whether the function should block or not. In
   the case of an asynchronous call, it is possible to synchronize with
   the termination of this operation either by the means of implicit
   dependencies (if enabled) or by calling starpu_task_wait_for_all(). If
   \p callback_func is not <c>NULL</c>, this callback function is executed after
   the handle has been copied, and it is given the pointer \p
   callback_arg as argument.
*/
int starpu_data_cpy(starpu_data_handle_t dst_handle, starpu_data_handle_t src_handle, int asynchronous, void (*callback_func)(void*), void *callback_arg);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // __STARPU_HELPER_H__
