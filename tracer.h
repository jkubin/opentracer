/* internal header file
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

#ifndef __TRACER_H
#define __TRACER_H

#include <time.h>
#include <ftdi.h>

#define ftdi_error_return(code, str) do {  \
        ftdi->error_str = str;             \
        return code;                       \
   } while(0)


/*
 * https://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming
 *
 * Modem Status Register (MSR)
 * This register is another read-only register that is here
 * to inform your software about the current status of the modem.
 * The modem accessed in this manner can either be an external modem,
 * or an internal modem that uses a UART as an interface to the computer.
 */

#define TRACE_MODEM_STATUS_DELTA_CTS		0x01	// Delta Clear To Send.
#define TRACE_MODEM_STATUS_DELTA_DSR		0x02	// Delta Data Set Ready.
#define TRACE_MODEM_STATUS_TRAILING_EDGE_RI	0x04	// Trailing Edge Ring Indicator.
#define TRACE_MODEM_STATUS_DELTA_DCD		0x08	// Delta Data Carrier Detect.
#define TRACE_MODEM_STATUS_CTS			0x10	// Clear To Send Control Input / Handshake Signal.
#define TRACE_MODEM_STATUS_DSR			0x20	// Data Set Ready Control Input / Handshake Signal.
#define TRACE_MODEM_STATUS_RI			0x40	// Ring Indicator Control Input. When remote wake up is enabled in the internal EEPROM taking RI# low (20ms active low pulse) can be used to resume the PC USB host controller from suspend.
#define TRACE_MODEM_STATUS_DCD			0x80	// Data Carrier Detect Control Input.

/*
 * Line Status Register (LSR)
 * This register is used primarily to give you information on possible error
 * conditions that may exist within the UART, based on the data that has been received.
 */

#define TRACE_LINE_STATUS_DR			0x0100	// Data Ready.
#define TRACE_LINE_STATUS_OE			0x0200	// Overrun Error.
#define TRACE_LINE_STATUS_PE			0x0400	// Parity Error.
#define TRACE_LINE_STATUS_FE			0x0800	// Framing Error.
#define TRACE_LINE_STATUS_BI			0x1000	// Break Interrupt.
#define TRACE_LINE_STATUS_THRE			0x2000	// Empty Transmitter Holding Register.
#define TRACE_LINE_STATUS_TEMT			0x4000	// Empty Data Holding Registers.
#define TRACE_LINE_STATUS_FIFO			0x8000	// Error in Received FIFO.

#define TRACE_ABORT_ON_EXPIRATION		0b00000000000000000000000000000001
#define TRACE_ABORT_ON_OVERRUN_ERROR		0b00000000000000000000000000000010
#define TRACE_ABORT_ON_PARITY_ERROR		0b00000000000000000000000000000100
#define TRACE_APPEND_ASCII			0b00000000000000000000000000001000
#define TRACE_APPEND_HEX			0b00000000000000000000000000010000
#define TRACE_APPEND_TIME_ELAPSED		0b00000000000000000000000000100000
#define TRACE_APPEND_TIME_REAL			0b00000000000000000000000001000000
#define TRACE_BITF24_AS_SINT24			0b00000000000000000000000010000000
#define TRACE_BITF24_AS_UINT24			0b00000000000000000000000100000000
#define TRACE_DEBUG_MODE			0b00000000000000000000001000000000
#define TRACE_SUPPRESS_CONFIG_SUMMARY		0b00000000000000000000010000000000
#define TRACE_SUPPRESS_HEADERS			0b00000000000000000000100000000000
#define TRACE_SUPPRESS_MSR_IN_XXD		0b00000000000000000001000000000000
#define TRACE_PRINT_TSM				0b00000000000000000010000000000000
#define TRACE_VERBOSE				0b00000000000000000100000000000000
#define TRACE_VERY_VERBOSE			0b00000000000000001000000000000000
#define TRACE_QUIET_TRACER			0b00000000000000010000000000000000
#define TRACE_SHOW_UART_STATUS			0b00000000000000100000000000000000
// #define TRACE_IGNORE_TMR_DATA		0b00000000000000000000000000010000
// #define TRACE_SUPPRESS_RTC			0b00000000000000000010000000000000
// #define TRACE_SUPPRESS_TPOINT_FLOOD		0b00000000000000000000000000010000
// #define TRACE_UNIQ_TPOINT_FLOOD		0b00000000000000000000000000010000
// #define TRACE_VIM_MODELINE			0b00000000000000000000000000000100

