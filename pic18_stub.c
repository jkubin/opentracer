/* generic PIC18 stub; fully working example with comments
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

#include <p18cxxx.h>
#include "pic18_stub.h"

#ifndef TRACE_TRIVIAL

#define SEND_CHECKSUM_ALWAYS(x)		do {while (!PIR1bits.TX1IF); TXREG1 = ~x;} while (0)
#define SEND_CHECKSUM_IF_REQUIRED(x)	// SEND_CHECKSUM_ALWAYS(x)
#define pgm_trace(p, n)			(TBLPTR = p, _pgm_trace(n))	/* set HW rom pointer for speed */
#define ram_trace(p, n)			(FSR0 = p, _ram_trace(n))	/* set HW ram pointer for speed */

/*
 * DO NOT FORGET TO DISABLE OR CALL WATCHDOG!
 */

/****
 * kdyz se zvedne RTS
 * ulozit konfiguraci casovace
 * kdyz spadne RTS, vratit ji zpatky
 *
 * mozna mit bitovy priznak na testovani checkpointu
 * strict_checkpoint	<--- kdyz bude 1, pak se ocekava, ze bude chodit periodicky (napr. kazdou sekundu)
 */
#if TRACE_TIMER_WIDTH == 0
static const rom struct trace_config_plain_points stub_cfg = {
	TRACE_CONFIG_VERSION,
#  if TRACE_POINT_WIDTH == 1
	TRACE_128_PLAIN_POINTS,			// 1B total width
#  elif TRACE_POINT_WIDTH == 2
	TRACE_32768_PLAIN_POINTS,		// 2B total width
#  elif TRACE_POINT_WIDTH == 3
	TRACE_8388608_PLAIN_POINTS,		// 3B total width
#  elif TRACE_POINT_WIDTH == 4
	TRACE_2147483648_PLAIN_POINTS,		// 4B total width
#  endif
};
#elif TRACE_TIMER_WIDTH == 1
static const rom struct trace_config_time_points stub_cfg = {
	TRACE_CONFIG_VERSION,
#  if TRACE_POINT_WIDTH == 1
	TRACE_128_POINTS_1B_TIMER,		// 2B total width
#  elif TRACE_POINT_WIDTH == 2
	TRACE_32768_POINTS_1B_TIMER,		// 3B total width
#  elif TRACE_POINT_WIDTH == 3
	TRACE_8388608_POINTS_1B_TIMER,		// 4B total width
#  elif TRACE_POINT_WIDTH == 4
	TRACE_2147483648_POINTS_1B_TIMER,	// 5B total width
#  endif
	41666667,		// cpu freq
	256ul * 256ul * 4,	// divisor (value depends on a TMR prescaler settings)
};
#elif TRACE_TIMER_WIDTH == 2
static const rom struct trace_config_time_points stub_cfg = {
	TRACE_CONFIG_VERSION,
#  if TRACE_POINT_WIDTH == 1
	TRACE_128_POINTS_2B_TIMER,		// 3B total width
#  elif TRACE_POINT_WIDTH == 2
	TRACE_32768_POINTS_2B_TIMER,		// 4B total width
#  elif TRACE_POINT_WIDTH == 3
	TRACE_8388608_POINTS_2B_TIMER,		// 5B total width
#  elif TRACE_POINT_WIDTH == 4
	TRACE_2147483648_POINTS_2B_TIMER,	// 6B total width
#  endif
	41666667,		// cpu freq
	256ul * 4,		// divisor (value depends on a TMR prescaler settings)
};
#endif

/****
{
	TRACE_PIC18,
	256ul * 256ul * 4,
	41666667,
	TRACE_CFG_VERSION(TRACE_VERSION),
	TRACE_TIMER_WIDTH,
	TARGET_CHECKPOINT_TIMER_WIDTH,
	TRACE_XSINGLE_FLOAT,
	TRACE_XSINGLE_FLOAT,
	TRACE_CFG_TRACEPOINT_WIDTH(TRACE_POINT_WIDTH),
	TRACE_CFG_CHECKSUM_LEVEL(0),
	TRACE_CFG_RTS(0),
};
*/
static unsigned char tsm_time;

