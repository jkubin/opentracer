/* generic header file for PIC18 stub
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

#ifndef __PIC18_STUB_H
#  define __PIC18_STUB_H

#  include "opentracer.h"

void traceinit(void);

void tracepoint
/*
 * nothing in between
 */
#  if TRACE_POINT_WIDTH == 1
 (unsigned char id);
#  elif TRACE_POINT_WIDTH == 2
 (unsigned int id);
#  elif TRACE_POINT_WIDTH == 3
 (unsigned short long id);
#  elif TRACE_POINT_WIDTH == 4
 (unsigned long id);
#  else
#    error undefined tracepoint width!
#  endif



#  ifdef TRACE_TRIVIAL

#    define trace0(s)
#    define trace1(s)
#    define trace10(s)
#    define trace11(s)
#    define trace12(s)
#    define trace13(s)
#    define trace14(s)
#    define trace2(s)
#    define trace3(s)
#    define trace4(s)
#    define trace5(s)
#    define trace6(s)
#    define trace7(s)
#    define trace8(s)
#    define trace9(s)
#    define tracebitf(x)
#    define tracebitf_(a, b);
#    define tracemac(a, b);
#    define tracepgm(x)
#    define tracepgm_(a, b);
#    define traceram(x)
#    define traceram_(a, b);
#    define tracesint(x)
#    define tracesint_(a, b);
#    define tracesm();
#    define tracestr(a);
#    define tracestrpgm(a);
#    define tracestruct_(a, b);
#    define traceuint(x)
#    define traceuint_(a, b);
#    define tracexmem(a, b);
#    define tracexxmem(a, b);
// #define trace15(s)
// #define tracecfg();
// #define tracedflash(a, b);
// #define tracexxd(a, b);

#  else		/* !defined(TRACE_TRIVIAL) */

#    define trace0(s)		tracestruct_(0, &s, sizeof(s))
#    define trace1(s)		tracestruct_(1, &s, sizeof(s))
#    define trace10(s)		tracestruct_(10, &s, sizeof(s))
#    define trace11(s)		tracestruct_(11, &s, sizeof(s))
#    define trace12(s)		tracestruct_(12, &s, sizeof(s))
#    define trace13(s)		tracestruct_(13, &s, sizeof(s))
#    define trace14(s)		tracestruct_(14, &s, sizeof(s))
#    define trace2(s)		tracestruct_(2, &s, sizeof(s))
#    define trace3(s)		tracestruct_(3, &s, sizeof(s))
#    define trace4(s)		tracestruct_(4, &s, sizeof(s))
#    define trace5(s)		tracestruct_(5, &s, sizeof(s))
#    define trace6(s)		tracestruct_(6, &s, sizeof(s))
#    define trace7(s)		tracestruct_(7, &s, sizeof(s))
#    define trace8(s)		tracestruct_(8, &s, sizeof(s))
#    define trace9(s)		tracestruct_(9, &s, sizeof(s))
#    define tracebitf(x)	tracebitf_(&x, sizeof(x))
#    define tracepgm(x)		tracepgm_(&x, sizeof(x))
#    define traceram(x)		traceram_(&x, sizeof(x))
#    define tracesint(x)	tracesint_(&x, sizeof(x))
#    define traceuint(x)		traceuint_(&x, sizeof(x))
// #define trace15(s)   	tracestruct_(15, &s, sizeof(s))        // <--- next level of a 255 structs

// void tracedflash(const rom far void *ptr, unsigned int dl);
// void tracexxd(void *ptr, unsigned int dl);
void tracebitf_(void *ptr, unsigned char dl);
void tracepgm_(const rom far unsigned char *ptr, unsigned int dl);
void traceram_(void *ptr, unsigned int dl);
void tracesint_(void *ptr, unsigned char dl);
void tracestruct_(unsigned char id, void *ptr, unsigned char dl);
void traceuint_(void *ptr, unsigned char dl);
// void tracecfg(void);
void traceinit(void);
void tracemac(unsigned int addr, unsigned int dl);
void tracesm(void);
void tracestr(void *ptr);
void tracestrpgm(const rom far char *ptr);
void tracexmem(unsigned int addr, unsigned int dl);
void tracexxmem(unsigned short long addr, unsigned short long dl);

#  endif	/* TRACE_TRIVIAL */

