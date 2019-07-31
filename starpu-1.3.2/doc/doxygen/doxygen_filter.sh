#!/bin/bash
# StarPU --- Runtime system for heterogeneous multicore architectures.
#
# Copyright (C) 2013,2014,2016,2017,2019                 CNRS
#
# StarPU is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or (at
# your option) any later version.
#
# StarPU is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU Lesser General Public License in COPYING.LGPL for more details.
#
if [ "$(basename $1)" == "starpufft.h" ] ; then
    gcc -E $1 -I ../../include/ -I ../../include/ |grep -i starpufft
else
    # the macro STARPU_DEPRECATED needs to be removed as it is not properly processed by doxygen
    # lines starting with // in the doxygen input files are considered as comments to be removed
    sed -e 's/STARPU_DEPRECATED//' $1 | sed -e 's/^\/\/.*//' | sed -e 's/STARPU_TASK_LIST_INLINE//'
fi