static unsigned char _pgm_trace(unsigned char dl)
{

	unsigned char checksum = 0;
	unsigned char i;

	for (i = 0; i < dl; i++) {

		/*
		 * reads a byte from MCU FLASH to TABLAT register
		 * and increments rom pointer in one instruction
		 * HW shortcut for TABLAT = *ptr++;
		 */
		_asm	tblrdpostinc	_endasm

		while (!PIR1bits.TX1IF);	/* UART barrier */

		checksum += TXREG1 = TABLAT;	/* copy TABLAT ---> UART */
	}

	return checksum;
}

static unsigned char _ram_trace(unsigned char dl)
{

	unsigned char checksum = 0;
	unsigned char i;

	for (i = 0; i < dl; i++) {

		while (!PIR1bits.TX1IF);	/* UART barrier */

		/* HW shortcut for *ptr++ ---> UART */
		checksum += TXREG1 = POSTINC0;
	}

	return checksum;
}

void tracebitf_(void *ptr, unsigned char dl)
{

	unsigned char checksum;

	if (!dl)
		return;

	if (dl > 4)
		return;

	while (!PIR1bits.TX1IF);	/* UART barrier */

	checksum = TXREG1 = TRACE_bitf(dl);	/* opcode for a bitfield (0b101010...) */

	checksum += ram_trace((unsigned)ptr, dl);	/* sends a payload */

	SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */
}

static void int_trace(unsigned char opcode, unsigned char dl)
{

	unsigned char checksum;
	unsigned char length;

	if (!dl)
		return;

	if (dl > 1) {

		if (dl & 0x1)
			return;

		if (dl == 6)
			return;

		if (dl > 8)
			return;
	}

	length = dl;

	length >>= 1;

	if (length == 4)
		length--;

	opcode |= length;	/* length is a part of opcode for int */

	while (!PIR1bits.TX1IF);	/* UART barrier */

	checksum = TXREG1 = opcode;	/* sends the int opcode to UART */

	checksum += _ram_trace(dl);	/* sends a payload */

	SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */
}

void tracesint_(void *ptr, unsigned char dl)
{
	FSR0 = (unsigned)ptr;	/* set HW pointer to RAM */

	int_trace(TRACE_OPCODE_INT, dl);	/* sends opcode and payload */
}

void traceuint_(void *ptr, unsigned char dl)
{
	FSR0 = (unsigned)ptr;	/* set HW pointer to RAM */

	int_trace(TRACE_OPCODE_INT | TRACE_SUBCODE_UINT, dl);	/* sends opcode and payload */
}

/****
union timepoint
{
	unsigned int test;
	struct
	{
		unsigned char lo;
		unsigned char hi;
	};
};

static unsigned int safe_time(void)
{
	union timepoint tmr, diff;

	while (1) {

		tmr.lo = TMR0L;
		tmr.hi = TMR0H;

		diff.lo = TMR0L - tmr.lo;

		if (!STATUSbits.C)
			tmr.hi++;

		diff.hi = TMR0H - tmr.hi;

		if (diff.test == 7)
			return tmr.test;
	}
}
*/

static unsigned char trace_timestamp(void)
{
#if TRACE_TIMER_WIDTH == 0

	TMR0L + 1;	/* dummy read of TMR0L copies TMR0 to shadow register TMR0H */

	return 0;

#elif TRACE_TIMER_WIDTH == 1

	TMR0L + 1;	/* dummy read of TMR0L copies TMR0 to shadow register TMR0H */

	while (!PIR1bits.TX1IF);	/* UART barrier */

	return TXREG1 = TMR0H;	/* sends HI byte of TMR0 */

#elif TRACE_TIMER_WIDTH == 2

	unsigned char checksum;

	while (!PIR1bits.TX1IF);	/* UART barrier */

	checksum = TXREG1 = TMR0L;	/* sends LO byte of TMR0 */

	while (!PIR1bits.TX1IF);	/* UART barrier */

	checksum += TXREG1 = TMR0H;	/* sends HI byte of TMR0 */

	return checksum;

#else
#  error PIC18 has max 2B timer!
#endif
}

/*
 * TSM (Trace Synchronisation Mark)
 * validation data mark for trace stream
 * necessary for time synchronisation
 */
