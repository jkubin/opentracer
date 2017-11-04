/* definition of arrays, structures and unions to trace later
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
#include <string.h>
#include "opentracer.h"
#include "tracer.h"

static int sizeof_struct_member(struct trace_context *tr, unsigned char type)
{

	switch (type) {

		case TRACE_bitf8_t:
		case TRACE_sint8_t:
		case TRACE_uint8_t:
			return 1;
		case TRACE_bitf16_t:
		case TRACE_sint16_t:
		case TRACE_uint16_t:
			return 2;
		case TRACE_bitf24_t:
			return 3;
		case TRACE_bitf32_t:
		case TRACE_sint32_t:
		case TRACE_uint32_t:
		case TRACE_single:
			return 4;
		case TRACE_sint64_t:
		case TRACE_uint64_t:
		case TRACE_double:
			return 8;
		case TRACE_usr_float_A:
			return trace_grep_float[tr->config.usr_float_A].size;
		case TRACE_usr_float_B:
			return trace_grep_float[tr->config.usr_float_B].size;
		case TRACE_tsm:
			return tr->config.tsm_tmr_width;
	}

	/* TRACE_hxd(len) */
	if ((type & (TRACE_OPCODE_TRACEPOINT | TRACE_OPCODE_DATA)) == TRACE_OPCODE_DATA)
		return (type & (TRACE_OPCODE_DATA - 1)) + 1;

	/* TRACE_txt(len) */
	if ((type & (TRACE_OPCODE_TRACEPOINT | TRACE_OPCODE_DATA | TRACE_OPCODE_TXT)) == TRACE_OPCODE_TXT)
		return (type & (TRACE_OPCODE_TXT - 1)) + 1;

	return -1;
}

static const char *suffix(struct trace_context *tr, unsigned char type)
{

	switch (type) {

		case TRACE_bitf8_t:
			return trace_grep.bitf1.abbrev;
		case TRACE_uint8_t:
			return trace_grep.uint1.abbrev;
		case TRACE_sint8_t:
			return trace_grep.sint1.abbrev;
		case TRACE_bitf16_t:
			return trace_grep.bitf2.abbrev;
		case TRACE_uint16_t:
			return trace_grep.uint2.abbrev;
		case TRACE_sint16_t:
			return trace_grep.sint2.abbrev;
		case TRACE_bitf24_t:
			if (tr->option & TRACE_BITF24_AS_UINT24)
				return trace_grep.uint3.abbrev;
			if (tr->option & TRACE_BITF24_AS_SINT24)
				return trace_grep.sint3.abbrev;
			return trace_grep.bitf3.abbrev;
		case TRACE_bitf32_t:
			return trace_grep.bitf4.abbrev;
		case TRACE_uint32_t:
			return trace_grep.uint4.abbrev;
		case TRACE_sint32_t:
			return trace_grep.sint4.abbrev;
		case TRACE_single:
			return trace_grep.single.abbrev;
		case TRACE_uint64_t:
			return trace_grep.uint8.abbrev;
		case TRACE_sint64_t:
			return trace_grep.sint8.abbrev;
		case TRACE_double:
			return trace_grep.dblf.abbrev;
		case TRACE_usr_float_A:
			return trace_grep_float[tr->config.usr_float_A].abbrev;
		case TRACE_usr_float_B:
			return trace_grep_float[tr->config.usr_float_B].abbrev;
		case TRACE_tsm:
			return trace_grep.tmr.abbrev;
	}

	return NULL;
}

static int trace_struct_types_data(struct trace_context *tr, unsigned char *bfr, unsigned char type, unsigned int dl)
{
	struct trace_struct *tsp = tr->tsp;
	unsigned char struct_index = tsp - tr->ts;

	unsigned char *ptr = bfr;
	unsigned int i;
	unsigned int types_len;

	if (struct_index == TRACE_STRUCT_TOTAL) {

		fprintf(stderr, "error: struct maximum number (" STRINGIFY(TRACE_STRUCT_TOTAL) ") has been reached\n");

		return -1;
	}

	if (!tsp->ntype)
		tr->ptype = tsp->type;

	types_len = tr->ptype - tsp->type + dl;

	if (types_len > TRACE_SIZEOF_STRUCT_TYPES) {

		fprintf(stderr, "error: struct types buffer overflow (recompile with bigger size)\n");

		return -1;
	}

	for (i = 0; i < dl; i++, ptr++, tr->ptype++, tsp->ntype++) {
		int size;

		if (bfr)
			type = *ptr;

		size = sizeof_struct_member(tr, type);

		if (size < 0) {

			fprintf(stderr, "error: struct member undefined (opcode 0x%02x at pos %u)\n", type, tsp->ntype);

			return size;
		}

		tsp->ndata += size;

		*tr->ptype = type;
	}

	if (tsp->ndata > TRACE_SIZEOF_STRUCT_DATA) {

		fprintf(stderr, "error: struct data buffer overflow (recompile with bigger size)\n");

		return -1;
	}

	return 0;
}

