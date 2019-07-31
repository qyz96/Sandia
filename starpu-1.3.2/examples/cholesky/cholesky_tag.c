/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2008-2017                                Université de Bordeaux
 * Copyright (C) 2012,2013                                Inria
 * Copyright (C) 2010-2013,2015,2017                      CNRS
 * Copyright (C) 2013                                     Thibaut Lambert
 * Copyright (C) 2010                                     Mehdi Juhoor
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

/*
 * This version of the Cholesky factorization uses explicit dependency
 * declaration through dependency tags.
 * It also uses data partitioning to split the matrix into submatrices
 */

#include "cholesky.h"
#include <starpu_perfmodel.h>

#if defined(STARPU_USE_CUDA) && defined(STARPU_HAVE_MAGMA)
#include "magma.h"
#endif

/*
 *	Some useful functions
 */

static struct starpu_task *create_task(starpu_tag_t id)
{
	struct starpu_task *task = starpu_task_create();
		task->cl_arg = NULL;
		task->use_tag = 1;
		task->tag_id = id;

	return task;
}

/*
 *	Create the codelets
 */

static struct starpu_task * create_task_11(starpu_data_handle_t dataA, unsigned k)
{
/*	FPRINTF(stdout, "task 11 k = %d TAG = %llx\n", k, (TAG11(k))); */

	struct starpu_task *task = create_task(TAG11(k));

	task->cl = &cl11;

	/* which sub-data is manipulated ? */
	task->handles[0] = starpu_data_get_sub_data(dataA, 2, k, k);

	/* this is an important task */
	if (!noprio_p)
		task->priority = STARPU_MAX_PRIO;

	/* enforce dependencies ... */
	if (k > 0)
	{
		starpu_tag_declare_deps(TAG11(k), 1, TAG22(k-1, k, k));
	}

	int n = starpu_matrix_get_nx(task->handles[0]);
	task->flops = FLOPS_SPOTRF(n);

	return task;
}

static void create_task_21(starpu_data_handle_t dataA, unsigned k, unsigned j)
{
	struct starpu_task *task = create_task(TAG21(k, j));

	task->cl = &cl21;

	/* which sub-data is manipulated ? */
	task->handles[0] = starpu_data_get_sub_data(dataA, 2, k, k);
	task->handles[1] = starpu_data_get_sub_data(dataA, 2, k, j);

	if (!noprio_p && (j == k+1))
	{
		task->priority = STARPU_MAX_PRIO;
	}

	/* enforce dependencies ... */
	if (k > 0)
	{
		starpu_tag_declare_deps(TAG21(k, j), 2, TAG11(k), TAG22(k-1, k, j));
	}
	else
	{
		starpu_tag_declare_deps(TAG21(k, j), 1, TAG11(k));
	}

	int n = starpu_matrix_get_nx(task->handles[0]);
	task->flops = FLOPS_STRSM(n, n);

	int ret = starpu_task_submit(task);
        if (STARPU_UNLIKELY(ret == -ENODEV))
	{
                FPRINTF(stderr, "No worker may execute this task\n");
                exit(0);
        }

}

static void create_task_22(starpu_data_handle_t dataA, unsigned k, unsigned i, unsigned j)
{
/*	FPRINTF(stdout, "task 22 k,i,j = %d,%d,%d TAG = %llx\n", k,i,j, TAG22(k,i,j)); */

	struct starpu_task *task = create_task(TAG22(k, i, j));

	task->cl = &cl22;

	/* which sub-data is manipulated ? */
	task->handles[0] = starpu_data_get_sub_data(dataA, 2, k, i);
	task->handles[1] = starpu_data_get_sub_data(dataA, 2, k, j);
	task->handles[2] = starpu_data_get_sub_data(dataA, 2, i, j);

	if (!noprio_p && (i == k + 1) && (j == k +1) )
	{
		task->priority = STARPU_MAX_PRIO;
	}

	/* enforce dependencies ... */
	if (k > 0)
	{
		starpu_tag_declare_deps(TAG22(k, i, j), 3, TAG22(k-1, i, j), TAG21(k, i), TAG21(k, j));
	}
	else
	{
		starpu_tag_declare_deps(TAG22(k, i, j), 2, TAG21(k, i), TAG21(k, j));
	}

	int n = starpu_matrix_get_nx(task->handles[0]);
	task->flops = FLOPS_SGEMM(n, n, n);

	int ret = starpu_task_submit(task);
        if (STARPU_UNLIKELY(ret == -ENODEV))
	{
                FPRINTF(stderr, "No worker may execute this task\n");
                exit(0);
        }
}