void tracesm(void)
{

	unsigned char checksum;
	unsigned char tmr_msb;

	/****
	 * pokud nebude RTS pin, pak udelat prazdny makro
	 * jeden bit mit na rts
	 *
	 *
	 * uvnitr sledovat RTS_pin
	 * kdyz neni 1, pak otestovat promennou cts
	 * kdyz je 1, pak nastavit 0
	 * opustit funkci
	 *
	 * kdyz je RTS_pin 1
	 * a rts promenna je 0
	 * zavolat traceinit()
	 * nastavit rts na 1
	 * vyskocit ven
	 *
	 * pokud dojde ke shozeni RTS pinu
	 * pak tady mit barieru na dovysilani znaku
	 * pak vynulovat promennou cts
	 *
	// safe_time();
	 *
	 */

	TMR0L + 1;	/* dummy read TMR0L copies TMR0 to shadow register TMR0H */
	tmr_msb = TMR0H & 0x80;

	if (tsm_time != tmr_msb) {

		tsm_time = tmr_msb;

		while (!PIR1bits.TX1IF);	/* UART barrier */

		checksum = TXREG1 = TRACE_tsm;	/* sends opcode for TSM */

		checksum += trace_timestamp();	/* appends an optional timestamp */

		SEND_CHECKSUM_ALWAYS(checksum);	/* checksum is mandatory */
	}
}

static void tracecfg(void)
{

	unsigned char checksum = 0;
	unsigned char i;

	/****
	 * pokud bude rts 0
	 * pak se provede
	 * osetrit inicializaci promenne trace_rts
	 */

	/*
	 * sends start symbol to the trace automaton
	 * and as a side effect starts UART transmission
	 */
	TXREG1 = TRACE_start;

	/*
	 * sends the configuration and stub capabilities
	 * the better the stub the better the tracing ability
	 * (tracer appropriately configures internal automata behaviour)
	 */
	checksum += pgm_trace((unsigned short long)&stub_cfg, sizeof(stub_cfg));

	/* appends (initial) tmr value to configure internal clock in tracer */
	checksum += trace_timestamp();

	SEND_CHECKSUM_ALWAYS(checksum);	/* checksum is mandatory */
}

void tracestr(void *txt)
{

	unsigned char checksum;
	unsigned char dl;

	FSR0 = (unsigned)txt;	/* set HW pointer to RAM */

	while (1) {

		/*
		 * similar to strlen(txt) with the TRACE_txt_max length limit
		 * POSTINC0 is a HW shortcut for *ptr++
		 */
		for (dl = 0; POSTINC0 && (dl < TRACE_txt_max); dl++);

		if (!dl)
			return;

		while (!PIR1bits.TX1IF);	/* UART barrier */

		/* opcode for a text data */
		checksum = TXREG1 = TRACE_txt(dl);

		checksum += ram_trace((unsigned)txt, dl);	/* sends a payload */

		txt = (void *)FSR0;	/* save pointer FSR0 */

		SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

		tracesm();	/* sends TSM if it is necessary */
	}
}

void tracestrpgm(const rom far char *txt)
{

	unsigned char checksum;
	unsigned char dl;

	TBLPTR = (unsigned short long)txt;	/* set HW rom pointer to MCU FLASH */

	while (1) {

		/* similar to strlen(txt) with the TRACE_txt_max length limit */
		for (dl = 0; dl < TRACE_txt_max; dl++) {

			/*
			 * reads a byte from MCU FLASH to TABLAT register
			 * and increments rom pointer in one instruction
			 * HW shortcut for TABLAT = *ptr++;
			 */
			_asm	tblrdpostinc	_endasm

			/* test for NULL */
			if (!TABLAT)
				break;
		}

		if (!dl)
			return;

		while (!PIR1bits.TX1IF);	/* UART barrier */

		/* opcode for a text data */
		checksum = TXREG1 = TRACE_txt(dl);

		checksum += pgm_trace((unsigned short long)txt, dl);	/* sends a payload */

		txt = (const rom far void *)TBLPTR;	/* save HW pointer */

		SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

		tracesm();	/* sends TSM if it is necessary */
	}
}

