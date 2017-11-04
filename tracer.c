/* the main trace automaton
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

#define TRACE_POINT_MAX_BITWIDTH	31

static const char hex_digit[] = {
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'a',
	'b',
	'c',
	'd',
	'e',
	'f',
};

char * trace_escape_sequence(char *dst)
{

	unsigned char chr = *dst;

	if (chr < '\a' || (chr > '\r' && chr < ' ') || chr > '~') {

		*dst++ = '\\';

		if (!chr) {
			*dst = '0';
			return dst;
		}

		*dst++ = 'x';
		*dst++ = hex_digit[chr >> 4];
		*dst   = hex_digit[chr & 0xf];
		return dst;
	}

	switch (chr) {
		case '\a':
			*dst++ = '\\';
			*dst   = 'a';
			return dst;
		case '\b':
			*dst++ = '\\';
			*dst   = 'b';
			return dst;
		case '\t':
			*dst++ = '\\';
			*dst   = 't';
			return dst;
		case '\n':
			*dst++ = '\\';
			*dst   = 'n';
			return dst;
		case '\v':
			*dst++ = '\\';
			*dst   = 'v';
			return dst;
		case '\r':
			*dst++ = '\\';
			*dst   = 'r';
			return dst;
		case '\f':
			*dst++ = '\\';
			*dst   = 'f';
			return dst;
		case '\'':
			*dst++ = '\\';
			*dst   = '\'';
			return dst;
		case '\"':
			*dst++ = '\\';
			*dst   = '\"';
			return dst;
		case '\\':
			dst++;
			*dst   = '\\';
			return dst;
	}

	return dst;
}



void trace_bitfield_to_bfr(char *bfr, unsigned int bitfield, unsigned char width)
{

	unsigned char i;
	unsigned int bitmask;

	bitmask = 0x1 << (width - 1);

	for (i = 0; i < width; i++, bfr++, bitfield <<= 1) {

		if (bitfield & bitmask)
			*bfr = '1';
		else
			*bfr = '0';
	}

	*bfr = 0;
}

int trace_fetch_data(struct trace_context *tr,
		const char *name,
		unsigned char checksum,
		unsigned char always,
		unsigned char *ptr,
		unsigned char dl)
{

	int f;

	if (tr->config.checksum_level || always) {

		dl++;
	}

	f = trace_ftdi_read_data(tr, NULL, ptr, dl);

	if (f < 0) {

		fprintf(stderr, "error: %s (%d) " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n", ftdi_get_error_string(tr->ftdi), f);

		return f;
	}

#ifndef DEBUG
	if (tr->config.checksum_level || always) {

		/*
		 * loop unrolling for speed
		 */

		dl--;

		if (dl & 0x1) {
			checksum += *ptr++;
		}

		dl >>= 1;

		if (dl) {
			if (dl & 0x1) {
				checksum += *ptr++;
				checksum += *ptr++;
			}

			dl >>= 1;

			if (dl) {
				if (dl & 0x1) {
					checksum += *ptr++;
					checksum += *ptr++;
					checksum += *ptr++;
					checksum += *ptr++;
				}

				dl >>= 1;

				while (dl) {
					dl--;
					checksum += *ptr++;
					checksum += *ptr++;
					checksum += *ptr++;
					checksum += *ptr++;
					checksum += *ptr++;
					checksum += *ptr++;
					checksum += *ptr++;
					checksum += *ptr++;
				}
			}
		}

		checksum = ~checksum;

		if (*ptr != checksum) {

			fprintf(stderr, "error: checksum of %s (0x%02x, last known sync time %.9f) " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n",
					name, checksum, tr->time.sync);

			return -1;
		}
	}
#endif

	return 0;
}

