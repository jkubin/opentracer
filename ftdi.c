/* an ftdi bridge as a source of data
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
#include <unistd.h>	// sleep
#include "opentracer.h"
#include "tracer.h"

// static int trace_ftdi(struct trace_context *tr, unsigned char *opcode)
// {
// 	return f;
// }

int trace_libusb(struct trace_context *tr)
{

	int f;
	unsigned char opcode;

	tr->option =
		TRACE_PRINT_TSM |
		0;

	f = ftdi_set_interface(tr->ftdi, tr->interface);

	if (f < 0) {
		fprintf(stderr, "error: unable to set interface: %d (%s)\n", f, ftdi_get_error_string(tr->ftdi));

		return EXIT_FAILURE;
	}

	f = ftdi_usb_open(tr->ftdi, tr->vid, tr->pid);

	if (f < 0) {
		fprintf(stderr, "error: unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(tr->ftdi));

		return EXIT_FAILURE;
	}

	f = ftdi_usb_reset(tr->ftdi);

	if (f < 0) {
		fprintf(stderr, "error: %s (%d) " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n", ftdi_get_error_string(tr->ftdi), f);

		return EXIT_FAILURE;
	}

	// sleep(1);

	f = ftdi_set_baudrate(tr->ftdi, tr->baudrate);

	if (f < 0) {
		fprintf(stderr, "error: unable to set baudrate: %d (%s)\n", f, ftdi_get_error_string(tr->ftdi));

		return EXIT_FAILURE;
	}

	f = ftdi_set_line_property(tr->ftdi, 8, tr->stop_bit, tr->parity);

	if (f < 0) {
		fprintf(stderr, "error: unable to set line parameters: %d (%s)\n", f, ftdi_get_error_string(tr->ftdi));

		return EXIT_FAILURE;
	}



	trace_base_captions();





	f = trace_ftdi_init_status(tr);

	if (f < 0) {

		fprintf(stderr, "error: %s (%d) " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n", ftdi_get_error_string(tr->ftdi), f);

		return EXIT_FAILURE;
	}



	f = trace_ftdi_read_data(tr, NULL, &opcode, 1);

	if (f < 0) {

		fprintf(stderr, "error: %s (%d) " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n", ftdi_get_error_string(tr->ftdi), f);
			// fprintf(stderr, "error: %s (%d)\n", ftdi_get_error_string(tr->ftdi), f);

		return EXIT_FAILURE;
	}

	if (opcode != TRACE_start) {

		fprintf(stderr, "error: unexpected start symbol (received 0x%02x, expected 0x00)\n", opcode);

		return EXIT_FAILURE;
	}

gbr:
	if (trace_config(tr))
		return EXIT_FAILURE;

	while (1) {

			f = trace_ftdi_read_data(tr, &trace_exit_requested, &opcode, 1);

			if (f < 0) {

				fprintf(stderr, "error: %s (%d) " STRINGIFY(__FILE__) " " STRINGIFY(__LINE__) "\n", ftdi_get_error_string(tr->ftdi), f);

				break;
			}



		if ((f = tr->node(tr, opcode)))
			break;
	}

	if (tr->node == trace_xxd)
		trace_xxd_flush(tr);

	if (tr->node == trace_txt)
		trace_txt_flush(tr);

	if (tr->defered.any)
		trace_defered_print(tr);

	if (f == TRACE_TEST_SUCCESS)
		return EXIT_SUCCESS;



	return f;
}

