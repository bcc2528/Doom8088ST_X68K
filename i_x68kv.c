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
 *      X68000 video code
 *
 *-----------------------------------------------------------------------------*/

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"

#include <iocslib.h>
#include <doslib.h>


#define GVRAMWIDTH				512
#define GVRAMWIDTH_2				256

#define CRTC_BASE (0xE80000)
#define CRTC_R00 (CRTC_BASE)
#define CRTC_R01 (CRTC_BASE+0x2)
#define CRTC_R02 (CRTC_BASE+0x4)
#define CRTC_R03 (CRTC_BASE+0x6)
#define CRTC_R04 (CRTC_BASE+0x8)
#define CRTC_R05 (CRTC_BASE+0xA)
#define CRTC_R06 (CRTC_BASE+0xC)
#define CRTC_R07 (CRTC_BASE+0xE)
#define CRTC_R08 (CRTC_BASE+0x10)
#define CRTC_R09 (CRTC_BASE+0x12)
#define CRTC_R10 (CRTC_BASE+0x14)
#define CRTC_R11 (CRTC_BASE+0x16)
#define CRTC_R12 (CRTC_BASE+0x18)
#define CRTC_R13 (CRTC_BASE+0x1A)
#define CRTC_R14 (CRTC_BASE+0x1C)
#define CRTC_R15 (CRTC_BASE+0x1E)
#define CRTC_R16 (CRTC_BASE+0x20)
#define CRTC_R17 (CRTC_BASE+0x22)
#define CRTC_R18 (CRTC_BASE+0x24)
#define CRTC_R19 (CRTC_BASE+0x26)
#define CRTC_R20 (CRTC_BASE+0x28)
#define CRTC_R21 (CRTC_BASE+0x2A)
#define CRTC_R22 (CRTC_BASE+0x2C)
#define CRTC_R23 (CRTC_BASE+0x2E)
#define CRTC_MEM (CRTC_BASE+0x421)

short *crtc_r00 = (short*)CRTC_R00;	/* htotal*/
short *crtc_r01 = (short*)CRTC_R01;	/* hsync end*/
short *crtc_r02 = (short*)CRTC_R02;	/* hdisp start*/
short *crtc_r03 = (short*)CRTC_R03;	/* hdisp end*/

/* vertical control*/
short *crtc_r04 = (short*)CRTC_R04;	/* vtotal*/
short *crtc_r05 = (short*)CRTC_R05;	/* vsync end*/
short *crtc_r06 = (short*)CRTC_R06;	/* vdisp end*/
short *crtc_r07 = (short*)CRTC_R07;	/* vdisp start*/

/* external hsync adjust/hposition fine tune*/
short *crtc_r08 = (short*)CRTC_R08;	

/* raster numbers: raster interrupt(?)*/
short *crtc_r09 = (short*)CRTC_R09;	

/* text screen scrolling-*/
short *crtc_r10 = (short*)CRTC_R09;	/* x position*/
short *crtc_r11 = (short*)CRTC_R10;	/* y position*/

/* graphic screen scroll*/
short *crtc_r12 = (short*)CRTC_R12;	/* x0*/
short *crtc_r13 = (short*)CRTC_R13;	/* y0*/
short *crtc_r14 = (short*)CRTC_R14;	/* x1*/
short *crtc_r15 = (short*)CRTC_R15;	/* y1*/
short *crtc_r16 = (short*)CRTC_R16;	/* x2*/
short *crtc_r17 = (short*)CRTC_R17;	/* y2*/
short *crtc_r18 = (short*)CRTC_R18;	/* x3*/
short *crtc_r19 = (short*)CRTC_R19;	/* y3*/

/* graphic mode setting*/
volatile short *crtc_r20 = (short*)CRTC_R20;

/* tvram plane simultaneous access enable/tvram raster copy enable/fast clear gvram enable*/
volatile short *crtc_r21 = (short*)CRTC_R21;

/* raster copy source/destination*/
short *crtc_r22 = (short*)CRTC_R22;

/* text screen access mask pattern*/
short *crtc_r23 = (short*)CRTC_R23;

