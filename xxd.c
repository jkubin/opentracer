/* converts raw data to well known xxd format
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
#include <ctype.h>
#include "tracer.h"

/*
 * Why xxd hex format?
 * For easy conversion to binary.
 * hint: xxd -r
 */

void trace_xxd_print(struct trace_context *tr, unsigned char *bfr, unsigned char chr, unsigned int dl)
{

	char *asc;
	unsigned char *ptr = bfr;
	unsigned int i;
	unsigned int xxd_address;

	if (tr->node == trace_xxd) {
		asc = tr->ascii_ptr;
		xxd_address = tr->xxd_address;

		if (!(xxd_address & 0xf))
			printf("%06x: ", xxd_address);

	} else {
		tr->node = trace_xxd;
		tr->xxd_address = 0;
		xxd_address = 0;
		asc = tr->ascii_bfr;
		*asc = 0;

		printf("%lu\t", tr->id);
		// printf("0x%016lx\t%lu\t", tr->addr, tr->id);

		/*
		 * xxd base address is taken from previous integer record
		 * in order to easily find an xxd data block
		 */
		tr->addr = 0;

		trace_append_time(tr);

		printf(trace_grep.xxd.match);

		printf("000000: ");
	}

	for (i = 0; i < dl; i++, xxd_address++, ptr++, asc++) {

		if (!(xxd_address & 0xf)) {

			if (*tr->ascii_bfr) {
				printf(" %s \n", tr->ascii_bfr);

				if (tr->defered.any)
					trace_defered_print(tr);

				printf("%06x: ", xxd_address);
			}

			asc = tr->ascii_bfr;
		}

		if (bfr)
			chr = *ptr;

		*asc = isprint(chr) ? chr : '.';

		if (xxd_address & 0x1)
			printf("%02x ", chr);
		else
			printf("%02x", chr);
	}

	if (xxd_address & 0xf) {
		tr->ascii_ptr = asc;
	} else {
		printf(" %s \n", tr->ascii_bfr);

		if (tr->defered.any)
			trace_defered_print(tr);

		tr->ascii_ptr = tr->ascii_bfr;
		*tr->ascii_ptr = 0;
	}

	tr->xxd_address = xxd_address;
}