/*
 *	code to bootstrap the factorization
 *	and construct the DAG
 */

static void _cholesky(starpu_data_handle_t dataA, unsigned nblocks)
{
	double start;
	double end;

	struct starpu_task *entry_task = NULL;

	/* create all the DAG nodes */
	unsigned i,j,k;

	start = starpu_timing_now();

	for (k = 0; k < nblocks; k++)
	{
		starpu_iteration_push(k);
		struct starpu_task *task = create_task_11(dataA, k);
		/* we defer the launch of the first task */
		if (k == 0)
		{
			entry_task = task;
		}
		else
		{
			int ret = starpu_task_submit(task);
                        if (STARPU_UNLIKELY(ret == -ENODEV))
			{
                                FPRINTF(stderr, "No worker may execute this task\n");
                                exit(0);
                        }

		}

		for (j = k+1; j<nblocks; j++)
		{
			create_task_21(dataA, k, j);

			for (i = k+1; i<nblocks; i++)
			{
				if (i <= j)
					create_task_22(dataA, k, i, j);
			}
		}
		starpu_iteration_pop();
	}

	/* schedule the codelet */
	int ret = starpu_task_submit(entry_task);
        if (STARPU_UNLIKELY(ret == -ENODEV))
	{
                FPRINTF(stderr, "No worker may execute this task\n");
                exit(0);
        }


	/* stall the application until the end of computations */
	starpu_tag_wait(TAG11(nblocks-1));

	starpu_data_unpartition(dataA, STARPU_MAIN_RAM);

	end = starpu_timing_now();


	double timing = end - start;

	unsigned n = starpu_matrix_get_nx(dataA);

	double flop = (1.0f*n*n*n)/3.0f;

	PRINTF("# size\tms\tGFlops\n");
	PRINTF("%u\t%.0f\t%.1f\n", n, timing/1000, (flop/timing/1000.0f));
}

static int initialize_system(int argc, char **argv, float **A, unsigned pinned)
{
	int ret;
	int flags = STARPU_MALLOC_SIMULATION_FOLDED;

#ifdef STARPU_HAVE_MAGMA
	magma_init();
#endif

	ret = starpu_init(NULL);
	if (ret == -ENODEV)
		return 77;
	STARPU_CHECK_RETURN_VALUE(ret, "starpu_init");

	init_sizes();

	parse_args(argc, argv);

#ifdef STARPU_USE_CUDA
	initialize_chol_model(&chol_model_11,"chol_model_11",cpu_chol_task_11_cost,cuda_chol_task_11_cost);
	initialize_chol_model(&chol_model_21,"chol_model_21",cpu_chol_task_21_cost,cuda_chol_task_21_cost);
	initialize_chol_model(&chol_model_22,"chol_model_22",cpu_chol_task_22_cost,cuda_chol_task_22_cost);
#else
	initialize_chol_model(&chol_model_11,"chol_model_11",cpu_chol_task_11_cost,NULL);
	initialize_chol_model(&chol_model_21,"chol_model_21",cpu_chol_task_21_cost,NULL);
	initialize_chol_model(&chol_model_22,"chol_model_22",cpu_chol_task_22_cost,NULL);
#endif

	starpu_cublas_init();

	if (pinned)
		flags |= STARPU_MALLOC_PINNED;
	starpu_malloc_flags((void **)A, size_p*size_p*sizeof(float), flags);

	return 0;
}

