/* include/starpu_config.h.  Generated from starpu_config.h.in by configure.  */
/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2011,2012,2014,2016,2017                 Inria
 * Copyright (C) 2009-2018                                Université de Bordeaux
 * Copyright (C) 2010-2017,2019                           CNRS
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
 * This is the public config.h file, installed along libstarpu.
 *
 * It should only contain the build-time #defines which have an effect on the
 * API & ABI.
 */

#ifndef __STARPU_CONFIG_PUBLIC_H__
#define __STARPU_CONFIG_PUBLIC_H__

#define STARPU_MAJOR_VERSION 1
#define STARPU_MINOR_VERSION 3
#define STARPU_RELEASE_VERSION 2

#define STARPU_USE_CPU 1
/* #undef STARPU_USE_CUDA */
#define STARPU_USE_OPENCL 1
/* #undef STARPU_USE_MIC */
/* #undef STARPU_USE_MPI_MASTER_SLAVE */

/* #undef STARPU_OPENMP */
/* #undef STARPU_CLUSTER */

/* #undef STARPU_SIMGRID */
/* #undef STARPU_SIMGRID_MC */
/* #undef STARPU_SIMGRID_HAVE_XBT_BARRIER_INIT */
/* #undef STARPU_HAVE_SIMGRID_MSG_H */
/* #undef STARPU_HAVE_XBT_SYNCHRO_H */
/* #undef STARPU_HAVE_VALGRIND_H */
/* #undef STARPU_HAVE_MEMCHECK_H */
/* #undef STARPU_VALGRIND_FULL */
/* #undef STARPU_SANITIZE_LEAK */
#define STARPU_NON_BLOCKING_DRIVERS 1
/* workers must call callbacks on sleep/wake-up */
/* #undef STARPU_WORKER_CALLBACKS */


/* #undef STARPU_HAVE_ICC */

/* #undef STARPU_USE_MPI */
/* #undef STARPU_USE_MPI_MPI */
/* #undef STARPU_USE_MPI_NMAD */

/* #undef STARPU_ATLAS */
/* #undef STARPU_GOTO */
/* #undef STARPU_OPENBLAS */
/* #undef STARPU_MKL */
/* #undef STARPU_ARMPL */
#define STARPU_SYSTEM_BLAS 1

/* #undef STARPU_OPENCL_DATADIR */
/* #undef STARPU_HAVE_MAGMA */

/* #undef STARPU_OPENGL_RENDER */
/* #undef STARPU_USE_GTK */
#define STARPU_HAVE_X11 1

#define STARPU_HAVE_POSIX_MEMALIGN 1

/* #undef STARPU_HAVE_MEMALIGN */

/* #undef STARPU_HAVE_MALLOC_H */

#define STARPU_HAVE_SYNC_BOOL_COMPARE_AND_SWAP 1
#define STARPU_HAVE_SYNC_VAL_COMPARE_AND_SWAP 1
#define STARPU_HAVE_SYNC_FETCH_AND_ADD 1
#define STARPU_HAVE_SYNC_FETCH_AND_OR 1
#define STARPU_HAVE_SYNC_LOCK_TEST_AND_SET 1
#define STARPU_HAVE_SYNC_SYNCHRONIZE 1

/* #undef STARPU_DEVEL */
/* #undef STARPU_MODEL_DEBUG */
/* #undef STARPU_NO_ASSERT */
/* #undef STARPU_DEBUG */
/* #undef STARPU_VERBOSE */
/* #undef STARPU_GDB_PATH */

/* #undef STARPU_HAVE_FFTW */
/* #undef STARPU_HAVE_FFTWF */
/* #undef STARPU_HAVE_FFTWL */
/* #undef STARPU_HAVE_CUFFTDOUBLECOMPLEX */

/* #undef STARPU_HAVE_CURAND */

#define STARPU_MAXNODES 16
#define STARPU_NMAXBUFS 8
#define STARPU_MAXCPUS 64
#define STARPU_MAXNUMANODES 2
#define STARPU_MAXCUDADEVS 4
#define STARPU_MAXOPENCLDEVS 8
#define STARPU_MAXMICDEVS 0
#define STARPU_NMAXWORKERS 80
#define STARPU_NMAX_SCHED_CTXS 10
#define STARPU_MAXIMPLEMENTATIONS 4
#define STARPU_MAXMPKERNELS 10
/* #undef STARPU_USE_SC_HYPERVISOR */
/* #undef STARPU_SC_HYPERVISOR_DEBUG */
/* #undef STARPU_HAVE_GLPK_H */

/* #undef STARPU_HAVE_CUDA_MEMCPY_PEER */
/* #undef STARPU_HAVE_LIBNUMA */

/* #undef STARPU_HAVE_WINDOWS */
/* #undef STARPU_LINUX_SYS */
#define STARPU_HAVE_SETENV 1
#define STARPU_HAVE_UNSETENV 1
#define STARPU_HAVE_UNISTD_H 1
/* #undef STARPU_HAVE_HDF5 */

/* #undef STARPU_USE_FXT */
/* #undef STARPU_FXT_LOCK_TRACES */

#ifdef _MSC_VER
typedef long starpu_ssize_t;
#define __starpu_func__ __FUNCTION__
#else
#  include <sys/types.h>
typedef ssize_t starpu_ssize_t;
#define __starpu_func__ __func__
#endif

#if defined(c_plusplus) || defined(__cplusplus)
/* inline is part of C++ */
#  define __starpu_inline inline
#elif defined(_MSC_VER) || defined(__HP_cc)
#  define __starpu_inline __inline
#else
#  define __starpu_inline __inline__
#endif

/* #undef STARPU_QUICK_CHECK */
/* #undef STARPU_LONG_CHECK */
#define STARPU_USE_DRAND48 1
/* #undef STARPU_USE_ERAND48_R */
#define STARPU_HAVE_NEARBYINTF 1
#define STARPU_HAVE_RINTF 1

#define STARPU_HAVE_HWLOC 1
/* #undef STARPU_HAVE_PTHREAD_SPIN_LOCK */
/* #undef STARPU_HAVE_PTHREAD_BARRIER */
#define STARPU_HAVE_PTHREAD_SETNAME_NP 1
#define STARPU_HAVE_STRUCT_TIMESPEC 1

/* This is only for building examples */
/* #undef STARPU_HAVE_HELGRIND_H */

/* Enable Fortran to C MPI interface */
/* #undef HAVE_MPI_COMM_F2C */

#define STARPU_HAVE_DARWIN 1

#define STARPU_HAVE_CXX11 1
#define STARPU_HAVE_STRERROR_R 1
#define STARPU_HAVE_STATEMENT_EXPRESSIONS 1
/* #undef STARPU_PERF_MODEL_DIR */

#endif