void traceram_(void *ptr, unsigned int dl)
{

	unsigned char checksum;

	FSR0 = (unsigned)ptr;	/* set HW pointer to RAM */

	// unsigned char rle[0x40];

	/****
	 * rle generator udelat v rtt
	 * naplnit buffer nejakymi daty (ze souboru)
	 * naplnit druhy buffer zkomprimovanymi daty
	 * druhy buffer poslat do traceru

	 * nacist znak do chr
	 * nacist dalsi znak
	 * pokud bude shoda
	 *
	 * inkrementuj pocitadlo
	 *
	 * pokud nebude shoda
	 * chr uloz do rle[0] a do prev
	 * nacti chr
	 * porovnej s prev
	 * pokud delka dosahne 0x40
	 * poslat na linku
	 *
	 * pokud shoda
	 *
	 * 2 - 2049
	 *
	 * 0x08, 0x00, 0x00,	<--- 2 znaky 0x00
	 * 0x08, 0x01, 0x00,	<--- 3 znaky 0x00
	 *
	 * 0x08, 0x10, 0x00,	<--- 18 znaku 0x00
	 *
	 * 0x08, 0xfd, 0x00,	<--- 255 znaku 0x00
	 * 0x08, 0xfe, 0x00,	<--- 256 znaku 0x00
	 * 0x08, 0xff, 0x00,	<--- 257 znaku 0x00
	 * 0x09, 0x00, 0x00,	<--- 258 znaku 0x00
	 *
	 * 0x0f, 0xff, 0x00,	<--- 2049 znaku 0x00
	 *
	 *
	 * prvni navrh posila 64B bloky
	 *
	 * druhy navrh implementuje RLE kompresi
	 */

	for (; dl; dl -= TRACE_data_max) {

		while (!PIR1bits.TX1IF);	/* UART barrier */

		if (dl < TRACE_data_max) {

			/* data opcode with reduced length of a payload */
			checksum = TXREG1 = TRACE_data(dl & 0xff);

			checksum += _ram_trace(dl);	/* sends a payload */

			SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

			return;
		}

		/* data opcode with maximum data payload */
		checksum = TXREG1 = TRACE_OPCODE_DATA | (TRACE_data_max - 1);

		checksum += _ram_trace(TRACE_data_max);	/* sends a payload */

		SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

		tracesm();	/* sends TSM if it is necessary */
	}
}

void tracepgm_(const rom far unsigned char *ptr, unsigned int dl)
{

	unsigned char checksum;

	TBLPTR = (unsigned short long)ptr;	/* set HW rom pointer to MCU FLASH */

	for (; dl; dl -= TRACE_data_max) {

		while (!PIR1bits.TX1IF);	/* UART barrier */

		if (dl < TRACE_data_max) {

			/* data opcode with reduced length of a payload */
			checksum = TXREG1 = TRACE_data(dl & 0xff);

			checksum += _pgm_trace(dl);	/* sends a payload */

			SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

			return;
		}

		/* data opcode with maximum data payload */
		checksum = TXREG1 = TRACE_OPCODE_DATA | (TRACE_data_max - 1);

		checksum += _pgm_trace(TRACE_data_max);	/* sends a payload */

		SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

		tracesm();	/* sends TSM if it is necessary */
	}
}

/****
void tracexxd(void *ptr, unsigned int dl)
{

	unsigned int addr = *((unsigned int *)ptr);

	traceuint(&addr, 2);

	traceram_(ptr, dl);
}
*/

/****
 * udelat target asm
 * ;
 */

void tracestruct_(unsigned char id, void *ptr, unsigned char dl)
{

	/****
	 * kdyz je id vetsi jak 14
	 * pak nastavi prvni bajt na 0x1f
	 * a pripoji dalsi bajt jako zvetseny index
	 *
	 * struct 0
	 * 0x10,
	 *
	 * struct 1
	 * 0x11,
	 *
	 * ...
	 *
	 * struct 14
	 * 0x1e,
	 *
	 * struct 15
	 * 0x1f, 0x00, 
	 *
	 * struct 16
	 * 0x1f, 0x01, 
	 *
	 * struct 17
	 * 0x1f, 0x02, 
	 *
	 * ...
	 *
	 * struct 269
	 * 0x1f, 0xfe, 
	 *
	 * struct 270
	 * 0x1f, 0xff, 
	 */

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* sends opcode for a struct ID (or an array or an union) select */
	TXREG1 = TRACE_ssel(id);

	traceram_(ptr, dl);		/* copy data to UART */
}