/* image capture/fast clear/raster copy control*/
volatile char *crtc_mem = (char*)CRTC_MEM;


#define VIDCON_BASE	(0xE82000)
#define GRAPHICS_PAL	(VIDCON_BASE)	/* graphics layers*/
#define TEXT_PAL	(VIDCON_BASE+0x200)	/* text, sprite, bg*/
#define VIDCON_R0	(VIDCON_BASE+0x400)	
#define VIDCON_R1	(VIDCON_BASE+0x500)	
#define VIDCON_R2	(VIDCON_BASE+0x600)	

short *vidcon_r0 = (short*)VIDCON_R0;	/* screen mode*/
short *vidcon_r1 = (short*)VIDCON_R1;	/* priority control*/
short *vidcon_r2 = (short*)VIDCON_R2;	/* on/off control / special priority*/

short *gpal = (short*)GRAPHICS_PAL;
short *tpal = (short*)TEXT_PAL;


#define GVRAM_BASE (0xC00000)
#define GVRAM0 (GVRAM_BASE)
#define GVRAM1 (GVRAM_BASE+0x80000)
#define GVRAM2 (GVRAM_BASE+0x100000)
#define GVRAM3 (GVRAM_BASE+0x180000)

short *gvram[2] = {(short*)GVRAM0, (short*)GVRAM1};


#define TVRAM_BASE	(0xE00000)
#define TVRAM_0		(TVRAM_BASE)	
#define TVRAM_1		(TVRAM_BASE+0x20000)
#define TVRAM_2		(TVRAM_BASE+0x40000)
#define TVRAM_3		(TVRAM_BASE+0x60000)

short *tvram0 = (short*)TVRAM_0;
short *tvram1 = (short*)TVRAM_1;
short *tvram2 = (short*)TVRAM_2;
short *tvram3 = (short*)TVRAM_3;

byte oldTVPal[32];


extern const int16_t CENTERY;


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)


static int16_t page;
static short *_d_screen;


static int16_t palettelumpnum;

void I_ReloadPalette(void)
{
	char lumpName[8] = "PLAYPAL";
	if (_g_gamma != 0)
	{
		lumpName[7] = '0' + _g_gamma;
	}

	palettelumpnum = W_GetNumForName(lumpName);
}


static const uint16_t colors[14] =
{
	// normal
	0,

	// red
	 0xc0,
	0x1c0,
	0x280,
	0x380,
	0x440,
	0x540,
	0x600,
	0x740,

	// yellow
	0x10c2,
	0x2984,
	0x4286,
	0x5b48,

	// green
	0x2000
};


static void I_UploadNewPalette(int8_t pal)
{
	uint8_t r, g, b;
	// This is used to replace the current 256 colour cmap with a new one
	// Used by 256 colour PseudoColor modes

	const uint8_t *palette_lump = W_TryGetLumpByNum(palettelumpnum);
	if (palette_lump != NULL)
	{
		const byte *palette = &palette_lump[pal * 256 * 3];
		for (int_fast16_t i = 0; i < 256; i++)
		{
			r = (*palette++) & 0xf8;
			g = (*palette++) & 0xf8;
			b = (*palette++) & 0xf8;
			gpal[i] = (g << 8) | (r << 3) | (b >> 2) + 1;
		}

		Z_ChangeTagToCache(palette_lump);
	}
	else
	{
		gpal[0]  = colors[pal];
	}
}


static int crt_mode;

