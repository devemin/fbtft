// SPDX-License-Identifier: GPL-2.0+
/*
 * FB driver for the ILI9341 LCD display controller
 *
 * This display uses 9-bit SPI: Data/Command bit + 8 data bits
 * For platforms that doesn't support 9-bit, the driver is capable
 * of emulating this using 8-bit transfer.
 * This is done by transferring eight 9-bit words in 9 bytes.
 *
 * Copyright (C) 2013 Christian Vogelgsang
 * Based on adafruit22fb.c by Noralf Tronnes
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <video/mipi_display.h>
#include <linux/gpio/consumer.h>

#include "fbtft.h"

#define DRVNAME		"fb_majocairisLCD"
#define WIDTH		640
#define HEIGHT		48
#define TXBUFLEN	(4 * PAGE_SIZE)
#define DEFAULT_GAMMA	"00 1C 21 02 11 07 3D 79 4B 07 0F 0C 1B 1F 0F\n" \
			"00 1C 20 04 0F 04 33 45 42 04 0C 0A 22 29 0F"


static int init_display(struct fbtft_par *par)
{
	par->fbtftops.reset(par);

	//pr_info("majoca init_display\n");

	/* startup sequence for Maji Majo Pures LCD */

	write_reg(par, 0x01); /* software reset */
	mdelay(5);
	write_reg(par, 0x28); /* display off */

	/* --------------------------------------------------------- */
	///write_reg(par, 0x00);		//下記の化けのために試しに 0x00 送っても意味なかった
    // Set EXTC
	write_reg(par, 0xC8, 0xFF, 0x93, 0x42);		//0xC8 が DE に化けるので2回送ってる。。。
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
	xe = 319;
	ye = 95;
	//pr_info("majoca set_addr_win\n");

	write_reg(par, MIPI_DCS_SET_COLUMN_ADDRESS,
		  (xs >> 8) & 0xFF, xs & 0xFF, (xe >> 8) & 0xFF, xe & 0xFF);

	write_reg(par, MIPI_DCS_SET_PAGE_ADDRESS,
		  (ys >> 8) & 0xFF, ys & 0xFF, (ye >> 8) & 0xFF, ye & 0xFF);

	write_reg(par, MIPI_DCS_WRITE_MEMORY_START);
}

#define MEM_Y   BIT(7) /* MY row address order */
#define MEM_X   BIT(6) /* MX column address order */
#define MEM_V   BIT(5) /* MV row / column exchange */
#define MEM_L   BIT(4) /* ML vertical refresh order */
#define MEM_H   BIT(2) /* MH horizontal refresh order */
#define MEM_BGR (3) /* RGB-BGR Order */
static int set_var(struct fbtft_par *par)
{
	switch (par->info->var.rotate) {
	case 0:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  MEM_X | (par->bgr << MEM_BGR));
		break;
	case 270:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  MEM_V | MEM_L | (par->bgr << MEM_BGR));
		break;
	case 180:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  MEM_Y | (par->bgr << MEM_BGR));
		break;
	case 90:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  MEM_Y | MEM_X | MEM_V | (par->bgr << MEM_BGR));
		break;
	}

	return 0;
}



static int fbtft_write_gpio8_wr_majoca(struct fbtft_par* par, void* buf, size_t len)
{
	//pr_info("majoca fbtft_write_gpio8_wr in fbtft-io.c");

	u8 data;
	int i;
#ifndef DO_NOT_OPTIMIZE_FBTFT_WRITE_GPIO
	static u8 prev_data;
#endif

	fbtft_par_dbg_hex(DEBUG_WRITE, par, par->info->device, u8, buf, len,
		"%s(len=%zu): ", __func__, len);

	while (len--) {
		data = *(u8*)buf;

		/* Start writing by pulling down /WR */
		gpiod_set_value(par->gpio.wr, 0);

		/* Set data */
#ifndef DO_NOT_OPTIMIZE_FBTFT_WRITE_GPIO
		if (data == prev_data) {
			gpiod_set_value(par->gpio.wr, 0); /* used as delay */
		}
		else {
			for (i = 0; i < 8; i++) {
				if ((data & 1) != (prev_data & 1))
					gpiod_set_value(par->gpio.db[i],
						data & 1);
				data >>= 1;
				prev_data >>= 1;
			}
		}
#else
		for (i = 0; i < 8; i++) {
			gpiod_set_value(par->gpio.db[i], data & 1);
			data >>= 1;
		}
#endif

		/* Pullup /WR */
		gpiod_set_value(par->gpio.wr, 1);

#ifndef DO_NOT_OPTIMIZE_FBTFT_WRITE_GPIO
		prev_data = *(u8*)buf;
#endif
		buf++;
	}

	return 0;
}




/*
 * Gamma string format:
 *  Positive: Par1 Par2 [...] Par15
 *  Negative: Par1 Par2 [...] Par15
 */
#define CURVE(num, idx)  curves[(num) * par->gamma.num_values + (idx)]
static int set_gamma(struct fbtft_par *par, u32 *curves)
{
	int i;

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
	//.buswidth = 9,
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.txbuflen = TXBUFLEN,
	.gamma_num = 2,
	.gamma_len = 15,
	.gamma = DEFAULT_GAMMA,
	.fbtftops = {
		.write = fbtft_write_gpio8_wr_majoca,
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		//.set_var = set_var,
		//.set_gamma = set_gamma,
	},
};

FBTFT_REGISTER_DRIVER(DRVNAME, "ilitek,majocairisLCD", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:majocairisLCD");
MODULE_ALIAS("platform:majocairisLCD");

MODULE_DESCRIPTION("FB driver for the majocairis LCD display controller");
MODULE_AUTHOR("Christian Vogelgsang");
MODULE_LICENSE("GPL");