void traceinit(void)
{	
	tracecfg();

	/****
	// tracestr(modeline);

	// tracerom(struct0, sizeof(struct0));
	// tracestr(capt0);

	tracerom(struct1, sizeof(struct1));
	tracestr(capt1);

	tracerom(struct2, sizeof(struct2));
	tracestr(capt2);
	*/
}

static unsigned char spi_sending(unsigned char dl)
{

	unsigned char checksum = 0;
	unsigned char i;

	for (i = 0; i < dl; i++) {

		while (!PIR1bits.TX1IF);	/* UART barrier */

		while (!SSP1STATbits.BF);	/* SPI barrier */

		checksum += TXREG1 = SSP1BUF;	/* direct data copy SPI ---> UART */

		SSP1BUF = 0;	/* SPI starts read a next byte */
	}
}

static void eeprom_start_trace(void)
{
	SSP1CON1 = 0x21;	/* configure SPI */
	SSP1CON1bits.SSPEN = 1;	/* enable SPI */

	LATDbits.LATD7 = 0;	/* /CS */

	/* start SPI transmission; sends the READ cmd to external memory */
	SSP1BUF = READ;
}

static void eeprom_trace(unsigned int addr, unsigned short long dl)
{

	unsigned char checksum = 0;

	while (!SSP1STATbits.BF);	/* SPI barrier */
	SSP1BUF = ((unsigned char *)&addr)[1];	/* sends HI byte of address */

	while (!SSP1STATbits.BF);	/* SPI barrier */
	SSP1BUF = ((unsigned char *)&addr)[0];	/* sends LO byte of address */

	while (!SSP1STATbits.BF);	/* SPI barrier */
	SSP1BUF = 0;			/* sends dummy byte for read */

	for (; dl; dl -= TRACE_data_max) {

		while (!PIR1bits.TX1IF);	/* UART barrier */

		if (dl < TRACE_data_max) {

			/* data opcode with reduced length of a payload */
			checksum = TXREG1 = TRACE_data(dl & 0xff);

			checksum += spi_sending(dl);	/* sends a payload */

			SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

			break;
		}

		/* data opcode with maximum data payload */
		checksum = TXREG1 = TRACE_OPCODE_DATA | (TRACE_data_max - 1);

		checksum += spi_sending(TRACE_data_max);	/* sends a payload */

		SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

		tracesm();	/* sends TSM if it is necessary */
	}

	while (!SSP1STATbits.BF);	/* SPI barrier */

	/* stops SPI transmission, clears BF as a side effect of a dummy buffer read */
	SSP1BUF + 1;	/* drop the SPI content to WREG */

	LATDbits.LATD7 = 1;	/* /CS */
	SSP1CON1bits.SSPEN = 0;	/* disable SPI */
}

/*
 * traces a bigger external memory to UART
 * 3B address
 * 3B length
 */
void tracexxmem(unsigned short long addr, unsigned short long dl)
{
	eeprom_start_trace();

	while (!SSP1STATbits.BF);	/* SPI barrier */
	SSP1BUF = ((unsigned char *)&addr)[2];	/* sends MS byte of address */

	eeprom_trace(addr, dl);
}

/*
 * traces an small external memory to UART
 * 2B address
 * 2B length
 */
void tracexmem(unsigned int addr, unsigned int dl)
{
	eeprom_start_trace();

	eeprom_trace(addr, dl);
}

#ifdef __18F97J60
static unsigned char mac_trace(unsigned char dl)
{

	unsigned char checksum = 0;
	unsigned char i;

	for (i = 0; i < dl; i++) {

		while (!PIR1bits.TX1IF);	/* UART barrier */

		checksum += TXREG1 = EDATA;	/* copy EDATA ---> UART */
	}

	return checksum;
}

void tracemac(unsigned int addr, unsigned int dl)
{

	unsigned char checksum;

	ERDPT = addr;

	for (; dl; dl -= TRACE_data_max) {

		while (!PIR1bits.TX1IF);	/* UART barrier */

		if (dl < TRACE_data_max) {

			/* data opcode with reduced length of a payload */
			checksum = TXREG1 = TRACE_data(dl & 0xff);

			checksum += mac_trace(dl);	/* sends a payload */

			SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

			return;
		}

		/* data opcode with maximum data payload */
		checksum = TXREG1 = TRACE_OPCODE_DATA | (TRACE_data_max - 1);

		checksum += mac_trace(TRACE_data_max);	/* sends a payload */

		SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */

		tracesm();	/* sends TSM if it is necessary */
	}
}
#endif

