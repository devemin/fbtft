/*
 * FB driver for the ILI9341  LCD display controller
 *
 * This display uses 9-bit SPI: Data/Command bit + 8 data bits
 * For platforms that doesn't support 9-bit, the driver is capable
 * of emulating this using 8-bit transfer.
 * This is done by transfering eight 9-bit words in 9 bytes.
 *
 * Copyright (C) 2013 Christian Vogelgsang
 * Based on adafruit22fb.c by Noralf Tronnes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */



//mod by devemin
//fbtft_devicename : s6d1121.c referd.
//execution code : ili9341 refered.


//This is for Maji Majo Pures LCD !



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>

#include "fbtft.h"

#define DRVNAME		"fb_s6d1121"
#define WIDTH		640
#define HEIGHT		48
#define TXBUFLEN	(2 * PAGE_SIZE)
#define DEFAULT_GAMMA	"00 1C 21 02 11 07 3D 79 4B 07 0F 0C 1B 1F 0F\n" \
			"00 1C 20 04 0F 04 33 45 42 04 0C 0A 22 29 0F"



static int init_display(struct fbtft_par *par)
{
	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	par->fbtftops.reset(par);

	/* startup sequence for Maji Majo Pures LCD */

	//write_reg(par, 0x01); /* software reset */
	//mdelay(5);
	//write_reg(par, 0x28); /* display off */

	/* --------------------------------------------------------- */
    // Set EXTC
	write_reg(par, 0xC8, 0xFF, 0x93, 0x42);
    // Column Address Set
	write_reg(par, 0x2A, 0x00, 0x00, 0x01, 0x3F);
    // Page Address Set
	write_reg(par, 0x2B, 0x00, 0x00, 0x00, 0x5F);
    // Memory Access Control
	write_reg(par, 0x36, 0xC8);
	/* ------------power control-------------------------------- */
    // Power Control 1
	write_reg(par, 0xC0, 0x0E, 0x0E);
    // Power Control 2
	write_reg(par, 0xC1, 0x10);
	/* ------------VCOM --------- */
	write_reg(par, 0xC5, 0xFA);
    // Pixel Format Set
	/* ------------memory access control------------------------ */
	write_reg(par, 0x3A, 0x55); /* 16bit pixel */
	/* ------------frame rate----------------------------------- */
	write_reg(par, 0xB1, 0x00, 0x18);
	/* ------------Gamma---------------------------------------- */
	/* ------------display-------------------------------------- */

	write_reg(par, 0x11); /* sleep out */
	mdelay(130);
	write_reg(par, 0x29); /* display on */
	mdelay(10);

	return 0;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{
	fbtft_par_dbg(DEBUG_SET_ADDR_WIN, par,
		"%s(xs=%d, ys=%d, xe=%d, ye=%d)\n", __func__, xs, ys, xe, ye);

	xe = 319;
	ye = 95;

	/* Column address set */
	write_reg(par, 0x2A,
		(xs >> 8) & 0xFF, xs & 0xFF, (xe >> 8) & 0xFF, xe & 0xFF);

	/* Row adress set */
	write_reg(par, 0x2B,
		(ys >> 8) & 0xFF, ys & 0xFF, (ye >> 8) & 0xFF, ye & 0xFF);

	/* Memory write */
	write_reg(par, 0x2C);
}

#define MEM_Y   (7) /* MY row address order */
#define MEM_X   (6) /* MX column address order */
#define MEM_V   (5) /* MV row / column exchange */
#define MEM_L   (4) /* ML vertical refresh order */
#define MEM_H   (2) /* MH horizontal refresh order */
#define MEM_BGR (3) /* RGB-BGR Order */
static int set_var(struct fbtft_par *par)
{
	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	switch (par->info->var.rotate) {
	case 0:
		write_reg(par, 0x36, (1 << MEM_X) | (par->bgr << MEM_BGR));
		break;
	case 270:
		write_reg(par, 0x36,
			(1<<MEM_V) | (1 << MEM_L) | (par->bgr << MEM_BGR));
		break;
	case 180:
		write_reg(par, 0x36, (1 << MEM_Y) | (par->bgr << MEM_BGR));
		break;
	case 90:
		write_reg(par, 0x36, (1 << MEM_Y) | (1 << MEM_X) |
				     (1 << MEM_V) | (par->bgr << MEM_BGR));
		break;
	}

	return 0;
}

/*
  Gamma string format:
    Positive: Par1 Par2 [...] Par15
    Negative: Par1 Par2 [...] Par15
*/
#define CURVE(num, idx)  curves[num*par->gamma.num_values + idx]
static int set_gamma(struct fbtft_par *par, unsigned long *curves)
{
	int i;

	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	for (i = 0; i < par->gamma.num_curves; i++)
		write_reg(par, 0xE0 + i,
			CURVE(i, 0), CURVE(i, 1), CURVE(i, 2),
			CURVE(i, 3), CURVE(i, 4), CURVE(i, 5),
			CURVE(i, 6), CURVE(i, 7), CURVE(i, 8),
			CURVE(i, 9), CURVE(i, 10), CURVE(i, 11),
			CURVE(i, 12), CURVE(i, 13), CURVE(i, 14));

	return 0;
}
#undef CURVE


static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.txbuflen = TXBUFLEN,
	.gamma_num = 2,
	.gamma_len = 15,
	.gamma = DEFAULT_GAMMA,
	.fbtftops = {
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		//.set_var = set_var,
		//.set_gamma = set_gamma,
	},
};
FBTFT_REGISTER_DRIVER(DRVNAME, "samsung,s6d1121", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:s6d1121");
MODULE_ALIAS("platform:s6d1121");

MODULE_DESCRIPTION("FB driver for the S6D1121 LCD display controller");
MODULE_AUTHOR("Christian Vogelgsang");
MODULE_LICENSE("GPL");