/* EEPROM/DataFlash/FRAM/SRAM read command */
#  define READ	0b00000011					/* Read Memory Data */

/****
// nejjednodussi mozny konfigurak ktery vubec nepouziva casovac (celkem posle 4B vcetne checksum)
// rtt pole funkci bere jako argument version  a config (z toho si vypocita checksum)
// melo by to umet prijmout usr float data?
// version: spodni 4 bity je minor, horni 4 bity major cislo znamena kompletni prekopavku konfigurace
// hornich 4 bity (16) znamena jine pole obsluznych handleru
// hornich 5 bitu (32) znamena jine pole obsluznych handleru
//
// kazdy handler bude sledovat major cislo, tohle je vsade stejny
// pokud nesedi major, pak konec s chybovou zpravou
// major znamena, ze se pole handleru kompletne zmenilo
// minor znamena lokalni zmenu v handleru
// handler bude schopen odpovidat na vsechna minor cisla
//
// z pocatku to bude
// verzi potrebuji mit delanou nerovnomerne
// podle poctu jednickovych bitu bude rozdeleno major:minor
// z pocatku budou brutalni prebodavky
// minor verze prakticky nemaji smysl
// jak to bude zralejsi a zralejsi, pak bude pribyvat prostor pro minor verze
// horni bity jsou major
*/

struct trace_config_plain_points
{
	unsigned int version;
	unsigned tracepoint_width:2;
};

/*
 * once the tracepoint_tmr_width bits are nonzero,
 * then they pay for this configuration
 */
struct trace_config_time_points
{
	unsigned int version;
	unsigned char type;

	/****
	// jak se vyhnout temto dvema udajum?
	// jsou vubec potreba na synchronizaci?
	// kdyz je vzdalenost casovace delsi pak to stejne skonci chybou
	*/
	unsigned:8;
	unsigned long cpu_frequency;
	unsigned long tmr_divisor;
};

/****
struct trace_target_simple
{
	unsigned int version;
	unsigned char config;

	unsigned checksum_level:2;
	unsigned rts:1;
};

struct trace_target_name_8_char
{
	unsigned int version;
	unsigned char config;

	char name[8];
};

struct trace_target_name_16_char
{
	unsigned int version;
	unsigned char config;

	char name[16];
};

struct trace_config_plain_points
{
	unsigned int version;
	unsigned char config;

	unsigned checksum_level:2;
	// unsigned trace_sync_mark:1;
	unsigned rts:1;
};

// konfigurak nepouziva casovac, avsak umi zpracovat nejaka usr float cisla
struct trace_target_usr_float
{
	unsigned int version;
	unsigned char config;

	unsigned usr_float_A:4;
	unsigned usr_float_B:4;
};

 * konfigurak nepouziva casovac, avsak umi zpracovat nejaka usr float cisla
struct trace_target_usr_float_base_config
{
	unsigned int version;
	unsigned char config;

	unsigned checksum_level:2;
	// unsigned trace_sync_mark:1;
	unsigned rts:1;

	unsigned usr_float_A:4;
	unsigned usr_float_B:4;
};

// s pripojenou inicialni hodnotou casovace,
// potrebuju do predu vedet,
// kolik dat nacist coz rika config_iconfig_id v rtt budu mit pole funkci,
// delku prijatych dat urci makro odecitajici pointery
// 1, 2, 3, 4, 5, 6, 7, 8
// do summary handler vypisuje vyhodnocena data
// rtt pole funkci bere jako argument target a checksum (version + target)
struct trace_target_timer
{
	unsigned int version;
	unsigned char config;

	unsigned checksum_level:2;
	unsigned rts:1;

	unsigned long tmr_divisor;
	unsigned long cpu_frequency;
};

// s hodnotou casovace, ktery je pripojen za konfigurakem, potrebuju do predu vedet, kolik dat to prijme
// 1, 2, 3, 4, 5, 6, 7, 8
// umi pracovat s usr float-y
struct trace_target_usr_float_and_timer
{
	unsigned int version;
	unsigned char config;

	unsigned checksum_level:2;
	unsigned rts:1;

	unsigned long tmr_divisor;
	unsigned long cpu_frequency;

	unsigned usr_float_A:4;
	unsigned usr_float_B:4;
};
*/

#endif	/* __PIC18_STUB_H */
