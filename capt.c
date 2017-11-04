/* trace log strings intended for perl/grep/sed/awk/... processing
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

struct trace_record_type trace_grep_float[] = {
	/* match,		abbrev,								tabstop,	size,*/
	{"mini\n",		"/mi_" STRINGIFY(TRACE_SIZEOF_MINI_FLOAT)		"B\t",	24,		TRACE_SIZEOF_MINI_FLOAT,},
	{"umin\n",		"/um_" STRINGIFY(TRACE_SIZEOF_UNSIGNED_MINI_FLOAT)	"B\t",	24,		TRACE_SIZEOF_UNSIGNED_MINI_FLOAT,},
	{"half\n",		"/ha_" STRINGIFY(TRACE_SIZEOF_HALF_FLOAT)		"B\t",	24,		TRACE_SIZEOF_HALF_FLOAT,},
	{"fp24flt\n",		"/fp_" STRINGIFY(TRACE_SIZEOF_FP24FLT)			"B\t",	24,		TRACE_SIZEOF_FP24FLT,},
	{"mchp\n",		"/mc_" STRINGIFY(TRACE_SIZEOF_MCHP_FLOAT)		"B\t",	24,		TRACE_SIZEOF_MCHP_FLOAT,},
	{"xsngl\n",		"/xs_" STRINGIFY(TRACE_SIZEOF_XSINGLE_FLOAT)		"B\t",	24,		TRACE_SIZEOF_XSINGLE_FLOAT,},
	{"extended\n",		"/ex_" STRINGIFY(TRACE_SIZEOF_EXTENDED_FLOAT)		"B\t",	24,		TRACE_SIZEOF_EXTENDED_FLOAT,},
	{"quad\n",		"/qu_" STRINGIFY(TRACE_SIZEOF_QUAD_FLOAT)		"B\t",	24,		TRACE_SIZEOF_QUAD_FLOAT,},
	{"octuple\n",		"/oc_" STRINGIFY(TRACE_SIZEOF_OCTUPLE_FLOAT)		"B\t",	24,		TRACE_SIZEOF_OCTUPLE_FLOAT,},
	{"floating_godzilla\n",	"/go_" STRINGIFY(64)					"B\t",	24,		64,},
	{"floating_atombomb\n",	"/at_" STRINGIFY(128)					"B\t",	24,		128,},
	{"floating_guru\n",	"/gu_" STRINGIFY(255)					"B\t",	24,		255,},
	{"floating_william\n",	"/wi_" STRINGIFY(255)					"B\t",	24,		255,},
	{"floating_morton\n",	"/mo_" STRINGIFY(255)					"B\t",	24,		255,},
	{"floating_kahan\n",	"/ka_" STRINGIFY(255)					"B\t",	24,		255,},
	{"floating_padding\n",	"/pa_" STRINGIFY(255)					"B\t",	24,		255,},
};

struct trace_base_records trace_grep = {
/*		match,			abbrev,		tabstop,	size, */
/* bitf */	{"%u_bitf\n",		"",		34,		0},
/* bitf8 */	{"1_bitf\n",		"/bi_1B\t",	11,		0},
/* bitf16 */	{"2_bitf\n",		"/bi_2B\t",	19,		0},
/* bitf24 */	{"3_bitf\n",		"/bi_3B\t",	27,		0},
/* bitf32 */	{"4_bitf\n",		"/bi_4B\t",	35,		0},

/* uint */	{"%u_uint\n",		"",		24,		0},
/* uint8 */	{"1_uint\n",		"/ui_1B\t",	17,		0},
/* uint16 */	{"2_uint\n",		"/ui_2B\t",	17,		0},
/* uint24 */	{"3_uint\n",		"/ui_3B\t",	19,		0},
/* uint32 */	{"4_uint\n",		"/ui_4B\t",	19,		0},
/* uint64 */	{"8_uint\n",		"/ui_8B\t",	19,		0},

/* sint */	{"%u_sint\n",		"",		32,		0},
/* sint8 */	{"1_sint\n",		"/si_1B\t",	17,		0},
/* sint16 */	{"2_sint\n",		"/si_2B\t",	17,		0},
/* sint24 */	{"3_sint\n",		"/si_3B\t",	24,		0},
/* sint32 */	{"4_sint\n",		"/si_4B\t",	14,		0},
/* sint64 */	{"8_sint\n",		"/si_8B\t",	22,		0},

/* ascii */	{"",			"/ascii\t",	0,		0},
/* tsm */	{"tsm\n",		"",		17,		0},
/* dblf */	{"double\n",		"/do_8B\t",	17,		0},
/* hex */	{"",			"/hex\t",	0,		0},
/* pnt */	{"pnt\n",		"",		17,		0},
/* pnt2 */	{"",			"",		20,		0},
/* pnt3 */	{"",			"",		28,		0},
/* pnt4 */	{"",			"",		36,		0},
/* tr */	{"trace\n",		"",		8,		0},
/* single */	{"single\n",		"/sin_4B\t",	17,		0},
/* struct_t */	{"struct\n",		"",		24,		0},
/* tmr */	{"",			"/tmr\t",	24,		0},
/* txt */	{"txt\n",		"/tx_%uB\t",	29,		0},
/* u_struct */	{"%uB/%u_struct\n",	"",		24,		0},
/* flags */	{"flg\n",		"",		6,		0},
/* xxd */	{"xxd\n",		"/hx_%uB\t",	20,		0},

/* name */	/* {"",			"",		24,		0},*/
};

