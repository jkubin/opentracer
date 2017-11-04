/* unfinished bunch of options
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
#include <stdlib.h>
#include <getopt.h>
#include "tracer.h"
#include "opentracer.h"

unsigned int trace_option;



enum
{
	TRACE_OPT_BITFIELD = 128,

	TRACE_OPT_DOUBLE,
	TRACE_OPT_EXTENDED,
	TRACE_OPT_FP24FLT,
	TRACE_OPT_HALF,
	TRACE_OPT_MCHP,
	TRACE_OPT_MINI,
	TRACE_OPT_OCTUPLE,
	TRACE_OPT_PNT,
	TRACE_OPT_QUAD,
	TRACE_OPT_SINGLE,
	TRACE_OPT_SINT,
	TRACE_OPT_STRUCT,
	TRACE_OPT_TRACE,
	TRACE_OPT_TSM,
	TRACE_OPT_TXT,
	TRACE_OPT_UART,
	TRACE_OPT_UINT,
	TRACE_OPT_UMIN,
	TRACE_OPT_XSINGLE,
	TRACE_OPT_XXD,

};


const struct option long_options[] = {

	{"all", no_argument, NULL, 'a'},
	{"append-ascii", no_argument, NULL, 'A'},
	{"append-hex", no_argument, NULL, 'x'},
	{"baudlist", required_argument, NULL, 'l'},
	{"bitf24-as-sint24", no_argument, NULL, 'S'},
	{"bitf24-as-uint24", no_argument, NULL, 'u'},
	{"device-tty", required_argument, NULL, 't'},
	{"elapsed-time", no_argument, NULL, 'e'},
	{"fast-mode", no_argument, NULL, 'f'},
	{"from-time", required_argument, NULL, 'F'},
	{"help", no_argument, NULL, 'h'},
	{"interface", required_argument, NULL, 'i'},
	{"max-xxd", required_argument, NULL, 'm'},
	{"output-binary-file", required_argument, NULL, 'o'},
	{"parity", required_argument, NULL, 'p'},
	{"quiet-mode", no_argument, NULL, 'q'},
	{"real-time", no_argument, NULL, 'r'},
	{"serial-num", required_argument, NULL, 's'},
	{"stop-bit", required_argument, NULL, 'k'},
	{"to-time", required_argument, NULL, 'T'},
	{"verbose", no_argument, NULL, 'v'},

	{"ts-bitfield", required_argument, NULL, TRACE_OPT_BITFIELD},
	{"ts-tsm", required_argument, NULL, TRACE_OPT_TSM},
	{"ts-double", required_argument, NULL, TRACE_OPT_DOUBLE},
	{"ts-extended", required_argument, NULL, TRACE_OPT_EXTENDED},
	{"ts-flags", required_argument, NULL, TRACE_OPT_UART},
	{"ts-fp24flt", required_argument, NULL, TRACE_OPT_FP24FLT},
	{"ts-half", required_argument, NULL, TRACE_OPT_HALF},
	{"ts-mchp", required_argument, NULL, TRACE_OPT_MCHP},
	{"ts-mini", required_argument, NULL, TRACE_OPT_MINI},
	{"ts-octuple", required_argument, NULL, TRACE_OPT_OCTUPLE},
	{"ts-pnt", required_argument, NULL, TRACE_OPT_PNT},
	{"ts-quad", required_argument, NULL, TRACE_OPT_QUAD},
	{"ts-trace", required_argument, NULL, TRACE_OPT_TRACE},
	{"ts-single", required_argument, NULL, TRACE_OPT_SINGLE},
	{"ts-sint", required_argument, NULL, TRACE_OPT_SINT},
	{"ts-struct", required_argument, NULL, TRACE_OPT_STRUCT},
	{"ts-txt", required_argument, NULL, TRACE_OPT_TXT},
	{"ts-uint", required_argument, NULL, TRACE_OPT_UINT},
	{"ts-umin", required_argument, NULL, TRACE_OPT_UMIN},
	{"ts-xsingle", required_argument, NULL, TRACE_OPT_XSINGLE},
	{"ts-xxd", required_argument, NULL, TRACE_OPT_XXD},

	{0, 0, 0, 0},
};



static const char short_options[] =

"A"
"F:"
"S"
"T:"
"a"
"b:"
"e"
"f"
"h"
"i:"
"k:"
"l"
"m:"
"o:"
"p:"
"q:"
"r"
"s:"
"t:"
"u"
"v"
"x"

;



static const char usage[] =

"Usage: opentracer [options]\n"
"options:\n"

""
""
""
""

"-A, --ascii                      append ASCII to hex (struct data)\n"
"-F, --from-time MMDDhhmmss.frac  print offline trace records _from_ time (elapsed <= 99999999.9999 < absolut)\n"
"-S, --bitf24-as-sint24           print 24b bitfield as 24b signed int\n"
"-T, --to-time   MMDDhhmmss.frac  print offline trace records _to_ time (elapsed <= 99999999.9999 < absolut)\n"
"-a, --all-ftdi-devices           list all ftdi devices\n"
"-b, --baudspeed           SPEED  baudspeed [Bd]\n"
"-e, --elapsed-time               append elapsed time\n"
"-f, --fast-mode                  fast mode\n"
"-h, --help                       print this message and exit\n"
"-i, --interface         [01234]  ftdi interface: any = 0; A = 1; B = 2; C = 3; D = 4\n"
"-k, --stop-bit            [012]  stop bit 0 = 1; 1 = 1.5; 2 = 2\n"
"-l, --baudlist                   print baudlist of a (first/selected) device\n"
"-m, --max-xxd               NUM  maximum xxd block size to log file (" STRINGIFY(TRACE_MAX_XXD_HEXDUMP) " B), unlimited = 0\n"
"-o, --output-binary-file   NAME  explicit trace binary file name\n"
"-p, --parity              [012]  none = 0; odd = 1; even = 2\n"
"-q, --quiet-mode                 quiet mode is significantly faster (trace data saves to file)\n"
"-r, --real-time                  append real time\n"
"-s, --serial-num         SERIAL  select ftdi device by serial number\n"
"-t, --device-tty   /dev/ttyUSB0  serial device (not recommended: try to use libusb for direct access to a device)\n"
"-u, --bitf24-as-uint24           print 24b bitfield as 24b unsigned int\n"
"-v, --verbose                    +verbose level\n"
"-x, --append-hex                 append hex value to trace data\n"
// "\n"
// "\n"
// "\n"
// "\n"
// "\n"
// "\n"
// "\n"

"\n"

"    --ts-bitfield                tabstop bitfield\n"
"    --ts-double                  tabstop double float\n"
"    --ts-extended                tabstop extended float\n"
"    --ts-flags-line              tabstop flags of line\n"
"    --ts-fp24flt                 tabstop fp24flt float\n"
"    --ts-half                    tabstop half float\n"
"    --ts-mchp                    tabstop mchp float\n"
"    --ts-mini                    tabstop mini float\n"
"    --ts-octuple                 tabstop octuple float\n"
"    --ts-pnt                     tabstop pnt\n"
"    --ts-quad                    tabstop quad float\n"
"    --ts-single                  tabstop single float\n"
"    --ts-sint                    tabstop sint\n"
"    --ts-struct                  tabstop struct\n"
"    --ts-trace                   tabstop trace (all data)\n"
"    --ts-tsm                     tabstop trace synchronisation mark\n"
"    --ts-txt                     tabstop txt\n"
"    --ts-uint                    tabstop uint\n"
"    --ts-umin                    tabstop umin float\n"
"    --ts-xsingle                 tabstop xsingle float\n"
"    --ts-xxd                     tabstop xxd\n"

//"-b, --binary-trace-file    NAME  binary trace file name\n"

;



int trace_opt(struct trace_context *tr, int argc, char **argv)
{

	int opt;

	while ((opt = getopt_long(argc, argv, short_options, long_options,
					(int *)0)) != EOF) {
		switch (opt) {
			case 'p':
				break;

			case TRACE_OPT_BITFIELD:
				trace_grep.bitf.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_TSM:
				trace_grep.tsm.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_DOUBLE:
				trace_grep.dblf.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_EXTENDED:
				trace_grep_float[TRACE_EXTENDED_FLOAT].tabstop = atoi(optarg);
				break;
			case TRACE_OPT_FP24FLT:
				trace_grep_float[TRACE_FP24FLT].tabstop = atoi(optarg);
				break;
			case TRACE_OPT_HALF:
				trace_grep_float[TRACE_HALF_FLOAT].tabstop = atoi(optarg);
				break;
			case TRACE_OPT_MCHP:
				trace_grep_float[TRACE_MCHP_FLOAT].tabstop = atoi(optarg);
				break;
			case TRACE_OPT_MINI:
				trace_grep_float[TRACE_MINI_FLOAT].tabstop = atoi(optarg);
				break;
			case TRACE_OPT_OCTUPLE:
				trace_grep_float[TRACE_OCTUPLE_FLOAT].tabstop = atoi(optarg);
				break;
			case TRACE_OPT_PNT:
				trace_grep.pnt.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_QUAD:
				trace_grep_float[TRACE_QUAD_FLOAT].tabstop = atoi(optarg);
				break;
			case TRACE_OPT_TRACE:
				trace_grep.trace.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_SINGLE:
				trace_grep.single.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_SINT:
				trace_grep.sint.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_STRUCT:
				trace_grep.struct_t.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_TXT:
				trace_grep.txt.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_UART:
				trace_grep.flags.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_UINT:
				trace_grep.uint.tabstop = atoi(optarg);
				break;
			case TRACE_OPT_UMIN:
				trace_grep_float[TRACE_UNSIGNED_MINI_FLOAT].tabstop = atoi(optarg);
				break;
			case TRACE_OPT_XSINGLE:
				trace_grep_float[TRACE_XSINGLE_FLOAT].tabstop = atoi(optarg);
				break;
			case TRACE_OPT_XXD:
				trace_grep.xxd.tabstop = atoi(optarg);
				break;
		}
	}

	trace_option |=
		// TRACE_APPEND_HEX |
		// TRACE_APPEND_ASCII |
		// TRACE_BITF24_AS_UINT24 |
		TRACE_APPEND_TIME_ELAPSED |
		TRACE_APPEND_TIME_REAL |
		// TRACE_SHOW_UART_STATUS |
		0;



	return 0;
}

