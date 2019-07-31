/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2012                                     Inria
 * Copyright (C) 2010-2013,2015,2017                      CNRS
 * Copyright (C) 2010,2011,2013,2014,2016                 Université de Bordeaux
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

#include <stdio.h>
#include <unistd.h>

#include <starpu.h>
#include "../helper.h"

/*
 * Test that we can declare a dependency after submitting a non-auto-destroy task
 */

#ifdef STARPU_QUICK_CHECK
  #define NLOOPS	4
#else
  #define NLOOPS	128
#endif

void dummy_func(void *descr[], void *arg)
{
	(void)descr;
	(void)arg;
}

static struct starpu_codelet dummy_codelet =
{
	.cpu_funcs = {dummy_func},
	.cuda_funcs = {dummy_func},
	.opencl_funcs = {dummy_func},
	.cpu_funcs_name = {"dummy_func"},
	.model = NULL,
	.nbuffers = 0
};

static struct starpu_task *create_dummy_task(void)
{
	struct starpu_task *task = starpu_task_create();

	task->cl = &dummy_codelet;
	task->cl_arg = NULL;

	return task;
}

int main(int argc, char **argv)
{
	int ret;
	unsigned loop, nloops = NLOOPS;

	ret = starpu_initialize(NULL, &argc, &argv);
	if (ret == -ENODEV) return STARPU_TEST_SKIPPED;
	STARPU_CHECK_RETURN_VALUE(ret, "starpu_init");

	for (loop = 0; loop < nloops; loop++)
	{
		struct starpu_task *taskA, *taskB;

		taskA = create_dummy_task();
		taskB = create_dummy_task();

		/* By default, dynamically allocated tasks are destroyed at
		 * termination, we cannot declare a dependency on something
		 * that does not exist anymore. */
		taskA->destroy = 0;
		taskA->detach = 0;

		/* we wait for the tasks explicitly */
		taskB->detach = 0;

		ret = starpu_task_submit(taskA);
		if (ret == -ENODEV)
			return STARPU_TEST_SKIPPED;
		STARPU_CHECK_RETURN_VALUE(ret, "starpu_task_submit");

		starpu_task_declare_deps_array(taskB, 1, &taskA);

		ret = starpu_task_submit(taskB);
		if (ret == -ENODEV)
			return STARPU_TEST_SKIPPED;
		STARPU_CHECK_RETURN_VALUE(ret, "starpu_task_submit");

		ret = starpu_task_wait(taskB);
		STARPU_CHECK_RETURN_VALUE(ret, "starpu_task_submit");

		ret = starpu_task_wait(taskA);
		STARPU_CHECK_RETURN_VALUE(ret, "starpu_task_submit");

		starpu_task_destroy(taskA);
	}

	ret = starpu_task_wait_for_all();
	STARPU_CHECK_RETURN_VALUE(ret, "starpu_task_wait_for_all");

	starpu_shutdown();

	return EXIT_SUCCESS;
}