#define TRACE_CAPTION_BUFFER_OVERFLOW	stderr, "error: struct caption buffer overflow (max: " STRINGIFY(TRACE_SIZEOF_STRUCT_MAX_CAPTION) " B)\n"

static int trace_struct_captions_process(struct trace_context *tr, unsigned char opcode)
{
	struct trace_struct *tsp = tr->tsp;
	unsigned char struct_index = tsp - tr->ts;

	char *dst;
	char *src;
	char txt[TRACE_OPCODE_TXT + TRACE_CHECKSUM_DATA + TRACE_NULL_CHAR];
	int f;
	unsigned char chr;
	unsigned char dl = opcode ^ TRACE_OPCODE_TXT;
	unsigned char i;
	unsigned int bfr_limit;

	dl++;

	if ((f = trace_fetch_data(tr, "captions", opcode, TRACE_CHECKSUM_BY_CONFIG, (unsigned char *)txt, dl)))
		return f;

	if (struct_index == TRACE_STRUCT_TOTAL) {

		fprintf(stderr, "error: struct maximum number (" STRINGIFY(TRACE_STRUCT_TOTAL) ") has been reached\n");

		return -1;
	}

gbr:
	if (!*tsp->capt) {
		*tsp->capt = ' ';
		tr->capt_start = tr->pcapt = tsp->capt + 1;
		tr->ptype = tsp->type;
		tr->ncapt = 0;
	}

	for (dst = tr->pcapt, src = txt, i = 0; i < dl; src++, i++) {

		chr = *dst = *src;

		if (chr < '\t' || chr == '\v' || chr == '\f' || (chr > '\r' && chr < ' ') || chr > '~') {

			fprintf(stderr, "error: caption of struct #%u contains a nonprintable character 0x%02x\n", struct_index, chr);

			return -1;
		}

		if (chr == ' ' || chr == '\n')
			*dst = '_';

		if (chr == '\t') {

			unsigned char type;
			unsigned int clen;

			if (tr->ncapt == tsp->ntype) {

				fprintf(stderr, "error: struct #%u captions(%u) > types(%u)\n",
						struct_index,
						tsp->ntype + 1,
						tsp->ntype);

				return -1;
			}

			type = *tr->ptype;

			/* first column */
			clen = dst - tr->capt_start;

			if (!clen)
				*dst++ = '-';

			if (type >= TRACE_OPCODE_TXT && type < TRACE_SUBCODE_BITFIELD) {

				sprintf(dst, trace_grep.txt.abbrev, (type & (TRACE_OPCODE_TXT - 1)) + 1);

				dst += strlen(dst);

			} else if ((type & (TRACE_OPCODE_TRACEPOINT | TRACE_OPCODE_DATA)) == TRACE_OPCODE_DATA) {

				sprintf(dst, trace_grep.xxd.abbrev, (type & (TRACE_OPCODE_DATA - 1)) + 1);

				dst += strlen(dst);

				/* second column */
				if (trace_option & TRACE_APPEND_ASCII) {

					memcpy(dst, tr->capt_start, clen);
					dst += clen;
					dst = stpcpy(dst, trace_grep.ascii.abbrev);
				}
			} else {
				dst = stpcpy(dst, suffix(tr, type));

				/* second column */
				if (trace_option & TRACE_APPEND_HEX) {

					memcpy(dst, tr->capt_start, clen);
					dst += clen;
					dst = stpcpy(dst,  trace_grep.hex.abbrev);
				}
			}

			dst = tr->capt_start = tr->pcapt = tsp->capt;

			printf(dst);

			tr->ncapt++;
			tr->ptype++;

			continue;
		}

		if (chr == '\r')
			continue;

		dst++;
	}

	bfr_limit = dst - tsp->capt;

	if (bfr_limit > TRACE_SIZEOF_STRUCT_MAX_CAPTION) {

		fprintf(TRACE_CAPTION_BUFFER_OVERFLOW);

		return -1;
	}

	*dst = 0;

	tr->pcapt = dst;

	return 0;
}

int trace_struct_types_raw(struct trace_context *tr, unsigned char opcode)
{

	int f;
	unsigned char data[TRACE_OPCODE_DATA + TRACE_CHECKSUM_DATA];
	unsigned char dl = (opcode ^ TRACE_OPCODE_DATA) + 1;

	if ((f = trace_fetch_data(tr, "struct data types raw", opcode, TRACE_CHECKSUM_BY_CONFIG, data, dl)))
		return f;

	return trace_struct_types_data(tr, data, 0, dl);
}

int trace_struct_types_rle(struct trace_context *tr, unsigned char opcode)
{

	int f;
	unsigned char data[TRACE_RLE_WIDTH + TRACE_CHECKSUM_DATA];
	unsigned int dl = opcode  ^ (TRACE_OPCODE_MISC | TRACE_SUBCODE_RLE);

	if ((f = trace_fetch_data(tr, "struct data types rle", opcode, TRACE_CHECKSUM_BY_CONFIG, data, 2)))
		return f;

	dl <<= 8;
	*((unsigned char *)&dl) = *data;

	dl += 2;

	return trace_struct_types_data(tr, NULL, data[1], dl);
}