int trace_tracepoint(struct trace_context *tr, unsigned char opcode)
{

	char txt[TRACE_POINT_MAX_BITWIDTH + TRACE_NULL_CHAR];
	int f;
	unsigned char data[TRACE_MAX_TRACEPOINT_WIDTH + TRACE_MAX_TMR_WIDTH + TRACE_CHECKSUM_DATA];
	unsigned char dl;
	unsigned char pnt_width;
	unsigned char tmr_width;
	unsigned int pnt = opcode ^ TRACE_OPCODE_TRACEPOINT;

	tmr_width = tr->config.tracepoint_tmr_width;
	pnt_width = tr->config.tracepoint_width;

	dl = pnt_width + tmr_width;

	if ((f = trace_fetch_data(tr, "tracepoint", opcode, TRACE_CHECKSUM_BY_CONFIG, data, dl)))
		return f;

	if (pnt_width) {
		unsigned int data_mask;

		pnt <<= pnt_width * 8;

		data_mask = -1;

		data_mask >>= (sizeof(data_mask) - pnt_width) * 8;

		pnt |= *((unsigned int *)data) & data_mask;
	}

	trace_bitfield_to_bfr(txt, pnt, (pnt_width + 1) * 8 - 1);

	printf("0b%s\t%u\t", txt, pnt);

	if (trace_option & TRACE_APPEND_HEX)
		printf(hex_format[pnt_width + 1], pnt);

	if (tmr_width) {

		double inc;
		unsigned long diff;
		unsigned long tmr;

		tmr = *((unsigned long *)(data + pnt_width));

		tmr <<= (sizeof(tmr) - tmr_width) * 8;

		/* logical timestamp tests */

		diff = tmr - tr->time.tsm;

		if (diff > TRACE_TIMER_TIMEOUT) {

			fprintf(stderr, "error: tracepoint time (expired tsm or tracestream misalignment) "
					// #ifdef DEBUG_FILE
					STRINGIFY(__FILE__) " " STRINGIFY(__LINE__)
					// #endif
					"\n");

			return -1;
		}

		diff = tmr - tr->time.tmr;

		tr->time.tmr = tmr;

		if (diff > TRACE_TIMER_TIMEOUT) {

			fprintf(stderr, "error: tracepoint monotony (time) "
					// #ifdef DEBUG_FILE
					STRINGIFY(__FILE__) " " STRINGIFY(__LINE__)
					// #endif
					"\n");

			return -1;
		}

		diff >>= (sizeof(diff) - tmr_width) * 8;

		inc = diff * tr->time.tick;

		tr->time.mcu += inc;

		printf("%.9f\t", tr->time.mcu);
	}

	trace_append_time(tr);

	printf(trace_grep.pnt.match);

	return 0;
}

int trace_txt(struct trace_context *tr, unsigned char opcode)
{

	char *dst;
	char *src;
	char raw[TRACE_OPCODE_TXT + TRACE_CHECKSUM_DATA + TRACE_NULL_CHAR];
	char txt[TRACE_OPCODE_TXT + TRACE_OPCODE_TXT + TRACE_OPCODE_TXT + TRACE_OPCODE_TXT + TRACE_NULL_CHAR];
	int f;
	unsigned char dl = opcode ^ TRACE_OPCODE_TXT;
	unsigned char i;

	dl++;

	if ((f = trace_fetch_data(tr, "text", opcode, TRACE_CHECKSUM_BY_CONFIG, (unsigned char *)raw, dl)))
		return f;

	for (src = raw, dst = txt, i = 0; i < dl; src++, dst++, i++) {

		*dst = *src;

		dst = trace_escape_sequence(dst);
	}

	*dst = 0;

	printf(txt);

	return 0;
}

int trace_struct_select(struct trace_context *tr, unsigned char opcode)
{

	struct trace_struct *tsp;
	unsigned char struct_index;

	struct_index = opcode ^ TRACE_OPCODE_STRUCT_SELECT;

	if (struct_index == 0x0f) {

		fprintf(stderr, "error: struct #%u (in thread: %u) is reserved " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n",
				struct_index, tr->thread_idx);

		return -1;
	}

	tsp = tr->ts + struct_index;

	if (!tsp->ntype) {

		fprintf(stderr, "error: struct #%u (in thread: %u) is not defined " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n",
				struct_index, tr->thread_idx);

		return -1;
	}

	tr->node = trace_struct_selected;
	tr->pdata = tsp->data;
	tr->tsp = tsp;

	return 0;
}

#define ERR_TSM	stderr, "error: sync time (last known sync time %.9f) " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n", tr->time.sync

