/* list of configuration processing handlers
 * Copyright (C) 2017 Josef Kubin
 *
 * This file is part of opentracer.
 *
 * opentracer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * opentracer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "tracer.h"
#include "config.h"

static int undef(void)
{
	fprintf(stderr, "error: undefined configuration\n");

	return -1;
}

int (*trace_config_proc[])(struct trace_context *, union trace_config_prefix) = {

	trace_config_plain,
	trace_config_plain,
	trace_config_plain,
	trace_config_plain,

	trace_config_tmr,
	trace_config_tmr,
	trace_config_tmr,
	trace_config_tmr,

	trace_config_tmr,
	trace_config_tmr,
	trace_config_tmr,
	trace_config_tmr,

	trace_config_tmr,
	trace_config_tmr,
	trace_config_tmr,
	trace_config_tmr,

	trace_config_tmr,
	trace_config_tmr,
	trace_config_tmr,
	trace_config_tmr,

	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,
	(int (*)(struct trace_context *, union trace_config_prefix))undef,

	// ...
	// ...
	// ...
};

