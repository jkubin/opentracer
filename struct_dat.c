/* processing data from arrays, structures and unions printed to ONE row
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
#include <ctype.h>
#include "opentracer.h"
#include "tracer.h"

const char *hex_format[] = {
	"",
	"0x%02x\t",
	"0x%04x\t",
	"0x%06x\t",
	"0x%08x\t",
	"",
	"",
	"",
	"0x%016lx\t",
};

static void struct_member_hexdump(unsigned char **const ppdata, unsigned char dl)
{

	unsigned char *ptr = *ppdata;
	unsigned char chr;
	unsigned char i;

	for (i = 0; chr = *ptr, i < dl; i++, ptr++) {

		if (i & 0x1)
			printf("%02x ", chr);
		else
			printf("%02x", chr);
	}

	printf("%02x\t", chr);

	dl++;

	if (trace_option & TRACE_APPEND_ASCII) {

		char *asc;
		char txt[TRACE_OPCODE_DATA + TRACE_TAB_CHAR + TRACE_NULL_CHAR];

		ptr = *ppdata;

		for (asc = txt, i = 0; i < dl; i++, ptr++, asc++) {

			chr = *ptr;
			*asc = isprint(chr) ? chr : '.';
		}

		*asc++ = '\t';
		*asc = 0;

		printf(txt);
	}

	*ppdata += dl;
}

void trace_bitfield_print(struct trace_context *tr, unsigned char *ptr, unsigned char **const ppdata, unsigned char dl)
{

	char txt[TRACE_BITFIELD_WIDTH + TRACE_NULL_CHAR];
	unsigned int data_mask;
	unsigned int val;

	dl++;

	if (ppdata) {
		ptr = *ppdata;
		*ppdata += dl;
	}

	data_mask = -1;

	if (dl < sizeof(data_mask))
		data_mask >>= (sizeof(data_mask) - dl) * 8;

	val = *((unsigned int *)ptr) & data_mask;

	if (dl == 3) {

		if (tr->option & TRACE_BITF24_AS_UINT24) {

			printf("%u\t", val);

			if (trace_option & TRACE_APPEND_HEX)
				printf(hex_format[3], val);

			return;
		}

		if (tr->option & TRACE_BITF24_AS_SINT24) {

			unsigned int sign_mask = 0;

			if (((unsigned char *)&val)[2] & 0x80)
				sign_mask = 0xff000000;

			printf("%d\t", val | sign_mask);

			if (trace_option & TRACE_APPEND_HEX)
				printf(hex_format[3], val);

			return;
		}
	}

	trace_bitfield_to_bfr(txt, val, dl * 8);

	printf("0b%s\t", txt);

	if (trace_option & TRACE_APPEND_HEX)
		printf(hex_format[dl], val);
}

static void struct_member_text(unsigned char **const ppdata, unsigned char dl)
{

	char *dst;
	char *src;
	char txt[TRACE_OPCODE_TXT + TRACE_OPCODE_TXT + TRACE_OPCODE_TXT + TRACE_OPCODE_TXT + TRACE_TAB_CHAR + TRACE_NULL_CHAR];
	unsigned char i;

	dl++;

	for (src = (char *)*ppdata, dst = txt, i = 0; (*dst = *src) && (i < dl); src++, dst++, i++)
		dst = trace_escape_sequence(dst);

	*dst++ = '\t';
	*dst = 0;

	printf(txt);

	*ppdata += dl;
}

static void struct_member_int(unsigned char **const ppdata, unsigned char power)
{

	long val;
	unsigned char *ptr = *ppdata;
	unsigned char dl = 1;
	unsigned long data_mask;
	unsigned long sign_mask;

	dl <<= power;

	*ppdata += dl;

	data_mask = -1;

	if (dl < sizeof(data_mask))
		data_mask >>= (sizeof(data_mask) - dl) * 8;

	val = *((long *)ptr) & data_mask;

	sign_mask = 0;

	if (dl < sizeof(sign_mask)) {

		if (((unsigned char *)&val)[dl - 1] & 0x80) {

			sign_mask = -1;
			sign_mask <<= dl * 8;
		}
	}

	printf("%ld\t", val | sign_mask);

	if (trace_option & TRACE_APPEND_HEX)
		printf("0x%lx\t", val);		// <--- FIXME osetrit lx kdyz jsou zapnuty warningy
}

static void struct_member_uint(unsigned char **const ppdata, unsigned char power)
{

	unsigned char *ptr = *ppdata;
	unsigned char dl = 1;
	unsigned long data_mask;
	unsigned long val;

	dl <<= power;

	*ppdata += dl;

	data_mask = -1;

	if (dl < sizeof(data_mask))
		data_mask >>= (sizeof(data_mask) - dl) * 8;

	val = *((unsigned long *)ptr) & data_mask;

	printf("%lu\t", val);

	if (trace_option & TRACE_APPEND_HEX)
		printf("0x%lx\t", val);
}

static void struct_member_single(unsigned char **const ppdata)
{

	unsigned char *ptr = *ppdata;

	*ppdata += sizeof(float);

	printf("%f\t", *((float *)ptr));

	if (trace_option & TRACE_APPEND_HEX)
		printf("0x%08x\t", *((unsigned int *)ptr));
}

static void struct_member_double(unsigned char **const ppdata)
{

	unsigned char *ptr = *ppdata;

	*ppdata += sizeof(double);

	printf("%f\t", *((double *)ptr));

	if (trace_option & TRACE_APPEND_HEX)
		printf("0x%016lx\t", *((unsigned long *)ptr));
}

static void (* struct_member_std_float[])(unsigned char **const) = {
	struct_member_single,
	struct_member_double,
};

static int struct_member_tmr(struct trace_context *tr, unsigned char **const ppdata)
{

	double inc;
	unsigned char *ptr;
	unsigned char tmr_width = tr->config.tsm_tmr_width;
	unsigned long diff;
	unsigned long tmr;


	if (!tmr_width) {

		printf("N/A\t");

		return 0;
	}

	ptr = *ppdata;
	*ppdata += tmr_width;

	tmr = *((unsigned long *)ptr);

	tmr <<= (sizeof(tmr) - tmr_width) * 8;

	diff = tmr - tr->time.tsm;

	/* logical timestamp test */

	if (diff > TRACE_TIMER_TIMEOUT) {

		fprintf(stderr, "error: struct member tmr (no sync)\n");

		return -1;
	}

	diff = tmr - tr->time.tmr;

	tr->time.tmr = tmr;

	diff >>= (sizeof(diff) - tmr_width) * 8;

	inc = diff * tr->time.tick;

	tr->time.mcu += inc;

	printf("%.9f\t", tr->time.mcu);

	return 0;
}