int trace_tsm(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_MAX_TMR_WIDTH + TRACE_CHECKSUM_DATA];
	unsigned char tmr_width = tr->config.tsm_tmr_width;

	if ((f = trace_fetch_data(tr, "tsm",  TRACE_OPCODE_TSM, TRACE_CHECKSUM_ALWAYS, data, tmr_width)))
		return f;

	if (tmr_width) {

		double inc;
		unsigned long diff;
		unsigned long tmr;

		tmr = *((unsigned long *)data);

		tmr <<= (sizeof(tmr) - tmr_width) * 8;

		diff = tmr - tr->time.tsm;

		/* logical time test */

		if (diff > TRACE_TIMER_TIMEOUT) {

			fprintf(ERR_TSM);

			return -1;
		}

		diff = tmr - tr->time.tmr;

		tr->time.tsm = tr->time.tmr = tmr;

		diff >>= (sizeof(diff) - tmr_width) * 8;

		inc = diff * tr->time.tick;

		tr->time.mcu += inc;
		tr->time.sync = tr->time.mcu;
	}

	if (tr->option & TRACE_PRINT_TSM) {

		if (tr->node == trace_normal) {

			if (tmr_width)
				printf("%.9f\t", tr->time.mcu);

			trace_append_time(tr);

			printf(trace_grep.tsm.match);
		} else {
			// FIXME doresit zobrazovani defered
			if (tr->node != trace_modeline && tr->node != trace_struct_types/* && tr->node != trace_struct_captions*/) {

				if (!tr->defered.tsm) {
					tr->defered.tsm = 1;
					tr->tsm_time_rtc = tr->time.rtc;
					tr->tsm_time_rtc_elaps = tr->time.rtc_elaps;
				}
			}
		}
	}

	return 0;
}

int trace_sint(struct trace_context *tr, unsigned char opcode)
{

	int f;
	long sign_mask;
	unsigned char data[sizeof(long) + TRACE_CHECKSUM_DATA];
	unsigned char dl = 1;
	// unsigned char i;
	unsigned char power = opcode ^ TRACE_OPCODE_INT;

	*((long *)data) = 0;

	dl <<= power;

	if ((f = trace_fetch_data(tr, "signed int", opcode, TRACE_CHECKSUM_BY_CONFIG, data, dl)))
		return f;

	sign_mask = 0;

	if (dl < sizeof(sign_mask)) {

		if (data[dl - 1] & 0x80) {
			sign_mask = -1;
			sign_mask <<= dl * 8;
		}
	}

	printf("%ld\t", *((long *)data) | sign_mask);

	if (trace_option & TRACE_APPEND_HEX)
		printf(hex_format[dl], *((long *)data));

	trace_append_time(tr);

	printf(trace_grep.sint.match, dl);

	return 0;
}

int trace_uint(struct trace_context *tr, unsigned char opcode)
{

	int f;
	unsigned char data[sizeof(long) + TRACE_CHECKSUM_DATA];
	unsigned char dl = 1;
	// unsigned char i;
	unsigned char power = opcode ^ (TRACE_OPCODE_INT | TRACE_SUBCODE_UINT);

	*((unsigned long *)data) = 0;

	dl <<= power;

	if ((f = trace_fetch_data(tr, "unsigned int", opcode, TRACE_CHECKSUM_BY_CONFIG, data, dl)))
		return f;



	// if (dl == 2) {
		/*
		 * the last value of an uint64_t is used for xxd address as a side effect
		 * in order to awk to find certain hex dump block
		 */
		// tr->addr = *((unsigned long *)data);
	// }

	// printf("%lu\t", *((unsigned long *)data));

	// if (trace_option & TRACE_APPEND_HEX)
		printf(hex_format[dl], *((unsigned long *)data));

	trace_append_time(tr);

	printf(trace_grep.uint.match, dl);

	return 0;
}

int trace_bitfield(struct trace_context *tr, unsigned char opcode)
{

	int f;
	unsigned char data[TRACE_MAX_BITFIELD + TRACE_CHECKSUM_DATA];
	unsigned char dl = (opcode & ~TRACE_SUBCODE_BITFIELD) + 1;

	if ((f = trace_fetch_data(tr, "bitfield", opcode, TRACE_CHECKSUM_BY_CONFIG, data, dl)))
		return f;

	trace_bitfield_print(tr, data, NULL, dl - 1);

	trace_append_time(tr);

	printf(trace_grep.bitf.match, dl);

	return 0;
}