static int transition_capt_to(struct trace_context *tr)
{
	struct trace_struct *tsp = tr->tsp++;
	unsigned char struct_index = tsp - tr->ts;

	char *ptr;
	char prefix[16];
	unsigned char len;
	unsigned int bfr_limit;

gbr:
	if (tr->ncapt < tsp->ntype) {

		fprintf(stderr, "error: struct #%u captions(%u) < types(%u)\n",
				struct_index,
				tr->ncapt,
				tsp->ntype);

		return -1;
	}

	if (tsp->capt == tr->pcapt) {

		trace_caption_time();

		sprintf(tsp->capt, trace_grep.u_struct.match, tsp->ndata, struct_index);

		printf(tsp->capt);

		return 0;
	}

	trace_caption_time();

	// *tr->pcapt++ = '\n';
	// *tr->pcapt = 0;

	sprintf(prefix, "%uB/", tsp->ndata);

	len = strlen(prefix);

	ptr = tr->pcapt + len + 1;

	bfr_limit = ptr - tsp->capt;

	if (bfr_limit > TRACE_SIZEOF_STRUCT_MAX_CAPTION) {

		fprintf(TRACE_CAPTION_BUFFER_OVERFLOW);

		return -1;
	}

	// FIXME predelat na memmove

	*ptr-- = 0;
	*ptr-- = '\n';

	do {
		tr->pcapt--;
		*ptr-- = *tr->pcapt;
	} while (tsp->capt != tr->pcapt);

	memcpy(tsp->capt, prefix, len);

	printf(tsp->capt);

	return 0;
}

static int transition_capt_to_types(struct trace_context *tr)
{
	tr->node = trace_struct_types;

	return transition_capt_to(tr);
}

static int transition_capt_to_normal(struct trace_context *tr)
{
	tr->node = trace_normal;

	return transition_capt_to(tr) ||
		tr->timestamp(tr);
}

static int trace_struct_captions(struct trace_context *tr, unsigned char opcode)
{

	if (opcode & TRACE_OPCODE_TRACEPOINT) {

		return transition_capt_to_normal(tr) ||
			trace_tracepoint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_DATA) {

		return transition_capt_to_types(tr) ||
			trace_struct_types_raw(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_TXT) {

		if ((opcode & TRACE_SUBCODE_BITFIELD) == TRACE_SUBCODE_BITFIELD) {

			return transition_capt_to_normal(tr) ||
				trace_bitfield(tr, opcode);
		}

		return trace_struct_captions_process(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_STRUCT_SELECT) {

		return transition_capt_to_normal(tr) ||
			trace_struct_select(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_INT) {

		if (opcode & TRACE_SUBCODE_UINT)
			return transition_capt_to_normal(tr) ||
				trace_uint(tr, opcode);

		return transition_capt_to_normal(tr) ||
			trace_sint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_MISC) {

		if (opcode & TRACE_SUBCODE_RLE) {

			return transition_capt_to_types(tr) ||
				trace_struct_types_rle(tr, opcode);
		}

		return transition_capt_to_normal(tr) ||
			tr->usr_float[opcode ^ TRACE_OPCODE_MISC](tr);
	}

	if (opcode & TRACE_OPCODE_FLOAT) {

		return transition_capt_to_normal(tr) ||
			trace_std_float[opcode ^ TRACE_OPCODE_FLOAT](tr);
	}

	if (opcode & TRACE_OPCODE_TSM) {

		return trace_tsm(tr);
	}

	fprintf(stderr, "error: struct captions\n");

	return -1;
}

#define TRACE_STRUCT_TYPES	stderr, "error: struct types (forbidden instruction)\n"

int trace_struct_types(struct trace_context *tr, unsigned char opcode)
{

	if (opcode & TRACE_OPCODE_TRACEPOINT) {

		fprintf(TRACE_STRUCT_TYPES);

		return -1;
	}

	if (opcode & TRACE_OPCODE_DATA) {

		return trace_struct_types_raw(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_TXT) {

		if ((opcode & TRACE_SUBCODE_BITFIELD) == TRACE_SUBCODE_BITFIELD) {

			fprintf(TRACE_STRUCT_TYPES);

			return -1;
		}

		tr->node = trace_struct_captions;

		return trace_struct_captions_process(tr, opcode);
	}

	if (opcode & (TRACE_OPCODE_STRUCT_SELECT | TRACE_OPCODE_INT)) {

		fprintf(TRACE_STRUCT_TYPES);

		return -1;
	}

	if (opcode & TRACE_OPCODE_MISC) {

		if (opcode & TRACE_SUBCODE_RLE) {

			return trace_struct_types_rle(tr, opcode);
		}

		fprintf(TRACE_STRUCT_TYPES);

		return -1;
	}

	if (opcode & TRACE_OPCODE_FLOAT) {

		fprintf(TRACE_STRUCT_TYPES);

		return -1;
	}

	if (opcode & TRACE_OPCODE_TSM) {

		return trace_tsm(tr);
	}

	fprintf(TRACE_STRUCT_TYPES);

	return -1;
}

