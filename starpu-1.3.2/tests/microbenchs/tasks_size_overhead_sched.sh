#!/bin/sh
# StarPU --- Runtime system for heterogeneous multicore architectures.
#
# Copyright (C) 2010,2011,2016,2017                      CNRS
# Copyright (C) 2009,2010,2016                           Université de Bordeaux
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
ROOT=${0%.sh}
ROOT=${ROOT%_sched}
unset STARPU_SSILENT
$ROOT "$@" > tasks_size_overhead.output
ret=$?
if test "$ret" = "0"
then
    # if the program was successfull try to run gnuplot
    DIR=
    [ -z "$STARPU_BENCH_DIR" ] || DIR="$STARPU_BENCH_DIR/"
    export TERMINAL=png
    export OUTFILE=${DIR}tasks_size_overhead_${STARPU_SCHED}.png
    gnuplot_av=$(which gnuplot)
    if test -n "$gnuplot_av"
    then
	# If gnuplot is available, plot the result
	$ROOT.gp
	ret=$?
    fi
fi

exit $ret
