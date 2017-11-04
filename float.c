/* floating point routines for exotic float formats
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
 * floating point routines for exotic float formats
 * frequently used in embedded world
 *
 * bigger floats with wider mantissa lose precision because of double float limitation
 * but for logging purposes or a sanity number test it is sufficient
 */


/*
 * https://en.wikipedia.org/wiki/Minifloat	(signed)
 * |seee emmm|
 */
static void mini_float(unsigned char *ptr, unsigned char **const ppdata)
{

	double flt;
	unsigned char hex;
	unsigned char val;
	unsigned int exp;

	if (ppdata) {
		val = **ppdata;
		*ppdata += TRACE_SIZEOF_MINI_FLOAT;
	} else {
		val = *ptr;
	}

	hex = val;

	flt = 0;

	switch (val) {
		case 0x00:
			break;
		case 0x80:
			((unsigned char *)&flt)[7] = 0x80;	// -0
			break;
			/*
			 * IEEE 754 compliant code
		case 0x78:
			((unsigned int *)&flt)[1] = 0x7ff00000;	// +inf
			break;
		case 0xf8:
			((unsigned int *)&flt)[1] = 0xfff00000;	// -inf
			break;

		default:
			if (val > 0xf8) {
				*((unsigned long *)&flt) = -1;	// -nan
				break;
			}

			if ((val & 0x7f) > 0x78) {
				*((unsigned long *)&flt) = 0x7fffffffffffffff;	// +nan
				break;
			}
			...
			*/
		case 0xff:
			*((unsigned long *)&flt) = -1;	// -nan
			break;
		case 0x7f:
			*((unsigned long *)&flt) = 0x7fffffffffffffff;	// +nan
			break;
		default:
			if (val & 0x80)
				exp = 0xbf800000;
			else
				exp = 0x3f800000;

			if (!(val & 0x78)) {

				/* subnormals */

				if (!(val & 0x6)) {
					val <<= 2;
					exp -= 0x200000;
				}

				if (!(val & 0x4)) {
					val <<= 1;
					exp -= 0x100000;
				}

				val <<= 1;
				val &= 0x7;
			}

			((unsigned int *)&flt)[1] = (val & 0x7f) << 17;
			((unsigned int *)&flt)[1] += exp;
	}

	printf("%f\t", flt);

	if (trace_option & TRACE_APPEND_HEX)
		printf("0x%02x\t", hex);
}

static int fetch_mini_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_MINI_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "mini float", TRACE_OPCODE_MISC, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_MINI_FLOAT)))
		return f;

gbr:
	mini_float(data, NULL);

	trace_append_time(tr);

	printf(trace_grep_float[TRACE_MINI_FLOAT].match);

	return 0;
}

/*
 * https://en.wikipedia.org/wiki/Minifloat	unsigned
 * |eeee mmmm|
 */
static void umin_float(unsigned char *ptr, unsigned char **const ppdata)
{

	double flt;
	unsigned char hex;
	unsigned char val;
	unsigned int exp;

	if (ppdata) {
		val = **ppdata;
		*ppdata += TRACE_SIZEOF_UNSIGNED_MINI_FLOAT;
	} else {
		val = *ptr;
	}

	hex = val;

	flt = 0;

	switch (val) {
		case 0x00:
			break;
		case 0xff:
			*((unsigned long *)&flt) = 0x7fffffffffffffff;	// +nan
			break;
		default:
			exp = 0x3f800000;

			if (!(val & 0xf0)) {

				/* subnormals */

				if (!(val & 0xe)) {
					val <<= 3;
					exp -= 0x300000;
				}

				if (!(val & 0xc)) {
					val <<= 2;
					exp -= 0x200000;
				}

				if (!(val & 0x8)) {
					val <<= 1;
					exp -= 0x100000;
				}

				val <<= 1;
				val &= 0xf;
			}

			((unsigned int *)&flt)[1] = val << 16;
			((unsigned int *)&flt)[1] += exp;
	}

	printf("%f\t", flt);

	if (trace_option & TRACE_APPEND_HEX)
		printf("0x%02x\t", hex);
}

static int fetch_umin_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_UNSIGNED_MINI_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "unsigned mini float", TRACE_OPCODE_MISC, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_UNSIGNED_MINI_FLOAT)))
		return f;

gbr:
	umin_float(data, NULL);

	trace_append_time(tr);

	printf(trace_grep_float[TRACE_UNSIGNED_MINI_FLOAT].match);

	return 0;
}