#define TRACE_CHECKSUM_ALWAYS			1
#define TRACE_CHECKSUM_BY_CONFIG		0
#define TRACE_CHECKSUM_DATA			1
#define TRACE_MAX_ADDRESS_WIDTH			8
#define TRACE_MAX_BITFIELD			4
#define TRACE_MAX_TMR_WIDTH			8
#define TRACE_MAX_TRACEPOINT_WIDTH		4
#define TRACE_MAX_XXD_HEXDUMP			0x20000
#define TRACE_NULL_CHAR				1
#define TRACE_RLE_WIDTH				2
#define TRACE_BITFIELD_WIDTH			32
#define TRACE_STRUCT_TOTAL			16
#define TRACE_SIZEOF_STRUCT_DATA		0x1000
#define TRACE_SIZEOF_STRUCT_MAX_CAPTION		0x100
#define TRACE_SIZEOF_STRUCT_TYPES		0x400
#define TRACE_TAB_CHAR				1
#define TRACE_TIMER_TIMEOUT			0xa000000000000000

#define TRACE_SIZEOF_DOUBLE_FLOAT		8
#define TRACE_SIZEOF_EXTENDED_FLOAT		10
#define TRACE_SIZEOF_FP24FLT			3
#define TRACE_SIZEOF_HALF_FLOAT			2
#define TRACE_SIZEOF_MCHP_FLOAT			4
#define TRACE_SIZEOF_MINI_FLOAT			1
#define TRACE_SIZEOF_OCTUPLE_FLOAT		32
#define TRACE_SIZEOF_QUAD_FLOAT			16
#define TRACE_SIZEOF_SINGLE_FLOAT		4
#define TRACE_SIZEOF_UNSIGNED_MINI_FLOAT	1
#define TRACE_SIZEOF_XSINGLE_FLOAT		9

#define TRACE_TEST_SUCCESS			0xaa55

#define ARRAY_SIZE(x)			(sizeof(x)/sizeof((x)[0]))
#define STRINGIFY(x)			_STRINGIFY(x)
#define _STRINGIFY(x)			#x

#define TRACE_TARGET_HAS_CRASHED	stderr, "error: target has crashed (probably), last known tsm time is %.9f\n", tr->time.abs_sync

struct trace_context
{
	struct
	{
		unsigned long tmr;
		unsigned int tmr_divisor;
		unsigned int cpu_frequency;

		unsigned short version;

		unsigned char tracepoint_tmr_width;
		unsigned char tsm_tmr_width;	/* TSM (Trace Synchronisation Mark) */

		unsigned char usr_float_A;
		unsigned char usr_float_B;
		unsigned char tracepoint_width;
		unsigned char checksum_level;
		unsigned rts:1;

	} config;
	struct
	{

		double abs_sync;
		double mcu;
		double rtc;
		double rtc_elaps;
		double sync;
		double tick;		/* time to tmr tick */
		time_t rtc_sec;		/* rtc seconds at start */

		unsigned long tsm;
		unsigned long tmr;

		struct timespec start;

	} time;

	double uart_time_rtc;
	double uart_time_rtc_elaps;

	double tsm_time_rtc;
	double tsm_time_rtc_elaps;

	unsigned char *end;
	unsigned char *status;
	unsigned char *end_of_packet;

	char *ascii_ptr;
	char *capt_start;
	char *pcapt;
	char *tty;
	int (*node)(struct trace_context *, unsigned char);
	int (*timestamp)(struct trace_context *);
	int (*usr_float[2])(struct trace_context *);
	struct ftdi_context *ftdi;
	struct trace_struct *tsp;
	unsigned char *pdata;
	unsigned char *ptype;
	unsigned int ncapt;
	void (*usr_float_struct[2])(unsigned char *, unsigned char **const);

	unsigned long addr;
	unsigned long id;

	unsigned int baudrate;
	unsigned int ftdi_lsr_msr;
	unsigned int option;
	unsigned int pid;
	unsigned int uart_status;
	unsigned int vid;
	unsigned int xxd_address;

	enum ftdi_interface interface;
	enum ftdi_parity_type parity;
	enum ftdi_stopbits_type stop_bit;

	unsigned char thread_idx;
	char ascii_bfr[17];
	//unsigned offline:1;
	union
	{
		unsigned char any;
		struct
		{
			unsigned tsm:1;
			unsigned uart_status:1;
		};
	} defered;
	struct trace_struct
	{

		char *capt;
		unsigned char *data;
		unsigned char *type;