static void cholesky(float *matA, unsigned size, unsigned ld, unsigned nblocks)
{
	starpu_data_handle_t dataA;

	/* monitor and partition the A matrix into blocks :
	 * one block is now determined by 2 unsigned (i,j) */
	starpu_matrix_data_register(&dataA, STARPU_MAIN_RAM, (uintptr_t)matA, ld, size, size, sizeof(float));

	starpu_data_set_sequential_consistency_flag(dataA, 0);

	struct starpu_data_filter f =
	{
		.filter_func = starpu_matrix_filter_vertical_block,
		.nchildren = nblocks
	};

	struct starpu_data_filter f2 =
	{
		.filter_func = starpu_matrix_filter_block,
		.nchildren = nblocks
	};

	starpu_data_map_filters(dataA, 2, &f, &f2);

	_cholesky(dataA, nblocks);

	starpu_data_unregister(dataA);
}

static void shutdown_system(float **matA, unsigned dim, unsigned pinned)
{
	int flags = STARPU_MALLOC_SIMULATION_FOLDED;
	if (pinned)
		flags |= STARPU_MALLOC_PINNED;

	starpu_free_flags(*matA, dim*dim*sizeof(float), flags);

	starpu_cublas_shutdown();
	starpu_shutdown();
}

int main(int argc, char **argv)
{
	/* create a simple definite positive symetric matrix example
	 *
	 *	Hilbert matrix : h(i,j) = 1/(i+j+1)
	 * */

	float *mat = NULL;
	int ret = initialize_system(argc, argv, &mat, pinned_p);
	if (ret) return ret;

#ifndef STARPU_SIMGRID
	unsigned i,j;
	for (i = 0; i < size_p; i++)
	{
		for (j = 0; j < size_p; j++)
		{
			mat[j +i*size_p] = (1.0f/(1.0f+i+j)) + ((i == j)?1.0f*size_p:0.0f);
			/* mat[j +i*size_p] = ((i == j)?1.0f*size_p:0.0f); */
		}
	}
#endif


#ifdef CHECK_OUTPUT
	FPRINTF(stdout, "Input :\n");

	for (j = 0; j < size_p; j++)
	{
		for (i = 0; i < size_p; i++)
		{
			if (i <= j)
			{
				FPRINTF(stdout, "%2.2f\t", mat[j +i*size_p]);
			}
			else
			{
				FPRINTF(stdout, ".\t");
			}
		}
		FPRINTF(stdout, "\n");
	}
#endif


	cholesky(mat, size_p, size_p, nblocks_p);

#ifdef CHECK_OUTPUT
	FPRINTF(stdout, "Results :\n");

	for (j = 0; j < size_p; j++)
	{
		for (i = 0; i < size_p; i++)
		{
			if (i <= j)
			{
				FPRINTF(stdout, "%2.2f\t", mat[j +i*size_p]);
			}
			else
			{
				FPRINTF(stdout, ".\t");
				mat[j+i*size_p] = 0.0f; /* debug */
			}
		}
		FPRINTF(stdout, "\n");
	}

	FPRINTF(stderr, "compute explicit LLt ...\n");
	float *test_mat = malloc(size_p*size_p*sizeof(float));
	STARPU_ASSERT(test_mat);

	STARPU_SSYRK("L", "N", size_p, size_p, 1.0f,
		     mat, size_p, 0.0f, test_mat, size_p);

	FPRINTF(stderr, "comparing results ...\n");
	for (j = 0; j < size_p; j++)
	{
		for (i = 0; i < size_p; i++)
		{
			if (i <= j)
			{
				FPRINTF(stdout, "%2.2f\t", test_mat[j +i*size_p]);
			}
			else
			{
				FPRINTF(stdout, ".\t");
			}
		}
		FPRINTF(stdout, "\n");
	}
	free(test_mat);
#endif

	shutdown_system(&mat, size_p, pinned_p);
	return 0;
}
