#!/bin/bash
# StarPU --- Runtime system for heterogeneous multicore architectures.
#
# Copyright (C) 2013,2015-2017                           CNRS
# Copyright (C) 2013,2015,2017                           Université de Bordeaux
# Copyright (C) 2013                                     Thibaut Lambert
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
top_srcdir="../.."
export top_builddir="../.."

exec="$1"
shift

if test -z "$exec"
then
    echo "[Error] Syntax: $0 <mic_executable> <args>"
    exit 1
fi
case "$exec" in
	/*) ;;
	*) exec="$PWD/$exec"
esac

NATIVE=${exec/\/build_mic\//\/build_host\/}
DIR="$(dirname "$exec")"
FILE="$(basename "$exec")"

export SINK_LD_LIBRARY_PATH="$top_builddir/src/.libs:$SINK_LD_LIBRARY_PATH"
export STARPU_MIC_SINK_PROGRAM_NAME="$exec" 
export STARPU_MIC_SINK_PROGRAM_PATH="$DIR"

# in case libtool got into play
[ -x "$DIR/.libs/$FILE" ] && STARPU_MIC_SINK_PROGRAM_NAME="$DIR/.libs/$FILE" 

$top_builddir/../build_host/tests/loader "$NATIVE" "$@"
