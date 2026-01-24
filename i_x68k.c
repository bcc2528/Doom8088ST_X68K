/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2025 Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      X68000 implementation of i_system.h
 *
 *-----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <doslib.h>

#include <stdarg.h>

#include "doomdef.h"
#include "doomtype.h"
#include "compiler.h"
#include "a_pcfx.h"
#include "d_main.h"
#include "i_system.h"

#include "globdata.h"

#include <iocslib.h>
#include <doslib.h>


void I_InitGraphicsHardwareSpecificCode(void);
void I_ShutdownGraphics(void);


static boolean isGraphicsModeSet = false;
static int ssp;


//**************************************************************************************
//
// Screen code
//

void I_InitGraphics(void)
{
	ssp = B_SUPER(0);
	I_InitGraphicsHardwareSpecificCode();
	isGraphicsModeSet = true;
}


//**************************************************************************************
//
// Keyboard code
//

/* create a 2d translation table to avoid redefining this one
   X68XlatTable[0xf][0x8] = ASCII_Scan_Code */
/* keyboard tab = - */
uint8_t X68XlatTable[16][8] = { \
{ 0,88, 2, 3, 4, 5, 6, 7},\
{ 8, 9,10,11,12,13, 0, 0},\
{89,16,17,18,19,20,21,22},\
{23,24,25,14,26, 0,30,31},\
{32,33,34,35,36,37,38,39},\
{40,27,44,45,46,47,48,49},\
{50,51,52,53, 0,84, 0, 0},\
{91,90, 0,80,81,82,83, 0},\
{ 0, 0, 0, 0, 0, 0, 0, 0},\
{ 0, 0, 0, 0, 0, 0, 0, 0},\
{ 0, 0, 0, 0, 0,86,87,95},\
{ 0, 0, 0, 0, 0, 0, 0, 0},\
{ 0, 0, 0, 0, 0, 0, 0, 0},\
{ 0, 0, 0, 0, 0, 0, 0, 0},\
{94,85,92,93, 0, 0, 0, 0},\
{ 0, 0, 0, 0, 0, 0, 0, 0}};

uint8_t KeyDataTable[] =             /* Unshifted ASCII for scan codes */
{
/*	 0   1   2   3   4   5   6   7 */
	 0,  0, '1','2','3','4','5','6',/*  8 */
	'7','8','9','0','-','^','@', 0, /* 16 */
	'q','w','e','r','t','y','u','i',/* 24 */
	'o','p','[',']', 0,  0, 'a','s',/* 32 */ 
	'd','f','g','h','j','k','l',';',/* 40 */
	':','`', 0,  0, 'z','x','c','v',/* 48 */
	'b','n','m',',','.','/', 0, '*',/* 56 */
	 0, ' ', 0,  0,  0,  0,  0,  0, /* 64 */
	 0,  0,  0,  0,  0,  0,  0,  0, /* 72 */
	 0,  0,  0,  0,  0,  0,  0,  0, /* 80 */
	 KEYD_LEFT,KEYD_UP,KEYD_RIGHT,KEYD_DOWN,KEYD_A,KEYD_B,KEYD_L,KEYD_R,  /* 88 */
	 KEYD_START,KEYD_SELECT,KEYD_MINUS,KEYD_PLUS,KEYD_BRACKET_LEFT,KEYD_BRACKET_RIGHT,KEYD_SPEED,KEYD_STRAFE,  /* 96*/
};

static uint8_t oldkeystate[96];

static boolean isKeyboardIsrSet = false;


void I_InitKeyboard(void)
{
	isKeyboardIsrSet = true;

	memset(oldkeystate, 0, 96);
}


void I_StartTic(void)
{
	uint8_t key;
	uint8_t tempkey[96];	
	uint8_t i;
	d_event_t event;

	memset(tempkey, 0, 96);

	for(i=0;i<16;i++)
	{
		key = BITSNS(i);
		tempkey[X68XlatTable[i][0]] = (key & 1) ? 1 : 0;
		tempkey[X68XlatTable[i][1]] = (key & 2) ? 1 : 0;
		tempkey[X68XlatTable[i][2]] = (key & 4) ? 1 : 0;
		tempkey[X68XlatTable[i][3]] = (key & 8) ? 1 : 0;
		tempkey[X68XlatTable[i][4]] = (key &16) ? 1 : 0;
		tempkey[X68XlatTable[i][5]] = (key &32) ? 1 : 0;
		tempkey[X68XlatTable[i][6]] = (key &64) ? 1 : 0;
		tempkey[X68XlatTable[i][7]] = (key&128) ? 1 : 0;
	}

	for (i=0; i<96; i++)
	{
		if(tempkey[i] != oldkeystate[i])
		{
			event.type = (tempkey[i]) ? ev_keydown : ev_keyup;
			event.data1= KeyDataTable[i];

			D_PostEvent(&event);
		}
	}

	memcpy(oldkeystate,tempkey,96);
}