void I_InitGraphicsHardwareSpecificCode(void)
{
	crt_mode = CRTMOD(-1);

	/* various display timings*/

	// 256 * 256 31KHz
	/*#define VRAMOFFSETX 8
	#define VRAMOFFSETY 48
	*crtc_r00 = 0x2d;
	*crtc_r01 = 0x04;
	*crtc_r02 = 0x06;
	*crtc_r03 = 0x26;
	*crtc_r04 = 0x237;
	*crtc_r05 = 0x05;
	*crtc_r06 = 0x28;
	*crtc_r07 = 0x228;
	*crtc_r08 = 0x1b;*/
	//*crtc_r20 = 0x1110; // 256x256 256 color, 31khz, gvram enabled

	// 384 * 256 31KHz
	/*#define VRAMOFFSETX 72
	#define VRAMOFFSETY 48
	*crtc_r00 = 0x44;
	*crtc_r01 = 0x06;
	*crtc_r02 = 0x0b;
	*crtc_r03 = 0x3b;
	*crtc_r04 = 0x237;
	*crtc_r05 = 0x05;
	*crtc_r06 = 0x28;
	*crtc_r07 = 0x228;
	*crtc_r08 = 0x1b;
	*crtc_r20 = 0x1111;*/ // 512x256 256 color, 31khz, gvram enabled


	// 320 * 208 24(25.42)KHz
	#define VRAMOFFSETX 40
	#define VRAMOFFSETY 24
	*crtc_r00 = 0x38;
	*crtc_r01 = 0x07;
	*crtc_r02 = 0x0a;
	*crtc_r03 = 0x32;
	*crtc_r04 = 0x1d0;
	*crtc_r05 = 0x07;
	*crtc_r06 = 0x20;
	*crtc_r07 = 0x1c0;
	*crtc_r08 = 0x1b;
	*crtc_r20 = 0x1110; // 256x256 256 color, 31khz, gvram enabled

	/* gvram settings
	// 0 = 512x512, 16 color 
	// 4 = 1024x1024, 16
	// 1 = 512x512, 256
	// 2 = dont use
	// 3 = 512x512, 65536(swizzled)*/
	*vidcon_r0 = 0x1;	

	/* set scrolls to 0*/
	*crtc_r10 = 0;
	*crtc_r11 = 0;
	*crtc_r12 = 0;
	*crtc_r13 = 0;
	*crtc_r14 = 0;
	*crtc_r15 = 0;
	*crtc_r16 = 0;
	*crtc_r17 = 0;
	*crtc_r18 = 0;
	*crtc_r19 = 0;

	/* text, graphics, bg/sprite, 
	// graphics layers priority, 0, 1, 2, 3*/
	*vidcon_r1 = 0x24E4;
	/* set Hidden GVRAM layers*/
	*vidcon_r2 = 0;
	
	/* 8x8 31khz BGn*/
	/**cynthia_res = 0x10;*/
	
	C_CUROFF();
	SKEY_MOD(0, 0, 0);
	
	memset(gvram[0], 0x0, 0x100000);	/* clear all gvram*/
	/* set GVRAM1 layers visible*/
	*vidcon_r2 = 0;

	page = 0;
	gvram[0] += (VRAMOFFSETY * GVRAMWIDTH) + VRAMOFFSETX;
	gvram[1] += (VRAMOFFSETY * GVRAMWIDTH) + VRAMOFFSETX;
	_d_screen = gvram[page];
}


void I_ShutdownGraphics(void)
{
	CRTMOD(crt_mode);
	C_CURON();
	SKEY_MOD(-1, 0, 0);
}


static int8_t newpal;

void I_SetPalette(int8_t p)
{
	newpal = p;
}


#define NO_PALETTE_CHANGE 100

static boolean st_needrefresh = false;

void I_FinishUpdate(void)
{
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	if (st_needrefresh)
	{
		uint16_t *s;
		uint16_t *d;

		s = gvram[page];
		d = gvram[1 - page];
		s += ST_HEIGHT * GVRAMWIDTH;
		d += ST_HEIGHT * GVRAMWIDTH;

		for(int16_t y = ST_HEIGHT;y < SCREENHEIGHT; y++)
		{
			for(int16_t x = 0; x < SCREENWIDTH; x++)
			{
				*d++ = *s++;
			}
			s += GVRAMWIDTH - SCREENWIDTH;
			d += GVRAMWIDTH - SCREENWIDTH;
		}
		st_needrefresh = false;
	}

	page = 1 - page;
	_d_screen = gvram[page];
	*vidcon_r2 = 12 - (page * 9);
}