void trace_caption_time(void)
{
	if (trace_option & TRACE_APPEND_TIME_ELAPSED)
		printf("elapstime\t");

	if (trace_option & TRACE_APPEND_TIME_REAL)
		printf("MMDDhhmmss.frac\t");
}

void trace_caption_hex_time(void)
{
	if (trace_option & TRACE_APPEND_HEX)
		printf(&trace_grep.hex.abbrev[1]);

	trace_caption_time();
}

void trace_base_captions(void)
{

	// if (!(tr->option & TRACE_SUPPRESS_HEADERS))

	// printf("# modeline, see http://vim.wikia.com/wiki/Modeline_magic\n");

	printf(" vim:ts=%u:ft=%s",
			trace_grep.trace.tabstop,
			trace_grep.trace.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.txt.tabstop,
			trace_grep.txt.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.tsm.tabstop,
			trace_grep.tsm.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.bitf1.tabstop,
			trace_grep.bitf1.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.bitf2.tabstop,
			trace_grep.bitf2.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.bitf3.tabstop,
			trace_grep.bitf3.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.bitf4.tabstop,
			trace_grep.bitf4.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.uint1.tabstop,
			trace_grep.uint1.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.uint2.tabstop,
			trace_grep.uint2.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.uint3.tabstop,
			trace_grep.uint3.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.uint4.tabstop,
			trace_grep.uint4.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.uint8.tabstop,
			trace_grep.uint8.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.sint1.tabstop,
			trace_grep.sint1.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.sint2.tabstop,
			trace_grep.sint2.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.sint3.tabstop,
			trace_grep.sint3.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.sint4.tabstop,
			trace_grep.sint4.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.sint8.tabstop,
			trace_grep.sint8.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.single.tabstop,
			trace_grep.single.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.dblf.tabstop,
			trace_grep.dblf.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.flags.tabstop,
			trace_grep.flags.match);

	printf(" vim:ts=%u:ft=%s",
			trace_grep.struct_t.tabstop,
			trace_grep.struct_t.match);

	/* xxd (hexdump) keep it last, because the next empty line is required for easy awk-ing */
	printf(" vim:ts=%u:ft=%s\n",
			trace_grep.xxd.tabstop,
			trace_grep.xxd.match);

	/* txt (a text note or name of a variable)	 <--- keep it first because of name ??? */
	printf(" val\t");
	trace_caption_time();
	printf(trace_grep.txt.match);


	/* bitf (bitfield) */
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.bitf1.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.bitf2.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.bitf3.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.bitf4.match);


	/* uint */
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.uint1.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.uint2.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.uint3.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.uint4.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.uint8.match);


	/* sint */
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.sint1.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.sint2.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.sint3.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.sint4.match);
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.sint8.match);


	/* single float */
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.single.match);


	/* double float */
	printf(" val\t");
	trace_caption_hex_time();
	printf(trace_grep.dblf.match);


	/* xxd (hexdump) keep it last, because next empty line is required for correct awk-ing */
	// printf(" offset\t");
	printf("id\t");
	trace_caption_time();
	printf(trace_grep.xxd.match);
	printf("\n");


	/*
	 *        UART status registers:
	 *
	 *        Line Status Register (LSR)	Modem Status Register (MSR) */
	printf(" FIFO\tTEMT\tTHRE\tBI\tFE\tPE\tOE\tDR\tDCD\tRI\tDSR\tCTS\tΔDCD\tTERI\tΔDSR\tΔCTS\t");
	trace_caption_hex_time();
	printf(trace_grep.flags.match);

}

