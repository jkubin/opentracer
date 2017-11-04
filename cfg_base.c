/* configuration handler
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

#include "tracer.h"
#include "config.h"



int trace_config_plain(struct trace_context *tr, union trace_config_prefix prefix)
{

	unsigned char checksum;

	checksum = prefix.type + prefix.vers_lo + prefix.vers_hi;

	if (checksum != prefix.checksum) {

#ifndef DEBUG
		fprintf(stderr, "error: checksum (0x%02x) " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n",
				checksum);

		return -1;
#endif
	}

	return 0;
}