/*
 * https://en.wikipedia.org/wiki/Half-precision_floating-point_format
 * |seee eemm|mmmm mmmm|
 *
 * https://en.wikipedia.org/wiki/Single-precision_floating-point_format
 * |seee eeee|emmm mmmm|mmmm mmmm|mmmm mmmm|
 *
 * https://en.wikipedia.org/wiki/Double-precision_floating-point_format
 * |seee eeee|eeee mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|
 */
static void half_float(unsigned char *ptr, unsigned char **const ppdata)
{

	double flt;
	unsigned int exp;
	unsigned int hex;
	unsigned int val;

	if (ppdata) {
		ptr = *ppdata;
		*ppdata += TRACE_SIZEOF_HALF_FLOAT;
	}

	val = 0;

	((unsigned char *)&val)[0] = ptr[0];
	((unsigned char *)&val)[1] = ptr[1];

	hex = val;

	flt = 0;

	switch (val) {

		case 0x0000:
			break;
		case 0x8000:
			((unsigned char *)&flt)[7] = 0x80;	// -0
			break;
		case 0x7c00:
			((unsigned int *)&flt)[1] = 0x7ff00000;	// +inf
			break;
		case 0xfc00:
			((unsigned int *)&flt)[1] = 0xfff00000;	// -inf
			break;
		default:
			if (val > 0xfc00) {
				*((unsigned long *)&flt) = -1;	// -nan
				break;
			}

			if ((val & 0x7fff) > 0x7c00) {
				*((unsigned long *)&flt) = 0x7fffffffffffffff;	// +nan
				break;
			}

			if (hex & 0x8000)
				exp = 0xbf000000;
			else
				exp = 0x3f000000;

			if (!(val & 0x7c00)) {

				/* subnormals */

				if (!(val & 0x3fc)) {
					val <<= 8;
					exp -= 0x800000;
				}

				if (!(val & 0x3c0)) {
					val <<= 4;
					exp -= 0x400000;
				}

				if (!(val & 0x300)) {
					val <<= 2;
					exp -= 0x200000;
				}

				if (!(val & 0x200)) {
					val <<= 1;
					exp -= 0x100000;
				}

				val <<= 1;
				val &= 0x3ff;
			}

			((unsigned int *)&flt)[1] = (val & 0x7fff) << 10;
			((unsigned int *)&flt)[1] += exp;
	}

	// printf("%e\t", flt);
	printf("%f\t", flt);

	if (trace_option & TRACE_APPEND_HEX)
		printf("0x%04x\t", hex);
}

static int fetch_half_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_HALF_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "half float", TRACE_OPCODE_MISC, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_HALF_FLOAT)))
		return f;

gbr:
	half_float(data, NULL);

	trace_append_time(tr);

	printf(trace_grep_float[TRACE_HALF_FLOAT].match);

	return 0;
}

/*
 * Microchip 24 bit floating point format
 * |eeee eeee|smmm mmmm|mmmm mmmm|
 */

static void fp24_float(unsigned char *ptr, unsigned char **const ppdata)
{

	double flt;

	if (ppdata) {
		ptr = *ppdata;
		*ppdata += TRACE_SIZEOF_FP24FLT;
	}

	flt = 0;

	printf("%f\t", flt);

	if (trace_option & TRACE_APPEND_HEX) {

		unsigned int hex = *((unsigned int *)ptr) & 0x00ffffff;

		printf("0x%06x\t", hex);
	}
}

static int fetch_fp24_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_FP24FLT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "fp24flt", TRACE_OPCODE_MISC, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_FP24FLT)))
		return f;

	fp24_float(data, NULL);

	trace_append_time(tr);

	printf(trace_grep_float[TRACE_FP24FLT].match);

	return 0;
}

/*
 * https://en.wikipedia.org/wiki/Microsoft_Binary_Format
 *
 * Microchip 32 bit floating point format
 * |eeee eeee|smmm mmmm|mmmm mmmm|mmmm mmmm|
 */

static void mchp_float(unsigned char *ptr, unsigned char **const ppdata)
{
	if (ppdata) {
		ptr = *ppdata;
		*ppdata += TRACE_SIZEOF_MCHP_FLOAT;
	}
}

static int fetch_mchp_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_MCHP_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "mchp", TRACE_OPCODE_MISC, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_MCHP_FLOAT)))
		return f;

	mchp_float(data, NULL);

	trace_append_time(tr);

	printf(trace_grep_float[TRACE_MCHP_FLOAT].match);

	return 0;
}

/*
 * http://en.wikipedia.org/wiki/Single_precision with eXtended mantissa
 * |seee eeee|emmm mmmm|mmmm mmmm|mmmm mmmm|
 * |seee eeee|emmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|
 */

