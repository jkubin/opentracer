/* global opcodes for tracing and configuration
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

#ifndef __OPENTRACER_H
#define __OPENTRACER_H

#define TRACE_VERSION	1
#define TRACE_CONFIG_VERSION			0x0000

/* trace commands */

#define TRACE_uint(x)		(TRACE_OPCODE_INT | TRACE_SUBCODE_UINT | ((x) - 1))
#define TRACE_uint8_t		(TRACE_OPCODE_INT | TRACE_SUBCODE_UINT | 0)
#define TRACE_uint16_t		(TRACE_OPCODE_INT | TRACE_SUBCODE_UINT | 1)
#define TRACE_uint32_t		(TRACE_OPCODE_INT | TRACE_SUBCODE_UINT | 2)
#define TRACE_uint64_t		(TRACE_OPCODE_INT | TRACE_SUBCODE_UINT | 3)

#define TRACE_sint(x)		(TRACE_OPCODE_INT | ((x) - 1))
#define TRACE_sint8_t		(TRACE_OPCODE_INT | 0)
#define TRACE_sint16_t		(TRACE_OPCODE_INT | 1)
#define TRACE_sint32_t		(TRACE_OPCODE_INT | 2)
#define TRACE_sint64_t		(TRACE_OPCODE_INT | 3)

#define TRACE_bitf(x)		(TRACE_SUBCODE_BITFIELD | ((x) - 1))
#define TRACE_bitf8_t		(TRACE_SUBCODE_BITFIELD | 0)
#define TRACE_bitf16_t		(TRACE_SUBCODE_BITFIELD | 1)
#define TRACE_bitf24_t		(TRACE_SUBCODE_BITFIELD | 2)
#define TRACE_bitf32_t		(TRACE_SUBCODE_BITFIELD | 3)

#define TRACE_usr_float_A	(TRACE_OPCODE_MISC | TRACE_SUBCODE_FLOAT_A)
#define TRACE_usr_float_B	(TRACE_OPCODE_MISC | TRACE_SUBCODE_FLOAT_B)
#define TRACE_single		(TRACE_OPCODE_FLOAT | TRACE_SUBCODE_SINGLE)
#define TRACE_double		(TRACE_OPCODE_FLOAT | TRACE_SUBCODE_DOUBLE)

#define TRACE_break		0x00
#define TRACE_tsm		(TRACE_OPCODE_TSM)
#define TRACE_data(x)		TRACE_hxd(x)
#define TRACE_data_max		(TRACE_OPCODE_DATA)
#define TRACE_hxd(x)		(TRACE_OPCODE_DATA | ((x) - 1))
#define TRACE_hxd_max		(TRACE_OPCODE_DATA)
#define TRACE_rle(hi)		(TRACE_OPCODE_MISC | TRACE_SUBCODE_RLE | (hi))
#define TRACE_ssel(x)		(TRACE_OPCODE_STRUCT_SELECT | (x))
#define TRACE_start		0x00
#define TRACE_pnt(x)		(TRACE_OPCODE_TRACEPOINT | (x))
#define TRACE_txt(x)		(TRACE_OPCODE_TXT | ((x) - 1))
#define TRACE_txt_max		(TRACE_OPCODE_TXT - 4)

/* end of trace commands */

#define TRACE_OPCODE_TRACEPOINT		0x80
#define TRACE_OPCODE_DATA		0x40
#define TRACE_OPCODE_TXT		0x20
#define TRACE_OPCODE_STRUCT_SELECT	0x10
#define TRACE_OPCODE_INT		0x08
#define TRACE_OPCODE_MISC		0x04
#define TRACE_OPCODE_FLOAT		0x02
#define TRACE_OPCODE_TSM		0x01

#define TRACE_SUBCODE_BITFIELD		0x3c
#define TRACE_SUBCODE_DOUBLE		0x01
#define TRACE_SUBCODE_FLOAT_A		0x00
#define TRACE_SUBCODE_FLOAT_B		0x01
#define TRACE_SUBCODE_RLE		0x02
#define TRACE_SUBCODE_SINGLE		0x00
#define TRACE_SUBCODE_UINT		0x04

// #define TRACE_CFG_CHECKSUM_LEVEL(x)	(x)
// #define TRACE_CFG_RTS(x)		(x)
// #define TRACE_CFG_TRACEPOINT_WIDTH(x)	((x) - 1)
// #define TRACE_CFG_VERSION(x)		(x)

enum trace_user_defined_float_index
{
	TRACE_MINI_FLOAT,
	TRACE_UNSIGNED_MINI_FLOAT,
	TRACE_HALF_FLOAT,
	TRACE_FP24FLT,
	TRACE_MCHP_FLOAT,
	TRACE_XSINGLE_FLOAT,
	TRACE_EXTENDED_FLOAT,
	TRACE_QUAD_FLOAT,
	TRACE_OCTUPLE_FLOAT,
};

enum trace_config_index
{
	TRACE_PIC18_NO_TIMER,
	TRACE_PIC18_1B_LO_TIMER,
	TRACE_PIC18_1B_HI_TIMER,
	TRACE_PIC18_2B_TIMER,
};



enum trace_config_type
{
	/* struct trace_config_plain_points */
	TRACE_128_PLAIN_POINTS,			// 1B total width
	TRACE_32768_PLAIN_POINTS,		// 2B total width
	TRACE_8388608_PLAIN_POINTS,		// 3B total width
	TRACE_2147483648_PLAIN_POINTS,		// 4B total width

	/* struct trace_config_time_points */
	TRACE_128_POINTS_1B_TIMER,		// 2B total width
	TRACE_32768_POINTS_1B_TIMER,		// 3B total width
	TRACE_8388608_POINTS_1B_TIMER,		// 4B total width
	TRACE_2147483648_POINTS_1B_TIMER,	// 5B total width

	/* struct trace_config_time_points */
	TRACE_128_POINTS_2B_TIMER,		// 3B total width
	TRACE_32768_POINTS_2B_TIMER,		// 4B total width
	TRACE_8388608_POINTS_2B_TIMER,		// 5B total width
	TRACE_2147483648_POINTS_2B_TIMER,	// 6B total width

	/* struct trace_config_time_points */
	TRACE_128_POINTS_3B_TIMER,		// 4B total width
	TRACE_32768_POINTS_3B_TIMER,		// 5B total width
	TRACE_8388608_POINTS_3B_TIMER,		// 6B total width
	TRACE_2147483648_POINTS_3B_TIMER,	// 7B total width

	/* struct trace_config_time_points */
	TRACE_128_POINTS_4B_TIMER,		// 5B total width
	TRACE_32768_POINTS_4B_TIMER,		// 6B total width
	TRACE_8388608_POINTS_4B_TIMER,		// 7B total width
	TRACE_2147483648_POINTS_4B_TIMER,	// 8B total width


	TRACE_NOT_USED_KEEP_IT_LAST,

};

#endif	/* __OPENTRACER_H */