static int trace_struct_print(struct trace_context *tr, unsigned char *bfr, unsigned char chr, unsigned int dl)
{
	struct trace_struct *stp = tr->tsp;
	unsigned char **const ppdata = &tr->pdata;

	unsigned char *ptr = *ppdata;
	unsigned char *type;
	unsigned int columns;
	unsigned int i;
	unsigned int rcv_len;

	rcv_len = ptr - stp->data + dl;

	if (stp->ndata < rcv_len) {

		fprintf(stderr, "error: struct overflow %s\n", stp->capt);

		return -1;
	}

	if (bfr)
		memcpy(ptr, bfr, dl);
	else
		memset(ptr, chr, dl);

	if (stp->ndata > rcv_len) {

		*ppdata += dl;

		return 0;
	}

	type = stp->type;
	columns = stp->ntype;

	for (i = 0; i < columns; i++, type++) {

		unsigned char opcode = *type;

		if (opcode & TRACE_OPCODE_DATA) {

			struct_member_hexdump(ppdata, opcode & ~TRACE_OPCODE_DATA);
			continue;
		}

		if (opcode & TRACE_OPCODE_TXT) {

			if ((opcode & TRACE_SUBCODE_BITFIELD) == TRACE_SUBCODE_BITFIELD) {

				trace_bitfield_print(tr, NULL, ppdata, opcode & ~TRACE_SUBCODE_BITFIELD);
				continue;
			}

			struct_member_text(ppdata, opcode & ~TRACE_OPCODE_TXT);
			continue;
		}

		if (opcode & TRACE_OPCODE_INT) {

			if (opcode & TRACE_SUBCODE_UINT) {

				struct_member_uint(ppdata, opcode & ~(TRACE_OPCODE_INT | TRACE_SUBCODE_UINT));
				continue;
			}

			struct_member_int(ppdata, opcode & ~TRACE_OPCODE_INT);
			continue;
		}

		if (opcode & TRACE_OPCODE_MISC) {

			tr->usr_float_struct[opcode & ~TRACE_OPCODE_MISC](NULL, ppdata);
			continue;
		}

		if (opcode & TRACE_OPCODE_FLOAT) {

			struct_member_std_float[opcode & ~TRACE_OPCODE_FLOAT](ppdata);
			continue;
		}

		if (opcode & TRACE_OPCODE_TSM) {

			if (struct_member_tmr(tr, ppdata))
				return -1;

			continue;
		}

		fprintf(stderr, "error: the universe has collapsed\n");

		return -1;	/* unreachable */
	}

	trace_append_time(tr);

	printf(stp->capt);

	tr->node = trace_normal;

	return 0;
}

