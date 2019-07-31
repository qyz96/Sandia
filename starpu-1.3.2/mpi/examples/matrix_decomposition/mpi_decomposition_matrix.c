/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2010-2013,2015-2017                      CNRS
 * Copyright (C) 2009-2012,2014,2015                      Université de Bordeaux
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

#include "mpi_cholesky.h"

/* Returns the MPI node number where data indexes index is */
int my_distrib(int x, int y, int nb_nodes)
{
	(void)nb_nodes;
	//return (x+y) % nb_nodes;
	return (x%dblockx)+(y%dblocky)*dblockx;
}


void matrix_display(float ***bmat, int rank)
{
	if (display)
	{
		unsigned y;
		printf("[%d] Input :\n", rank);

		for(y=0 ; y<nblocks ; y++)
		{
			unsigned x;
			for(x=0 ; x<nblocks ; x++)
			{
				unsigned j;
				printf("Block %u,%u :\n", x, y);
				for (j = 0; j < BLOCKSIZE; j++)
				{
					unsigned i;
					for (i = 0; i < BLOCKSIZE; i++)
					{
						if (i <= j)
						{
							printf("%2.2f\t", bmat[y][x][j +i*BLOCKSIZE]);
						}
						else
						{
							printf(".\t");
						}
					}
					printf("\n");
				}
			}
		}
	}
}

void matrix_init(float ****bmat, int rank, int nodes, int alloc_everywhere)
{
	unsigned i,j,x,y;

	*bmat = malloc(nblocks * sizeof(float **));
	for(x=0 ; x<nblocks ; x++)
	{
		(*bmat)[x] = malloc(nblocks * sizeof(float *));
		for(y=0 ; y<nblocks ; y++)
		{
			int mpi_rank = my_distrib(x, y, nodes);
			if (alloc_everywhere || (mpi_rank == rank))
			{
				starpu_malloc((void **)&(*bmat)[x][y], BLOCKSIZE*BLOCKSIZE*sizeof(float));
				for (i = 0; i < BLOCKSIZE; i++)
				{
					for (j = 0; j < BLOCKSIZE; j++)
					{
#ifndef STARPU_SIMGRID
						(*bmat)[x][y][j +i*BLOCKSIZE] = (1.0f/(1.0f+(i+(x*BLOCKSIZE)+j+(y*BLOCKSIZE)))) + ((i+(x*BLOCKSIZE) == j+(y*BLOCKSIZE))?1.0f*size:0.0f);
						//mat[j +i*size] = ((i == j)?1.0f*size:0.0f);
#endif
					}
				}
			}
		}
	}
}

void matrix_free(float ****bmat, int rank, int nodes, int alloc_everywhere)
{
	unsigned x, y;

	for(x=0 ; x<nblocks ; x++)
	{
		for(y=0 ; y<nblocks ; y++)
		{
			int mpi_rank = my_distrib(x, y, nodes);
			if (alloc_everywhere || (mpi_rank == rank))
			{
				starpu_free((void *)(*bmat)[x][y]);
			}
		}
		free((*bmat)[x]);
	}
	free(*bmat);
}