static void xsingle_float(unsigned char *ptr, unsigned char **const ppdata)
{
	if (ppdata) {
		ptr = *ppdata;
		*ppdata += TRACE_SIZEOF_XSINGLE_FLOAT;
	}
}

static int fetch_xsingle_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_XSINGLE_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "xsingle", TRACE_OPCODE_MISC, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_XSINGLE_FLOAT)))
		return f;

	xsingle_float(data, NULL);

	trace_append_time(tr);

	printf(trace_grep_float[TRACE_XSINGLE_FLOAT].match);

	return 0;
}

/*
 * http://en.wikipedia.org/wiki/Extended_precision
 * |seee eeee|eeee eeee|1mmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|
 */

static void extended_float(unsigned char *ptr, unsigned char **const ppdata)
{
	if (ppdata) {
		ptr = *ppdata;
		*ppdata += TRACE_SIZEOF_EXTENDED_FLOAT;
	}
}

static int fetch_extended_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_EXTENDED_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "extended", TRACE_OPCODE_MISC, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_EXTENDED_FLOAT)))
		return f;

	extended_float(data, NULL);

	trace_append_time(tr);

	printf(trace_grep_float[TRACE_EXTENDED_FLOAT].match);

	return 0;
}

/*
 * https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format
 * |seee eeee|eeee eeee|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|
 */

static void quad_float(unsigned char *ptr, unsigned char **const ppdata)
{
	if (ppdata) {
		ptr = *ppdata;
		*ppdata += TRACE_SIZEOF_QUAD_FLOAT;
	}
	// http://babbage.cs.qc.cuny.edu/IEEE-754/
}

static int fetch_quad_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_QUAD_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "quad", TRACE_OPCODE_MISC, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_QUAD_FLOAT)))
		return f;

	quad_float(data, NULL);

	trace_append_time(tr);

	printf(trace_grep_float[TRACE_QUAD_FLOAT].match);

	return 0;
}

/*
 * https://en.wikipedia.org/wiki/Octuple-precision_floating-point_format
 * |seee eeee|eeee eeee|eeee mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|mmmm mmmm|
 */

static void octuple_float(unsigned char *ptr, unsigned char **const ppdata)
{
	if (ppdata) {
		ptr = *ppdata;
		*ppdata += TRACE_SIZEOF_OCTUPLE_FLOAT;
	}
}

static int fetch_octuple_float(struct trace_context *tr)
{

	int f;
	unsigned char data[TRACE_SIZEOF_OCTUPLE_FLOAT + TRACE_CHECKSUM_DATA];

	if ((f = trace_fetch_data(tr, "octuple", TRACE_OPCODE_MISC, TRACE_CHECKSUM_BY_CONFIG, data, TRACE_SIZEOF_OCTUPLE_FLOAT)))
		return f;

	octuple_float(data, NULL);

	trace_append_time(tr);

	printf(trace_grep_float[TRACE_OCTUPLE_FLOAT].match);

	return 0;
}

static void reserved_float(unsigned char *ptr, unsigned char **const ppdata)
{
}

static int fetch_reserved_float(struct trace_context *tr)
{
	printf("N/A\t");

	if (trace_option & TRACE_APPEND_HEX)
		printf("N/A\t");

	trace_append_time(tr);

	// printf(trace_grep_float[tr->config.usr_float_A].match);

	fprintf(stderr, "error: reserved float\n");

	return -1;
}

static void (* const usr_float_struct[16])(unsigned char *, unsigned char **const) = {
	mini_float,
	umin_float,
	half_float,
	fp24_float,
	mchp_float,
	xsingle_float,
	extended_float,
	quad_float,
	octuple_float,
	reserved_float,
	reserved_float,
	reserved_float,
	reserved_float,
	reserved_float,
	reserved_float,
	reserved_float,
};

static int (* const usr_float[16])(struct trace_context *) = {
	fetch_mini_float,
	fetch_umin_float,
	fetch_half_float,
	fetch_fp24_float,
	fetch_mchp_float,
	fetch_xsingle_float,
	fetch_extended_float,
	fetch_quad_float,
	fetch_octuple_float,
	fetch_reserved_float,
	fetch_reserved_float,
	fetch_reserved_float,
	fetch_reserved_float,
	fetch_reserved_float,
	fetch_reserved_float,
	fetch_reserved_float,
};

void trace_init_handlers(struct trace_context *tr)
{
	tr->usr_float[0] = usr_float[tr->config.usr_float_A];
	tr->usr_float[1] = usr_float[tr->config.usr_float_B];

	tr->usr_float_struct[0] = usr_float_struct[tr->config.usr_float_A];
	tr->usr_float_struct[1] = usr_float_struct[tr->config.usr_float_B];
}

