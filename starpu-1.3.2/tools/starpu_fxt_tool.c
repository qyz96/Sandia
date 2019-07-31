/* StarPU --- Runtime system for heterogeneous multicore architectures.
 *
 * Copyright (C) 2008-2014,2016-2018                      Université de Bordeaux
 * Copyright (C) 2010-2015,2017,2018                      CNRS
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
 * This program should be used to parse the log generated by FxT
 */

#include <starpu.h>
#include <common/config.h>

#define PROGNAME "starpu_fxt_tool"

static void usage()
{
	fprintf(stderr, "Generate a trace in the Paje format\n\n");
	fprintf(stderr, "Usage: %s [ options ]\n", PROGNAME);
        fprintf(stderr, "\n");
        fprintf(stderr, "Options:\n");
	fprintf(stderr, "   -i <input file[s]>  specify the input file[s]. Several files can be provided,\n");
	fprintf(stderr, "                       or the option specified several times for MPI execution\n");
	fprintf(stderr, "                       case\n");
        fprintf(stderr, "   -o <output file>    specify the output file\n");
        fprintf(stderr, "   -c                  use a different colour for every type of task\n");
	fprintf(stderr, "   -no-events          do not show events\n");
	fprintf(stderr, "   -no-counter         do not show scheduler counters\n");
	fprintf(stderr, "   -no-bus             do not show PCI bus transfers\n");
	fprintf(stderr, "   -no-flops           do not show flops\n");
	fprintf(stderr, "   -no-smooth          avoid smoothing values for gflops etc.\n");
	fprintf(stderr, "   -no-acquire         do not show application data acquisitions tasks in DAG\n");
	fprintf(stderr, "   -label-deps         add label on dependencies.\n");
	fprintf(stderr, "   -memory-states      show detailed memory states of handles\n");
	fprintf(stderr, "   -internal           show StarPU-internal tasks in DAG\n");
	fprintf(stderr, "   -h, --help          display this help and exit\n");
	fprintf(stderr, "   -v, --version       output version information and exit\n\n");
        fprintf(stderr, "Report bugs to <%s>.", PACKAGE_BUGREPORT);
        fprintf(stderr, "\n");
}

static struct starpu_fxt_options options;

static int parse_args(int argc, char **argv)
{
	/* Default options */
	starpu_fxt_options_init(&options);

	/* We want to support arguments such as "fxt_tool -i trace_*" */
	unsigned reading_input_filenames = 0;

	int i;
	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-c") == 0)
		{
			options.per_task_colour = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-o") == 0)
		{
			options.out_paje_path = argv[++i];
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-i") == 0)
		{
			options.filenames[options.ninputfiles++] = argv[++i];
			reading_input_filenames = 1;
			continue;
		}

		if (strcmp(argv[i], "-no-events") == 0)
		{
			options.no_events = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-no-counter") == 0)
		{
			options.no_counter = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-no-bus") == 0)
		{
			options.no_bus = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-no-flops") == 0)
		{
			options.no_flops = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-no-smooth") == 0)
		{
			options.no_smooth = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-no-acquire") == 0)
		{
			options.no_acquire = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-memory-states") == 0)
		{
			options.memory_states = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-internal") == 0)
		{
			options.internal = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-label-deps") == 0)
		{
			options.label_deps = 1;
			reading_input_filenames = 0;
			continue;
		}

		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			usage();
			return EXIT_SUCCESS;
		}

		if (strcmp(argv[i], "-v") == 0
		 || strcmp(argv[i], "--version") == 0)
		{
		        fputs(PROGNAME " (" PACKAGE_NAME ") " PACKAGE_VERSION "\n", stderr);
			return EXIT_SUCCESS;
		}

		/* That's pretty dirty: if the reading_input_filenames flag is
		 * set, and that the argument does not match an option, we
		 * assume this may be another filename */
		if (reading_input_filenames)
		{
			options.filenames[options.ninputfiles++] = argv[i];
			continue;
		}
	}

	if (!options.ninputfiles)
	{
		fprintf(stderr, "Incorrect usage, aborting\n");
                usage();
		return 77;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int ret = parse_args(argc, argv);
	if (ret) return ret;

	starpu_fxt_generate_trace(&options);

	return 0;
}