static void I_DrawBuffer(uint16_t *buffer)
{
	uint16_t *src = buffer;
	uint16_t *dst = gvram[1 - page];

	for(int16_t y = 0;y < SCREENHEIGHT; y++)
	{
		for(int16_t x = 0;x < SCREENWIDTH; x++)
		{
			*dst++ = *src++;
		}
		dst += (GVRAMWIDTH - SCREENWIDTH);
	}
}


void I_FinishViewWindow(void)
{
}


union reg
{
	unsigned char b[2];
	unsigned short w;
};

void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	union reg pix;

	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *src = dcvars->source;

	const uint8_t *colmap = dcvars->colormap;

	uint16_t *dest = &_d_screen[(dcvars->x << 1) + dcvars->yl * GVRAMWIDTH];

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	int16_t l = count >> 4;
	while (l--)
	{
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;

		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;

		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;

		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		pix.b[1] = colmap[src[frac >> COLBITS]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
	}

	// Optimization for m68k GCC.
	// Even with the O2 option enabled, the compiler creates the `moveq` instructions
	// before the `lsr` instructions in every `case` statement.
	unsigned int col_shift; //col_shift = COLBITS
	__asm__ volatile(
		"moveq   #9, %0\n": "=r"(col_shift)
    		: "r"(col_shift)
    		: "cc", "memory");

	switch (count & 15)
	{
		case 15: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case 14: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case 13: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case 12: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case 11: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case 10: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case  9: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case  8: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case  7: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case  6: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case  5: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case  4: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case  3: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case  2: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w; dest += GVRAMWIDTH; frac += fracstep;
		case  1: pix.b[1] = colmap[src[frac >> col_shift]]; dest[0] = dest[1] = pix.w;
	}
}


void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	R_DrawColumnSprite(dcvars);
}


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	// Optimize with 32bit move
	uint32_t *dest = (uint32_t *)_d_screen;
	dest += dcvars->x + dcvars->yl * GVRAMWIDTH_2;
	const uint32_t c32 = (color <<16) | color;

	uint16_t l = count >> 4;

	while (l--)
	{
		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;

		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;

		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;

		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;
		*dest = c32; dest += GVRAMWIDTH_2;
	}

	switch (count & 15)
	{
		case 15: dest[GVRAMWIDTH_2 * 14] = c32;
		case 14: dest[GVRAMWIDTH_2 * 13] = c32;
		case 13: dest[GVRAMWIDTH_2 * 12] = c32;
		case 12: dest[GVRAMWIDTH_2 * 11] = c32;
		case 11: dest[GVRAMWIDTH_2 * 10] = c32;
		case 10: dest[GVRAMWIDTH_2 *  9] = c32;
		case  9: dest[GVRAMWIDTH_2 *  8] = c32;
		case  8: dest[GVRAMWIDTH_2 *  7] = c32;
		case  7: dest[GVRAMWIDTH_2 *  6] = c32;
		case  6: dest[GVRAMWIDTH_2 *  5] = c32;
		case  5: dest[GVRAMWIDTH_2 *  4] = c32;
		case  4: dest[GVRAMWIDTH_2 *  3] = c32;
		case  3: dest[GVRAMWIDTH_2 *  2] = c32;
		case  2: dest[GVRAMWIDTH_2 *  1] = c32;
		case  1: dest[                0] = c32;
	}

	/*uint16_t *dest = &_d_screen[(dcvars->x << 1) + dcvars->yl * GVRAMWIDTH];

	uint16_t l = count >> 4;

	while (l--)
	{
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;

		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;

		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;

		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
		dest[0] = dest[1] = color; dest += GVRAMWIDTH;
	}

	switch (count & 15)
	{
		case 15: dest[GVRAMWIDTH * 14] = dest[GVRAMWIDTH * 14 + 1] = color;
		case 14: dest[GVRAMWIDTH * 13] = dest[GVRAMWIDTH * 13 + 1] = color;
		case 13: dest[GVRAMWIDTH * 12] = dest[GVRAMWIDTH * 12 + 1] = color;
		case 12: dest[GVRAMWIDTH * 11] = dest[GVRAMWIDTH * 11 + 1] = color;
		case 11: dest[GVRAMWIDTH * 10] = dest[GVRAMWIDTH * 10 + 1] = color;
		case 10: dest[GVRAMWIDTH *  9] = dest[GVRAMWIDTH *  9 + 1] = color;
		case  9: dest[GVRAMWIDTH *  8] = dest[GVRAMWIDTH *  8 + 1] = color;
		case  8: dest[GVRAMWIDTH *  7] = dest[GVRAMWIDTH *  7 + 1] = color;
		case  7: dest[GVRAMWIDTH *  6] = dest[GVRAMWIDTH *  6 + 1] = color;
		case  6: dest[GVRAMWIDTH *  5] = dest[GVRAMWIDTH *  5 + 1] = color;
		case  5: dest[GVRAMWIDTH *  4] = dest[GVRAMWIDTH *  4 + 1] = color;
		case  4: dest[GVRAMWIDTH *  3] = dest[GVRAMWIDTH *  3 + 1] = color;
		case  3: dest[GVRAMWIDTH *  2] = dest[GVRAMWIDTH *  2 + 1] = color;
		case  2: dest[GVRAMWIDTH *  1] = dest[GVRAMWIDTH *  1 + 1] = color;
		case  1: dest[0] = dest[1] = color;
	}*/
}


