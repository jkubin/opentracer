/* processing of well known Vim modeline from a target
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
#include "opentracer.h"
#include "tracer.h"

/* 
 * Vim modeline is decisive for file type recognition after pipe
 * or if the csv file have no extension or wrong extension
 * columns are selected by cut/name/awk
 * and properly formated by vim modeline option "ts" (tabstop)
 * syntax highlighting is also working ...
 */

static int print_modeline(struct trace_context *tr, unsigned char opcode)
{

	char *dst;
	char *src;
	char raw[TRACE_OPCODE_TXT + TRACE_CHECKSUM_DATA + TRACE_NULL_CHAR];
	char txt[TRACE_OPCODE_TXT + TRACE_OPCODE_TXT + TRACE_OPCODE_TXT + TRACE_OPCODE_TXT + TRACE_NULL_CHAR];
	int f;
	unsigned char dl = opcode ^ TRACE_OPCODE_TXT;
	unsigned char i;

	dl++;

	if ((f = trace_fetch_data(tr, "modeline", opcode, TRACE_CHECKSUM_BY_CONFIG, (unsigned char *)raw, dl)))
		return f;

	for (src = raw, dst = txt, i = 0; i < dl; src++, dst++, i++) {

gbr:
		if ((*dst = *src) != '\n')
			dst = trace_escape_sequence(dst);
	}

	*dst = 0;

	printf(txt);

	return 0;
}

static int transition_to_normal(struct trace_context *tr)
{
	tr->node = trace_normal;

	printf("\n");

	// if (tr->defered.any)
		// trace_defered_print(tr);

	return tr->timestamp(tr);
}

static void transition_to_struct_types(struct trace_context *tr)
{
	// printf("# captions from a target\n");

	tr->tsp = tr->ts;
	tr->node = trace_struct_types;
}

#define TRACE_MODELINE_ERR	stderr, "error: vim modeline (forbidden instruction)\n"

int trace_modeline(struct trace_context *tr, unsigned char opcode)
{

	if (opcode & TRACE_OPCODE_TRACEPOINT) {

		return transition_to_normal(tr) ||
			trace_tracepoint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_DATA) {

		transition_to_struct_types(tr);

		return trace_struct_types_raw(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_TXT) {

		if ((opcode & TRACE_SUBCODE_BITFIELD) == TRACE_SUBCODE_BITFIELD) {

			return transition_to_normal(tr) ||
				trace_bitfield(tr, opcode);
		}

		return print_modeline(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_STRUCT_SELECT) {

		fprintf(TRACE_MODELINE_ERR);

		return -1;
	}

	if (opcode & TRACE_OPCODE_INT) {

		if (opcode & TRACE_SUBCODE_UINT)
			return transition_to_normal(tr) ||
				trace_uint(tr, opcode);

		return transition_to_normal(tr) ||
			trace_sint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_MISC) {

		if (opcode & TRACE_SUBCODE_RLE) {

			transition_to_struct_types(tr);

			return trace_struct_types_rle(tr, opcode);
		}

		return transition_to_normal(tr) ||
			tr->usr_float[opcode ^ TRACE_OPCODE_MISC](tr);
	}

	if (opcode & TRACE_OPCODE_FLOAT) {

		return transition_to_normal(tr) ||
			trace_std_float[opcode ^ TRACE_OPCODE_FLOAT](tr);
	}

	if (opcode & TRACE_OPCODE_TSM) {

		return trace_tsm(tr);
	}

	fprintf(TRACE_MODELINE_ERR);

	return -1;
}

