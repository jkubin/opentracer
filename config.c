/* process configuration from a target
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

/*
static const char *trace_checksum_level[] = {
	"config, tsm",
	"config, tsm, tracepoint",
	"config, tsm, tracepoint, data",
	"all traffic (useless for reliable line)",
};
*/

int trace_config(struct trace_context *tr)
{

	int f;
	union trace_config_prefix prefix;
	unsigned int tpoints;

	if ((f = trace_fetch_data(tr, "config prefix", 0, 0, prefix.data, 4)))
		return f;

	if (prefix.type > TRACE_CONFIG_LIST_LEN) {

		fprintf(stderr, "error: config is out of bounds\n");

		return -1;
	}

	tr->config.version              = prefix.version;
	tr->config.tracepoint_width     = prefix.tracepoint_width;
	tr->config.tracepoint_tmr_width =
		tr->config.tsm_tmr_width =
		prefix.tracepoint_tmr_width;

	if ((f = trace_config_proc[prefix.type](tr, prefix)))
		return f;

	trace_init_handlers(tr);

	tr->node = trace_modeline;

	if (trace_option & TRACE_SUPPRESS_CONFIG_SUMMARY)
		return 0;


	tpoints = 1;

	tpoints <<= (tr->config.tracepoint_width + 1) * 8 - 1;

	printf("# config data version:            %d.%d.%d\n",
				(tr->config.version / 1000),
				(tr->config.version / 10) % 100,
				tr->config.version % 10);
	printf("# maximum tracepoints:            %u\n", tpoints);
	printf("# tracepoint width [B]:           %u\n", tr->config.tracepoint_width + 1);

	return 0;
}

