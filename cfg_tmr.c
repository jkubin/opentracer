/* configuration handler with tmr points
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



union trace_config_time_points
{
	unsigned char data[0];
	struct
	{
		unsigned int cpu_frequency;
		unsigned int tmr_divisor;
		unsigned long tmr_value;
	};
};

int trace_config_tmr(struct trace_context *tr, union trace_config_prefix prefix)
{

	int f;
	union trace_config_time_points suffix;
	unsigned char checksum;
	unsigned char dl;
	unsigned char i;
	unsigned char tmr_width;

	tmr_width = prefix.tracepoint_tmr_width;

	dl = (unsigned long)&((union trace_config_time_points *)0)->tmr_value;

	dl += tmr_width;

	if ((f = trace_fetch_data(tr, "config suffix", 0, 0, suffix.data, dl + 1)))
		return f;

	for (checksum = 0, i = 0; i < sizeof(prefix); i++) {
		checksum += prefix.data[i];
	}

	for (i = 0; i < dl; i++) {
		checksum += suffix.data[i];
	}

	if (checksum != suffix.data[dl]) {

#ifndef DEBUG
		fprintf(stderr, "error: checksum (0x%02x) " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n",
				checksum);

		return -1;
#endif
	}

	tr->config.cpu_frequency = suffix.cpu_frequency;
	tr->config.tmr_divisor   = suffix.tmr_divisor;

	suffix.tmr_value <<= (sizeof(suffix.tmr_value) - tmr_width) * 8;

	tr->time.tsm =
		tr->time.tmr =
		tr->config.tmr =
		suffix.tmr_value;

	return 0;
}