//**************************************************************************************
//
// Audio
//

static uint16_t	data[146];
static int16_t	PCFX_LengthLeft;
static const uint16_t *PCFX_Sound = NULL;
static uint16_t	PCFX_LastSample = 0;

static void PCFX_Service(void)
{
	if (PCFX_Sound)
	{
		uint16_t value = *PCFX_Sound++;

		if (value != PCFX_LastSample)
		{
			PCFX_LastSample = value;
			OPMSET(0x28, value >> 5); // chA. KC
			OPMSET(0x30,(value << 2) & 0x7c); // chA. KF
		}

		if (--PCFX_LengthLeft == 0)
		{
			OPMSET(0x08, 0x0); // chA. OFF

			PCFX_Sound      = NULL;
			PCFX_LastSample = 0;
		}
	}
}

static void PCFX_Stop(void)
{
	if (PCFX_Sound == NULL)
		return;

	OPMSET(0x08, 0x0); // chA. OFF

	PCFX_Sound      = NULL;
	PCFX_LastSample = 0;

}

typedef struct {
	uint16_t	length;
	uint16_t	data[];
} pcspkmuse_t;

void PCFX_Play(int16_t lumpnum)
{
	PCFX_Stop();

	const pcspkmuse_t *pcspkmuse = W_GetLumpByNum(lumpnum);
	PCFX_LengthLeft = pcspkmuse->length;
	memcpy(data, pcspkmuse->data, pcspkmuse->length * sizeof(uint16_t));
	Z_ChangeTagToCache(pcspkmuse);

	PCFX_Sound = &data[0];

	OPMSET(0x08, 0x78); // chA. ON
}


void PCFX_Init(void)
{
	OPMSET(0x08, 0x0); // chA. OFF
	OPMSET(0x20,0xC0); // chA. pan / FL / CONNECT
	OPMSET(0x28, 0x0); // chA. KC
	OPMSET(0x30, 0x0); // chA. KF
	OPMSET(0x40, 0x2); // chA. OP1 - DeTune1 / MULtiple
	OPMSET(0x48, 0x2); // chA. OP3 - DeTune1 / MULtiple
	OPMSET(0x50, 0x2); // chA. OP2 - DeTune1 / MULtiple
	OPMSET(0x58, 0x1); // chA. OP4 - DeTune1 / MULtiple
	OPMSET(0x60,0x1D); // chA. OP1 - TL
	OPMSET(0x68,0x29); // chA. OP3 - TL
	OPMSET(0x70,0x21); // chA. OP2 - TL
	OPMSET(0x78, 0x0); // chA. OP4 - TL
	OPMSET(0x80,0x1F); // chA. OP1 - Key Scale / Attack Rate
	OPMSET(0x88,0x1F); // chA. OP3 - Key Scale / Attack Rate
	OPMSET(0x90,0x1F); // chA. OP2 - Key Scale / Attack Rate
	OPMSET(0x98,0x1F); // chA. OP4 - Key Scale / Attack Rate
	OPMSET(0xA0, 0x0); // chA. OP1 - AMS-Enable / 1st Decay Rate
	OPMSET(0xA8, 0x0); // chA. OP3 - AMS-Enable / 1st Decay Rate
	OPMSET(0xB0, 0x0); // chA. OP2 - AMS-Enable / 1st Decay Rate
	OPMSET(0xB8, 0x0); // chA. OP4 - AMS-Enable / 1st Decay Rate
	OPMSET(0xC0, 0x0); // chA. OP1 - DeTune2 / 2nd Decay Rate
	OPMSET(0xC8, 0x0); // chA. OP3 - DeTune2 / 2nd Decay Rate
	OPMSET(0xD0, 0x0); // chA. OP2 - DeTune2 / 2nd Decay Rate
	OPMSET(0xD8, 0x0); // chA. OP4 - DeTune2 / 2nd Decay Rate
	OPMSET(0xE0, 0xF); // chA. OP1 - 1st Decay Level / Release Rate
	OPMSET(0xE8, 0xF); // chA. OP3 - 1st Decay Level / Release Rate
	OPMSET(0xF0, 0xF); // chA. OP2 - 1st Decay Level / Release Rate
	OPMSET(0xF8, 0xF); // chA. OP4 - 1st Decay Level / Release Rate
}