#define FUZZOFF GVRAMWIDTH_2 /* SCREENWIDTH / 2 so it fits in an int8_t */
#define FUZZTABLE 50

static const int16_t fuzzoffset[FUZZTABLE] =
{
	FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
	FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
	FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
	FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};

//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//
void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	union reg pix;

	int16_t dc_yl = dcvars->yl;
	int16_t dc_yh = dcvars->yh;

	// Adjust borders. Low...
	if (dc_yl <= 0)
		dc_yl = 1;

	// .. and high.
	if (dc_yh >= VIEWWINDOWHEIGHT - 1)
		dc_yh = VIEWWINDOWHEIGHT - 2;

	int16_t count = (dc_yh - dc_yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *colmap = &fullcolormap[6*256];

	uint16_t *dest = _d_screen + (dc_yl * GVRAMWIDTH) + (dcvars->x * 4 * 60 / VIEWWINDOWWIDTH);

	static int16_t fuzzpos = 0;

	do
	{
		pix.b[1] = colmap[dest[fuzzoffset[fuzzpos] * 2]];
		dest[0] = dest[1] = pix.w;
		dest += GVRAMWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while(--count);
}


void V_ClearViewWindow(void)
{
	uint16_t *dest = _d_screen;
	for (int16_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
	{
		for(int16_t x = 0; x < SCREENWIDTH; x++)
		{
			dest[x] = 0;
		}
		dest += GVRAMWIDTH;
	}
}


void V_InitDrawLine(void)
{
}


void V_ShutdownDrawLine(void)
{
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	union reg pix;

	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);

	int16_t err = dx + dy;

	uint16_t *dst = &_d_screen[y0 * GVRAMWIDTH];
	pix.b[1] = color;

	if(y0 < y1)
	{
		while (true)
		{
			dst[x0] = pix.w;

			if (x0 == x1 && y0 == y1)
				break;

			int16_t e2 = 2 * err;

			if (e2 >= dy)
			{
				err += dy;
				x0  += sx;
			}

			if (e2 <= dx)
			{
				err += dx;
				y0++;
				dst+=GVRAMWIDTH;
			}
		}
	}
	else
	{
		while (true)
		{
			dst[x0] = pix.w;

			if (x0 == x1 && y0 == y1)
				break;

			int16_t e2 = 2 * err;

			if (e2 >= dy)
			{
				err += dy;
				x0  += sx;
			}

			if (e2 <= dx)
			{
				err += dx;
				y0--;
				dst-=GVRAMWIDTH;
			}
		}
	}
}


void V_DrawBackground(int16_t backgroundnum)
{
	union reg pix;

	/* erase the entire screen to a tiled background */
	const byte *src = W_GetLumpByNum(backgroundnum);

	for (int16_t y = 0; y < SCREENHEIGHT; y++)
	{
		for (int16_t x = 0; x < SCREENWIDTH; x += 64)
		{
			uint16_t *d = &_d_screen[y * GVRAMWIDTH + x];
			const byte * s = &src[((y & 63) * 64)];

			size_t len = 64;

			if (SCREENWIDTH - x < 64)
				len = SCREENWIDTH - x;

			while(len--)
			{
				pix.b[1] = *s++;
				*d++ = pix.w;
			}
		}
	}

	Z_ChangeTagToCache(src);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	union reg pix;

	uint16_t *dest = &_d_screen[(offset / SCREENWIDTH) * GVRAMWIDTH];
	const uint8_t *lump = W_TryGetLumpByNum(num);

	if (lump != NULL)
	{
		uint16_t lumpLength = W_LumpLength(num);
		while(lumpLength)
		{
			for(int i = 0; i < SCREENWIDTH; i++)
			{
				pix.b[1] = *lump++;
				*dest++ = pix.w;
			}
			dest += GVRAMWIDTH - SCREENWIDTH;
			lumpLength-=SCREENWIDTH;
		}
		Z_ChangeTagToCache(lump);

	}
}


void ST_Drawer(void)
{
	if (ST_NeedUpdate())
	{
		ST_doRefresh();
		st_needrefresh = true;
	}
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	union reg pix;

	y -= patch->topoffset;
	x -= patch->leftoffset;

	int16_t *desttop = &_d_screen[(y * GVRAMWIDTH) + x];

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++, desttop++)
	{
		const column_t *column = (const column_t *)((const byte *)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte * source = (const byte __far*)column + 3;
			uint16_t *dest = desttop + (column->topdelta * GVRAMWIDTH);

			uint16_t count = column->length;

			switch(count)
			{
				case 7: pix.b[1] = *source++; *dest = pix.w; dest += GVRAMWIDTH;
				case 6: pix.b[1] = *source++; *dest = pix.w; dest += GVRAMWIDTH;
				case 5: pix.b[1] = *source++; *dest = pix.w; dest += GVRAMWIDTH;
				case 4: pix.b[1] = *source++; *dest = pix.w; dest += GVRAMWIDTH;
				case 3: pix.b[1] = *source++; *dest = pix.w; dest += GVRAMWIDTH;
				case 2: pix.b[1] = *source++; *dest = pix.w; dest += GVRAMWIDTH;
				case 1: pix.b[1] = *source++; *dest = pix.w; break;
				default:
				while (count--)
				{
					pix.b[1] = *source++;
					*dest = pix.w;
					dest += GVRAMWIDTH;
				}
			}

			column = (const column_t *)((const byte *)column + column->length + 4);
		}
	}
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	union reg pix;

	static const int32_t   DX  = (((int32_t)SCREENWIDTH)<<FRACBITS) / SCREENWIDTH_VGA;
	static const int16_t   DXI = ((((int32_t)SCREENWIDTH_VGA)<<FRACBITS) / SCREENWIDTH) >> 8;
	static const int32_t   DY  = ((((int32_t)SCREENHEIGHT)<<FRACBITS)+(FRACUNIT-1)) / SCREENHEIGHT_VGA;
	static const int16_t   DYI = ((((int32_t)SCREENHEIGHT_VGA)<<FRACBITS) / SCREENHEIGHT) >> 8;

	y -= patch->topoffset;
	x -= patch->leftoffset;

	const int16_t left   = ( x * DX ) >> FRACBITS;
	const int16_t right  = ((x + patch->width)  * DX) >> FRACBITS;
	const int16_t bottom = ((y + patch->height) * DY) >> FRACBITS;

	uint16_t   col = 0;

	for (int16_t dc_x = left; dc_x < right; dc_x++, col += DXI)
	{
		if (dc_x < 0)
			continue;
		else if (dc_x >= SCREENWIDTH)
			break;

		const column_t *column = (const column_t *)((const byte *)patch + (uint16_t)patch->columnofs[col >> 8]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			int16_t *dest = &_d_screen[(dc_yl * GVRAMWIDTH) + dc_x];

			int16_t frac = 0;

			const byte *source = (const byte *)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				pix.b[1] = source[frac >> 8];
				*dest = pix.w;
				dest += GVRAMWIDTH;
				frac += DYI;
			}

			column = (const column_t *)((const byte *)column + column->length + 4);
		}
	}
}


