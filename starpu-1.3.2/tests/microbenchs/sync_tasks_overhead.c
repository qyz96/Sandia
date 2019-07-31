/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2009-2014,2016                           Université de Bordeaux
 * Copyright (C) 2010-2013,2015-2017                      CNRS
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
 * Measure the cost of submitting synchronous tasks
 */

static unsigned ntasks = 65536;

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

static
int inject_one_task(void)
{
	int ret;
	struct starpu_task *task = starpu_task_create();

	task->cl = &dummy_codelet;
	task->cl_arg = NULL;
	task->callback_func = NULL;
	task->synchronous = 1;

	ret = starpu_task_submit(task);
	return ret;

}

static void parse_args(int argc, char **argv)
{
	int c;
	while ((c = getopt(argc, argv, "i:")) != -1)
	switch(c)
	{
		case 'i':
			ntasks = atoi(optarg);
			break;
	}
}

int main(int argc, char **argv)
{
	int ret;
	unsigned i;
	double timing;
	double start;
	double end;
	struct starpu_conf conf;
	starpu_conf_init(&conf);
	conf.ncpus = 2;

#ifdef STARPU_QUICK_CHECK
	ntasks = 128;
#endif

	parse_args(argc, argv);

	ret = starpu_initialize(&conf, &argc, &argv);
	if (ret == -ENODEV) return STARPU_TEST_SKIPPED;
	STARPU_CHECK_RETURN_VALUE(ret, "starpu_init");

	fprintf(stderr, "#tasks : %u\n", ntasks);

	start = starpu_timing_now();
	for (i = 0; i < ntasks; i++)
	{
		ret = inject_one_task();
		if (ret == -ENODEV) goto enodev;
		STARPU_CHECK_RETURN_VALUE(ret, "starpu_task_submit");
	}
	end = starpu_timing_now();

	timing = end - start;

	fprintf(stderr, "Total: %f secs\n", timing/1000000);
	fprintf(stderr, "Per task: %f usecs\n", timing/ntasks);

        {
                char *output_dir = getenv("STARPU_BENCH_DIR");
                char *bench_id = getenv("STARPU_BENCH_ID");

                if (output_dir && bench_id)
		{
                        char file[1024];
                        FILE *f;

                        snprintf(file, sizeof(file), "%s/sync_tasks_overhead_total.dat", output_dir);
                        f = fopen(file, "a");
                        fprintf(f, "%s\t%f\n", bench_id, timing/1000000);
                        fclose(f);

                        snprintf(file, sizeof(file), "%s/sync_tasks_overhead_per_task.dat", output_dir);
                        f = fopen(file, "a");
                        fprintf(f, "%s\t%f\n", bench_id, timing/ntasks);
                        fclose(f);
                }
        }

	starpu_shutdown();

	return EXIT_SUCCESS;

enodev:
	fprintf(stderr, "WARNING: No one can execute this task\n");
	/* yes, we do not perform the computation but we did detect that no one
 	 * could perform the kernel, so this is not an error from StarPU */
	starpu_shutdown();
	return STARPU_TEST_SKIPPED;
}