static int single_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_SINGLE_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "single float", TRACE_OPCODE_FLOAT | TRACE_SUBCODE_SINGLE, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_SINGLE_FLOAT)))
		return f;

	printf("%.6f\t", *((float *)data));

	if (trace_option & TRACE_APPEND_HEX)
		printf(hex_format[sizeof(float)], *((unsigned int *)data));

	trace_append_time(tr);

	printf(trace_grep.single.match);

	return 0;
}

static int double_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_DOUBLE_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "double float", TRACE_OPCODE_FLOAT | TRACE_SUBCODE_DOUBLE, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_DOUBLE_FLOAT)))
		return f;

	printf("%.9f\t", *((double *)data));

	if (trace_option & TRACE_APPEND_HEX)
		printf(hex_format[sizeof(double)], *((unsigned long *)data));

	trace_append_time(tr);

	printf(trace_grep.dblf.match);

	return 0;
}

int (* const trace_std_float[])(struct trace_context *) = {
	single_float,
	double_float,
};

static int trace_data(struct trace_context *tr, unsigned char opcode)
{

	int f;
	unsigned char data[TRACE_OPCODE_DATA + TRACE_CHECKSUM_DATA];
	unsigned char dl = (opcode ^ TRACE_OPCODE_DATA) + 1;

	if ((f = trace_fetch_data(tr, "xxd raw", opcode, TRACE_CHECKSUM_BY_CONFIG, data, dl)))
		return f;

	trace_xxd_print(tr, data, 0, dl);

	return 0;
}

static int trace_rle(struct trace_context *tr, unsigned char opcode)
{

	int f;
	unsigned char data[TRACE_RLE_WIDTH + TRACE_CHECKSUM_DATA];
	unsigned int dl = opcode  ^ (TRACE_OPCODE_MISC | TRACE_SUBCODE_RLE);

	if ((f = trace_fetch_data(tr, "xxd rle", opcode, TRACE_CHECKSUM_BY_CONFIG, data, 2)))
		return f;



	dl <<= 8;
	*((unsigned char *)&dl) = *data;

	dl += 2;

	trace_xxd_print(tr, NULL, data[1], dl);

	return 0;
}

void trace_txt_flush(struct trace_context *tr)
{
	printf("\t");

	trace_append_time(tr);

	printf(trace_grep.txt.match);

	if (tr->defered.any)
		trace_defered_print(tr);
}

void trace_xxd_flush(struct trace_context *tr)
{
	unsigned char unaligned = tr->xxd_address & 0xf;

	if (unaligned) {

		unsigned char i;

		memset(tr->ascii_ptr, ' ', 16 - unaligned);

		for (i = unaligned; i < 16; i++) {

			if (i & 0x1)
				printf("   ");
			else
				printf("  ");
		}

		printf(" %s \n", tr->ascii_bfr);
	}

	printf("\n");

	if (tr->defered.any)
		trace_defered_print(tr);
}

static void transition_txt_to_normal(struct trace_context *tr)
{
	trace_txt_flush(tr);

	tr->node = trace_normal;
}

static void transition_xxd_to_normal(struct trace_context *tr)
{
	trace_xxd_flush(tr);

	tr->node = trace_normal;
}

static int trace_test(struct trace_context *tr)
{

	int f;
	unsigned char data[sizeof(TRACE_TEST_SUCCESS) + TRACE_CHECKSUM_DATA];



	if ((f = trace_fetch_data(tr, "test", 0, TRACE_CHECKSUM_BY_CONFIG, data, sizeof(TRACE_TEST_SUCCESS))))		// <--- udelat nejake nastaveni, ze nema nikdy cist checksum
		return f;

	if (*((unsigned int *)data) == TRACE_TEST_SUCCESS) {

		fprintf(stderr, "test: success\n");

		return TRACE_TEST_SUCCESS;
	}

	fprintf(TRACE_TARGET_HAS_CRASHED);

	return -1;
}