static uint16_t frontbuffer[SCREENWIDTH * SCREENHEIGHT];
static int16_t *wipe_y_lookup;


static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint32_t *backbuffer = (uint32_t *)_d_screen;

	while (ticks--)
	{
		for (int16_t i = 0; i < SCREENWIDTH / 2; i++)
		{
			if (wipe_y_lookup[i] < 0)
			{
				wipe_y_lookup[i]++;
				done = false;
				continue;
			}

			// scroll down columns, which are still visible
			if (wipe_y_lookup[i] < SCREENHEIGHT)
			{
				/* cph 2001/07/29 -
				 *  The original melt rate was 8 pixels/sec, i.e. 25 frames to melt
				 *  the whole screen, so make the melt rate depend on SCREENHEIGHT
				 *  so it takes no longer in high res
				 */
				int16_t dy = (wipe_y_lookup[i] < 16) ? wipe_y_lookup[i] + 1 : SCREENHEIGHT / 25;
				// At most dy shall be so that the column is shifted by SCREENHEIGHT (i.e. just invisible)
				if (wipe_y_lookup[i] + dy >= SCREENHEIGHT)
					dy = SCREENHEIGHT - wipe_y_lookup[i];

				uint32_t *s = (uint32_t *)&frontbuffer;
				s += i + ((SCREENHEIGHT - dy - 1) * (SCREENWIDTH / 2));
				uint32_t *d = (uint32_t *)&frontbuffer;
				d += i + ((SCREENHEIGHT - 1) * (SCREENWIDTH / 2));

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -(SCREENWIDTH / 2);
					s += -(SCREENWIDTH / 2);
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * GVRAMWIDTH_2;
				d = (uint32_t *)&frontbuffer;
				d += i + wipe_y_lookup[i] * (SCREENWIDTH / 2);

				for (int16_t j = 0 ; j < dy; j++)
				{
					*d = *s;
					d += (SCREENWIDTH / 2);
					s += (GVRAMWIDTH_2);
				}

				wipe_y_lookup[i] += dy;
				done = false;
			}
		}
	}

	I_DrawBuffer(frontbuffer);

	return done;
}