		unsigned int ndata;
		unsigned int ntype;

	} ts[TRACE_STRUCT_TOTAL];
};

extern struct trace_record_type
{
	const char *match;
	const char *abbrev;
	unsigned char tabstop;
	unsigned char size;
} trace_grep_float[16];

extern struct trace_base_records
{

	struct trace_record_type bitf;
	struct trace_record_type bitf1;
	struct trace_record_type bitf2;
	struct trace_record_type bitf3;
	struct trace_record_type bitf4;

	struct trace_record_type uint;
	struct trace_record_type uint1;
	struct trace_record_type uint2;
	struct trace_record_type uint3;
	struct trace_record_type uint4;
	struct trace_record_type uint8;

	struct trace_record_type sint;
	struct trace_record_type sint1;
	struct trace_record_type sint2;
	struct trace_record_type sint3;
	struct trace_record_type sint4;
	struct trace_record_type sint8;

	struct trace_record_type ascii;
	struct trace_record_type tsm;
	struct trace_record_type dblf;
	struct trace_record_type hex;
	struct trace_record_type pnt;
	struct trace_record_type pnt2;
	struct trace_record_type pnt3;
	struct trace_record_type pnt4;
	struct trace_record_type trace;
	struct trace_record_type single;
	struct trace_record_type struct_t;
	struct trace_record_type tmr;
	struct trace_record_type txt;
	struct trace_record_type u_struct;
	struct trace_record_type flags;
	struct trace_record_type xxd;

} trace_grep;

// extern const char hex_digit[];
// int trace_baudlist(unsigned int value);
// void trace_append_time_tsm(struct trace_context *tr);
// void trace_append_time_uart(struct trace_context *tr);
char * trace_escape_sequence(char *dst);
extern const char *hex_format[];
extern int (* const trace_std_float[2])(struct trace_context *);
extern unsigned char trace_exit_requested;
extern unsigned int trace_option;
int trace_bitfield(struct trace_context *tr, unsigned char opcode);
int trace_tsm(struct trace_context *tr);
int trace_config(struct trace_context *);
int trace_fetch_data(struct trace_context *, const char *name, unsigned char checksum, unsigned char always, unsigned char *ptr, unsigned char dl);
int trace_ftdi_init_status(struct trace_context *tr);
int trace_ftdi_read_data(struct trace_context *, unsigned char *, unsigned char *, unsigned int);
int trace_init(struct trace_context *);
int trace_libusb(struct trace_context *);
int trace_modeline(struct trace_context *tr, unsigned char opcode);
int trace_normal(struct trace_context *, unsigned char);
int trace_opt(struct trace_context *, int, char **);
int trace_sint(struct trace_context *tr, unsigned char opcode);
int trace_struct_select(struct trace_context *tr, unsigned char opcode);
int trace_struct_selected(struct trace_context *tr, unsigned char opcode);
int trace_struct_types(struct trace_context *tr, unsigned char opcode);
int trace_struct_types_raw(struct trace_context *tr, unsigned char opcode);
int trace_struct_types_rle(struct trace_context *tr, unsigned char opcode);
int trace_synced(void /*struct trace_context *, int */);
int trace_text(struct trace_context *tr, unsigned char opcode);
int trace_time_from_file(struct trace_context *tr);
int trace_time_init(struct trace_context *tr);
int trace_tracepoint(struct trace_context *tr, unsigned char opcode);
int trace_tty(struct trace_context *);
int trace_txt(struct trace_context *tr, unsigned char opcode);
int trace_uint(struct trace_context *tr, unsigned char opcode);
int trace_xxd(struct trace_context *tr, unsigned char opcode);
void trace_append_time(struct trace_context *tr);
void trace_base_captions(void);
void trace_bitfield_print(struct trace_context *tr, unsigned char *ptr, unsigned char **const ppdata, unsigned char dl);
void trace_bitfield_to_bfr(char *bfr, unsigned int num, unsigned char width);
void trace_caption_hex_time(void);
void trace_caption_time(void);
void trace_tsm_print(struct trace_context *tr);
void trace_defered_print(struct trace_context *tr);
void trace_free(struct trace_struct *);
void trace_init_handlers(struct trace_context *tr);
void trace_txt_flush(struct trace_context *tr);
void trace_uart_status_print(struct trace_context *tr);
void trace_xxd_flush(struct trace_context *tr);
void trace_xxd_print(struct trace_context *, unsigned char *, unsigned char, unsigned int);

#endif	/* __TRACER_H */