int trace_text(struct trace_context *tr, unsigned char opcode)
{

	if (tr->timestamp(tr))
		return -1;

	tr->id++;

	if (opcode & TRACE_OPCODE_TRACEPOINT) {

		transition_txt_to_normal(tr);

		return trace_tracepoint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_DATA) {

		trace_txt_flush(tr);

		return trace_data(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_TXT) {

		if ((opcode & TRACE_SUBCODE_BITFIELD) == TRACE_SUBCODE_BITFIELD) {

			transition_txt_to_normal(tr);

			return trace_bitfield(tr, opcode);
		}

		return trace_txt(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_STRUCT_SELECT) {

		trace_txt_flush(tr);

		return trace_struct_select(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_INT) {

		transition_txt_to_normal(tr);

		if (opcode & TRACE_SUBCODE_UINT)
			return trace_uint(tr, opcode);

		return trace_sint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_MISC) {

		if (opcode & TRACE_SUBCODE_RLE) {

			trace_txt_flush(tr);

			return trace_rle(tr, opcode);
		}

		transition_txt_to_normal(tr);

		return tr->usr_float[opcode ^ TRACE_OPCODE_MISC](tr);
	}

	if (opcode & TRACE_OPCODE_FLOAT) {

		transition_txt_to_normal(tr);

		return trace_std_float[opcode ^ TRACE_OPCODE_FLOAT](tr);
	}

	if (opcode & TRACE_OPCODE_TSM) {

		return trace_tsm(tr);
	}

	fprintf(TRACE_TARGET_HAS_CRASHED);

	return -1;
}

int trace_xxd(struct trace_context *tr, unsigned char opcode)
{

	if (tr->timestamp(tr))
		return -1;

	tr->id++;

	if (opcode & TRACE_OPCODE_TRACEPOINT) {

		transition_xxd_to_normal(tr);

		return trace_tracepoint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_DATA) {

		return trace_data(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_TXT) {

		if ((opcode & TRACE_SUBCODE_BITFIELD) == TRACE_SUBCODE_BITFIELD) {

			transition_xxd_to_normal(tr);

			return trace_bitfield(tr, opcode);
		}

		trace_xxd_flush(tr);

		tr->node = trace_text;

		return trace_txt(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_STRUCT_SELECT) {

		trace_xxd_flush(tr);

		return trace_struct_select(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_INT) {

		transition_xxd_to_normal(tr);

		if (opcode & TRACE_SUBCODE_UINT)
			return trace_uint(tr, opcode);

		return trace_sint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_MISC) {

		if (opcode & TRACE_SUBCODE_RLE) {

			return trace_rle(tr, opcode);
		}

		transition_xxd_to_normal(tr);

		return tr->usr_float[opcode ^ TRACE_OPCODE_MISC](tr);
	}

	if (opcode & TRACE_OPCODE_FLOAT) {

		transition_xxd_to_normal(tr);

		return trace_std_float[opcode ^ TRACE_OPCODE_FLOAT](tr);
	}

	if (opcode & TRACE_OPCODE_TSM) {

		return trace_tsm(tr);
	}

	fprintf(TRACE_TARGET_HAS_CRASHED);

	return -1;
}

int trace_normal(struct trace_context *tr, unsigned char opcode)
{

	if (tr->timestamp(tr))
		return -1;

	tr->id++;

	if (opcode & TRACE_OPCODE_TRACEPOINT) {

		return trace_tracepoint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_DATA) {

		return trace_data(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_TXT) {

		if ((opcode & TRACE_SUBCODE_BITFIELD) == TRACE_SUBCODE_BITFIELD) {

			return trace_bitfield(tr, opcode);
		}

		tr->node = trace_text;

		return trace_txt(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_STRUCT_SELECT) {

		return trace_struct_select(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_INT) {

		if (opcode & TRACE_SUBCODE_UINT)
			return trace_uint(tr, opcode);

		return trace_sint(tr, opcode);
	}

	if (opcode & TRACE_OPCODE_MISC) {

		if (opcode & TRACE_SUBCODE_RLE) {

			return trace_rle(tr, opcode);
		}

		return tr->usr_float[opcode ^ TRACE_OPCODE_MISC](tr);
	}

	if (opcode & TRACE_OPCODE_FLOAT) {

		return trace_std_float[opcode ^ TRACE_OPCODE_FLOAT](tr);
	}

	if (opcode & TRACE_OPCODE_TSM) {

		return trace_tsm(tr);
	}

	return trace_test(tr);
}