static int trace_struct_raw(struct trace_context *tr, unsigned char opcode)
{

	int f;
	unsigned char data[TRACE_OPCODE_DATA + TRACE_CHECKSUM_DATA];
	unsigned char dl = (opcode ^ TRACE_OPCODE_DATA) + 1;

	if ((f = trace_fetch_data(tr, "struct raw data", opcode, TRACE_CHECKSUM_BY_CONFIG, data, dl)))
		return f;

	return trace_struct_print(tr, data, 0, dl);
}

static int trace_struct_rle(struct trace_context *tr, unsigned char opcode)
{

	int f;
	unsigned char data[TRACE_RLE_WIDTH + TRACE_CHECKSUM_DATA];
	unsigned int dl = opcode  ^ (TRACE_OPCODE_MISC | TRACE_SUBCODE_RLE);

	if ((f = trace_fetch_data(tr, "struct rle data", opcode, TRACE_CHECKSUM_BY_CONFIG, data, 2)))
		return f;

	dl <<= 8;
	*((unsigned char *)&dl) = *data;

	dl += 2;

	return trace_struct_print(tr, NULL, data[1], dl);
}

#define TRACE_STRUCT_DATA_ERR	stderr, "error: struct insufficient length of received data\n"

static int trace_struct_data(struct trace_context *tr, unsigned char opcode)
{

	if (opcode & TRACE_OPCODE_TRACEPOINT) {

		fprintf(TRACE_STRUCT_DATA_ERR);

		return -1;
	}

	if (opcode & TRACE_OPCODE_DATA) {

		return trace_struct_raw(tr, opcode);
	}

	if (opcode & (TRACE_OPCODE_TXT | TRACE_OPCODE_STRUCT_SELECT | TRACE_OPCODE_INT)) {

		fprintf(TRACE_STRUCT_DATA_ERR);

		return -1;
	}

	if (opcode & TRACE_OPCODE_MISC) {

		if (opcode & TRACE_SUBCODE_RLE) {

			return trace_struct_rle(tr, opcode);
		}

		fprintf(TRACE_STRUCT_DATA_ERR);

		return -1;
	}

	if (opcode & TRACE_OPCODE_FLOAT) {

		fprintf(TRACE_STRUCT_DATA_ERR);

		return -1;
	}

	if (opcode & TRACE_OPCODE_TSM) {

		return trace_tsm(tr);
	}

	fprintf(TRACE_TARGET_HAS_CRASHED);

	return -1;
}

#define TRACE_STRUCT_INSTRUCT_ERR	stderr, "error: struct data (forbidden instruction)\n"

int trace_struct_selected(struct trace_context *tr, unsigned char opcode)
{

	if (opcode & TRACE_OPCODE_TRACEPOINT) {

		fprintf(TRACE_STRUCT_INSTRUCT_ERR);

		return -1;
	}

	if (opcode & TRACE_OPCODE_DATA) {

		tr->node = trace_struct_data;

		return trace_struct_raw(tr, opcode);
	}

	if (opcode & (TRACE_OPCODE_TXT | TRACE_OPCODE_STRUCT_SELECT | TRACE_OPCODE_INT)) {

		fprintf(TRACE_STRUCT_INSTRUCT_ERR);

		return -1;
	}

	if (opcode & TRACE_OPCODE_MISC) {

		if (opcode & TRACE_SUBCODE_RLE) {

			tr->node = trace_struct_data;

			return trace_struct_rle(tr, opcode);
		}

		fprintf(TRACE_STRUCT_INSTRUCT_ERR);

		return -1;
	}

	if (opcode & TRACE_OPCODE_FLOAT) {

		fprintf(TRACE_STRUCT_INSTRUCT_ERR);

		return -1;
	}

	if (opcode & TRACE_OPCODE_TSM) {

		return trace_tsm(tr);
	}

	fprintf(TRACE_TARGET_HAS_CRASHED);

	return -1;
}

