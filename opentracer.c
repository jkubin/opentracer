/* opentracer is a tracing tool for small processors
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
#include "tracer.h"



static struct trace_context tr = {
	.vid = 0x0403,
	.pid = 0x6010,
	//.interface = INTERFACE_ANY,
	//.baudrate = 200321,	// PICDEM.net
	//.baudrate = 297619,	// PICDEM.net
	.baudrate = 347222,	// PICDEM.net
	//.baudrate = 400641,	// PICDEM.net	obcas (OE)
	//.baudrate = 120480,
	//.baudrate = 200000,
	//.baudrate = 125000,
	//.baudrate = 250000,
	//.baudrate = 500000,
	//.baudrate = 1000000,
	//.baudrate = 3000000,
	//.baudrate = 3125000,
	//.baudrate = 8000000,
	//.baudrate = 5208333,
	.parity = NONE,
	.stop_bit = STOP_BIT_1,
};

unsigned char trace_exit_requested;


int main(int argc, char **argv)
{

	int f;

	if ((f = trace_opt(&tr, argc, argv)))
		return f;

	if (trace_init(&tr)) {

		fprintf(stderr, "error: out of memory\n");

		return EXIT_FAILURE;
	}

	//signal(SIGINT, sigintHandler);

	if (tr.tty) {
		// f = trace_tty(&tr);
	} else {
		if (!(tr.ftdi = ftdi_new())) {

			fprintf(stderr, "error: ftdi_new failed\n");

			return EXIT_FAILURE;
		}

		f = trace_libusb(&tr);

		ftdi_usb_close(tr.ftdi);

		ftdi_free(tr.ftdi);
	}

	trace_free(tr.ts);

	return f;
}

