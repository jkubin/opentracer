/* initialization and clean-up functions
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

#include <stdlib.h>
#include "tracer.h"

int trace_init(struct trace_context *tr)
{

	struct trace_struct *ts = tr->ts;
	unsigned char i;

	tr->timestamp = trace_time_init;

	for (i = 0; i < TRACE_STRUCT_TOTAL; i++, ts++) {

		if (!(ts->capt = malloc(TRACE_SIZEOF_STRUCT_MAX_CAPTION)))
			return -1;

		*ts->capt = 0;

		if (!(ts->data = malloc(TRACE_SIZEOF_STRUCT_DATA)))
			return -1;

		if (!(ts->type = malloc(TRACE_SIZEOF_STRUCT_TYPES)))
			return -1;
	}

	return 0;
}

void trace_free(struct trace_struct *ts)
{
	unsigned char i;

	for (i = 0; i < TRACE_STRUCT_TOTAL; i++, ts++) {

		free(ts->capt);
		free(ts->data);
		free(ts->type);
	}
}