#endif

void tracepoint
/*
 * nothing in between
 */
#ifdef TRACE_TRIVIAL
#  if TRACE_POINT_WIDTH == 1
(unsigned char id)
{
	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* low 8 bits of a tracepoint */
	TXREG1 = id;
}
#  elif TRACE_POINT_WIDTH == 2
(unsigned int id)
{
	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* low 8 bits of a tracepoint */
	TXREG1 = ((unsigned char *)&id)[0];

	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* high 8 bits of a tracepoint */
	TXREG1 = ((unsigned char *)&id)[1];
}
#  elif TRACE_POINT_WIDTH == 3
(unsigned short long id)
{
	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* low 8 bits of a tracepoint */
	TXREG1 = ((unsigned char *)&id)[0];

	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* high 8 bits of a tracepoint */
	TXREG1 = ((unsigned char *)&id)[1];

	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* upper 8 bits of a tracepoint */
	TXREG1 = ((unsigned char *)&id)[2];
}
#  elif TRACE_POINT_WIDTH == 4
(unsigned long id)
{
	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* low 8 bits of a tracepoint */
	TXREG1 = ((unsigned char *)&id)[0];

	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* high 8 bits of a tracepoint */
	TXREG1 = ((unsigned char *)&id)[1];

	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* upper 8 bits of a tracepoint */
	TXREG1 = ((unsigned char *)&id)[2];

	/* UART barrier */
	while (!PIR1bits.TX1IF);

	/* most significant 8 bits of a tracepoint */
	TXREG1 = ((unsigned char *)&id)[3];
}
#  endif
#else	/* !defined(TRACE_TRIVIAL) */
#  if TRACE_POINT_WIDTH == 1
(unsigned char id)
{

	unsigned char checksum;

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* low 7 bits of a tracepoint with trace opcode */
	checksum = TXREG1 = TRACE_pnt(id);

	checksum += trace_timestamp();	/* appends an optional timestamp */

	SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */
}
#  elif TRACE_POINT_WIDTH == 2
(unsigned int id)
{

	unsigned char checksum;

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* high 7 bits of a tracepoint with trace opcode */
	checksum = TXREG1 = TRACE_pnt(((unsigned char *)&id)[1]);

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* low 8 bits of a tracepoint */
	checksum += TXREG1 = ((unsigned char *)&id)[0];

	checksum += trace_timestamp();	/* appends an optional timestamp */

	SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */
}
#  elif TRACE_POINT_WIDTH == 3
(unsigned short long id)
{

	unsigned char checksum;

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* upper 7 bits of a tracepoint with trace opcode */
	checksum = TXREG1 = TRACE_pnt(((unsigned char *)&id)[2]);

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* low 8 bits of a tracepoint */
	checksum += TXREG1 = ((unsigned char *)&id)[0];

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* high 8 bits of a tracepoint */
	checksum += TXREG1 = ((unsigned char *)&id)[1];

	checksum += trace_timestamp();	/* appends an optional timestamp */

	SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */
}
#  elif TRACE_POINT_WIDTH == 4
(unsigned long id)
{

	unsigned char checksum;

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* most significant 7 bits of a tracepoint with trace opcode */
	checksum = TXREG1 = TRACE_pnt(((unsigned char *)&id)[3]);

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* low 8 bits of a tracepoint */
	checksum += TXREG1 = ((unsigned char *)&id)[0];

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* high 8 bits of a tracepoint */
	checksum += TXREG1 = ((unsigned char *)&id)[1];

	while (!PIR1bits.TX1IF);	/* UART barrier */

	/* upper 8 bits of a tracepoint */
	checksum += TXREG1 = ((unsigned char *)&id)[2];

	checksum += trace_timestamp();	/* appends an optional timestamp */

	SEND_CHECKSUM_IF_REQUIRED(checksum);	/* depends on configuration */
}
#  endif
#endif	/* TRACE_TRIVIAL */

