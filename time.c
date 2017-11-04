/* timestamp handlers
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
#include "tracer.h"

static int trace_time_local(struct trace_context *tr)
{

	double frac;
	double rtc_elaps;
	struct timespec diff;
	struct timespec now;
	struct timespec start;
	struct tm tm;
	time_t diff_sec;
	time_t rtc_sec;

	clock_gettime(CLOCK_MONOTONIC_RAW, &now);

	start = tr->time.start;

	diff.tv_sec = now.tv_sec - start.tv_sec;
	diff.tv_nsec = now.tv_nsec - start.tv_nsec;

	tr->time.rtc_elaps =
		rtc_elaps =
		diff.tv_sec + diff.tv_nsec / 1.0e9;

	if (!(trace_option & TRACE_APPEND_TIME_REAL))
		return 0;

	diff_sec = rtc_elaps;

	frac = rtc_elaps - diff_sec;

	rtc_sec = tr->time.rtc_sec;

	rtc_sec += diff_sec;

	/*
	// FIXME tohle se muze udelat v offline modu
	// na zacatku ulozit startovaci cas, pak uz jen ukladat casovac
	// kdyz to budu vyhodnocovat, pak pustit tuhle hruzu
	*/

	tm = *localtime(&rtc_sec);

	tr->time.rtc =
		// (tm.tm_year - 100) * 1e10 +	<--- removing of the year increases double float resolution
		(tm.tm_mon + 1) * 1e8 +
		tm.tm_mday * 1e6 +
		tm.tm_hour * 1e4 +
		tm.tm_min * 1e2 +
		tm.tm_sec +
		frac;

	return 0;
}

int trace_time_init(struct trace_context *tr)
{

	struct timespec now;
	struct timespec rtc;
	struct tm tm;

	clock_gettime(CLOCK_MONOTONIC_RAW, &now);

	clock_gettime(CLOCK_REALTIME, &rtc);
	tr->time.rtc_sec = rtc.tv_sec;
	tr->time.start = now;

	tr->timestamp = trace_time_local;

	tm = *localtime(&rtc.tv_sec);

	tr->time.rtc =
		// (tm.tm_year - 100) * 1e10 +	<--- removing of the year increases double float resolution
		(tm.tm_mon + 1) * 1e8 +
		tm.tm_mday * 1e6 +
		tm.tm_hour * 1e4 +
		tm.tm_min * 1e2 +
		tm.tm_sec;

	return 0;
}

int trace_synced(void /*struct trace_context *tr, int f */)
{
	/*
	if (tr->tsm) {
		return 0;
	}

	 * jednou za nekolik prazdnych cyklu se podivam, kolik je hodin, abych nezatezoval prijem dat
	 * bude to brat parametr, jestli to prijalo data, nebo byl timeout
	 * tohle pujde vypnout prepinacem
	 *
	 * pocet prazdnych cyklu bude zavyset na deklaraci sync doby prectene z MCU
	 *
	 * po startu obvykle cekam ve smycce
	 * takze mit priznak, ze uz to neco prijalo
	 * aby tr nebylo predcasne ukonceno touto funkci
	 */

#if 0
	unsigned int diff;
	unsigned int tmr;
	double time;

	diff = time - tr->time.abs_sync;

	/*
	 * tady testuji, jestli neuvazlo posilani sync
	 * pokud dlouho neprisel sync, pak dojde k ukonceni
	 * programu s chybovou hlaskou
	 *
	 * do hodnoty tmr spocitam aktualni cas
	 */

	if (tr->synced) {

		if (diff > SYNC_UPPER_THRESHOLD) {

			fprintf(stderr, "error: expiration sync; last known sync time %.9f\n", tr->time.sync);

			return -1;
		}
	}

	/*
	 * uplynulou dobu mezi sync merit pocitacem
	 * uplatni se, kdyz je sirka timeru 0B
	 *
	 * posledni mereni resit pres cas pocitace
	 * ulozit cas z pocitace

	 if (tr->option & TRACE_IGNORE_TMR_DATA)

	// tr->tsm = 1;	???

	return 0;
	*/
#endif

	return 0;
}

/*
int trace_time_from_file(struct trace_context *tr)
{
	return 0;
}
*/

void trace_append_time(struct trace_context *tr)
{
	if (trace_option & TRACE_APPEND_TIME_ELAPSED)
		printf("%.6f\t", tr->time.rtc_elaps);

	if (trace_option & TRACE_APPEND_TIME_REAL) {
		//if (tr->time.rtc > 1000000000)
			printf("%.5f\t", tr->time.rtc);
		//else
			//printf(" %.7f\t", tr->time.rtc);
	}
}

/*
static void uart_status_to_bfr(char *ptr, unsigned int status)
{

	unsigned char i;

	for (i = 0; i < 16; i++, status <<= 1) {

		if (status & 0x8000)
			*ptr = '1';
		else
			*ptr = '0';

		ptr++;

		*ptr = '\t';

		ptr++;
	}

	*ptr = 0;
}
*/

void trace_uart_status_print(struct trace_context *tr)
{

	char *ptr;
	char txt[32 + TRACE_NULL_CHAR];
	unsigned char i;
	unsigned int status;

	ptr = txt;
	status = tr->uart_status;

	for (i = 0; i < 16; i++, status <<= 1) {

		if (status & 0x8000)
			*ptr = '1';
		else
			*ptr = '0';

		ptr++;

		*ptr = '\t';

		ptr++;
	}

	*ptr = 0;

	printf(txt);

	if (trace_option & TRACE_APPEND_HEX)
		printf("0x%04x\t", tr->uart_status);

	if (trace_option & TRACE_APPEND_TIME_ELAPSED)
		printf("%.6f\t", tr->uart_time_rtc_elaps);

	if (trace_option & TRACE_APPEND_TIME_REAL)
		printf("%.6f\t", tr->uart_time_rtc);

	printf(trace_grep.flags.match);
}

void trace_defered_print(struct trace_context *tr)
{
	if (tr->defered.tsm) {
		tr->defered.tsm = 0;

		if (tr->config.tsm_tmr_width)
			printf("time\t");

		if (trace_option & TRACE_APPEND_TIME_ELAPSED)
			printf("%.6f\t", tr->tsm_time_rtc_elaps);

		if (trace_option & TRACE_APPEND_TIME_REAL)
			printf("%.6f\t", tr->tsm_time_rtc);

		printf(trace_grep.tsm.match);
	}

	if (tr->defered.uart_status) {
		tr->defered.uart_status = 0;
		trace_uart_status_print(tr);
	}
}