static void wipe_initMelt()
{
	wipe_y_lookup = Z_MallocStatic((SCREENWIDTH / 2) * sizeof(int16_t));

	// setup initial column positions (y<0 => not ready to scroll yet)
	wipe_y_lookup[0] = -(M_Random() % 16);
	for (int8_t i = 1; i < SCREENWIDTH / 2; i++)
	{
		int8_t r = (M_Random() % 3) - 1;

		wipe_y_lookup[i] = wipe_y_lookup[i - 1] + r;

		if (wipe_y_lookup[i] > 0)
			wipe_y_lookup[i] = 0;
		else if (wipe_y_lookup[i] == -16)
			wipe_y_lookup[i] = -15;
	}
}


void wipe_StartScreen(void)
{
	uint16_t *src;
	uint16_t *dst;

	src = _d_screen;
	dst = frontbuffer;
	// copy back buffer to front buffer
	for(int16_t y = 0;y < SCREENHEIGHT; y++)
	{
		for(int16_t x = 0;x < SCREENWIDTH; x++)
		{
			*dst++ = *src++;
		}
		src += GVRAMWIDTH - SCREENWIDTH;
	}
}


void D_Wipe(void)
{
	wipe_initMelt();

	boolean done;
	int32_t wipestart = I_GetTime() - 1;

	do
	{
		int32_t nowtime;
		int16_t tics;
		do
		{
			nowtime = I_GetTime();
			tics = nowtime - wipestart;
		} while (!tics);

		wipestart = nowtime;
		done = wipe_ScreenWipe(1);

		M_Drawer();                   // menu is drawn even on top of wipes

	} while (!done);

	Z_Free(wipe_y_lookup);
}