void PCFX_Shutdown(void)
{
	PCFX_Stop();
	OPMSET(0x08, 0x0); // chA. OP4 OFF
	OPMSET(0x20, 0x0); // chA. pan / FL / CONNECT
	OPMSET(0x28, 0x0); // chA. KC
	OPMSET(0x30, 0x0); // chA. KF
	OPMSET(0x40, 0x0); // chA. OP1 - DeTune1 / MULtiple
	OPMSET(0x48, 0x0); // chA. OP3 - DeTune1 / MULtiple
	OPMSET(0x50, 0x0); // chA. OP2 - DeTune1 / MULtiple
	OPMSET(0x58, 0x0); // chA. OP4 - DeTune1 / MULtiple
	OPMSET(0x60, 0x0); // chA. OP1 - TL
	OPMSET(0x68, 0x0); // chA. OP3 - TL
	OPMSET(0x70, 0x0); // chA. OP2 - TL
	OPMSET(0x78, 0x0); // chA. OP4 - TL
	OPMSET(0x80, 0x0); // chA. OP1 - Key Scale / Attack Rate
	OPMSET(0x88, 0x0); // chA. OP3 - Key Scale / Attack Rate
	OPMSET(0x90, 0x0); // chA. OP2 - Key Scale / Attack Rate
	OPMSET(0x98, 0x0); // chA. OP4 - Key Scale / Attack Rate
	OPMSET(0xA0, 0x0); // chA. OP1 - AMS-Enable / 1st Decay Rate
	OPMSET(0xA8, 0x0); // chA. OP3 - AMS-Enable / 1st Decay Rate
	OPMSET(0xB0, 0x0); // chA. OP2 - AMS-Enable / 1st Decay Rate
	OPMSET(0xB8, 0x0); // chA. OP4 - AMS-Enable / 1st Decay Rate
	OPMSET(0xC0, 0x0); // chA. OP1 - DeTune2 / 2nd Decay Rate
	OPMSET(0xC8, 0x0); // chA. OP3 - DeTune2 / 2nd Decay Rate
	OPMSET(0xD0, 0x0); // chA. OP2 - DeTune2 / 2nd Decay Rate
	OPMSET(0xD8, 0x0); // chA. OP4 - DeTune2 / 2nd Decay Rate
	OPMSET(0xE0, 0x0); // chA. OP1 - 1st Decay Level / Release Rate
	OPMSET(0xE8, 0x0); // chA. OP3 - 1st Decay Level / Release Rate
	OPMSET(0xF0, 0x0); // chA. OP2 - 1st Decay Level / Release Rate
	OPMSET(0xF8, 0x0); // chA. OP4 - 1st Decay Level / Release Rate
}


//**************************************************************************************
//
// Returns time in 1/35th second tics.
//

#include <time.h>

static volatile int32_t ticcount;

static boolean isTimerSet = false;

__attribute__((interrupt)) static void Timer_D_Function(void)
{
	ticcount++;
	PCFX_Service();
}


int32_t I_GetTime(void)
{
	return ticcount >> 2;
}


void I_InitTimer(void)
{
	// 140hz = about 7412us, but Timer-D on X68000 can't match that period accurately.
	// Mode 7 = 50us, 50 * 143 = 7150us (139hz)
	TIMERDST((byte *)Timer_D_Function, 7, 143);
	isTimerSet = true;
}


static void I_ShutdownTimer(void)
{
	TIMERDST((byte *)Timer_D_Function, 0, 0);
}


//**************************************************************************************
//
// Memory
//

static int ZoneMemory_ptr;
static boolean isZoneMemorySet = false;

uint8_t __far* I_ZoneBase(uint32_t *heapSize)
{
	uint8_t *ptr;
	uint32_t paragraphs;

	for(uint16_t i = 4;i > 0;i--)
	{
		paragraphs = i * 1024 * 1024;
		ptr = (uint8_t *) MALLOC (paragraphs);
		ZoneMemory_ptr = ptr;

		if(0x81000000 == (ZoneMemory_ptr & 0x81000000) || 0x82000000 == (ZoneMemory_ptr & 0x82000000))
		{
			continue;
		}

		isZoneMemorySet = true;
		*heapSize = paragraphs;
		return ptr;
	}

	printf("Doom8088: X68000 Edition is requirement Free Memory 1.4MB\n");
	exit(1);
	return 0;
}


//**************************************************************************************
//
// Exit code
//

static void I_Shutdown(void)
{
	if (isGraphicsModeSet)
	{
		I_ShutdownGraphics();
	}

	I_ShutdownSound();

	if (isTimerSet)
		I_ShutdownTimer();

	if (isKeyboardIsrSet)
	{
	}

	if (isZoneMemorySet)
	{
		MFREE(ZoneMemory_ptr);
	}

	KFLUSHIO(0xff); // Key Buffer Clear
}


void I_Quit(void)
{
	I_Shutdown();

	printf("\n");
	exit(0);
}


void I_Error (const char *error, ...)
{
	va_list argptr;

	I_Shutdown();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}


int main(int argc, const char * const * argv)
{
	printf("Doom8088: X68000 Edition\n");

	D_DoomMain(argc, argv);
	return 0;
}
