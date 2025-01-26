/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <string.h>

#include "hardware/gpio.h"
#include "pico/time.h"
#include "pico/st7789.h"

static struct st7789_config st7789_cfg;
static uint16_t st7789_width;
static uint16_t st7789_height;
static bool st7789_data_mode = false;

static void st7789_cmd(uint8_t cmd, const uint8_t* data, size_t len)
{
	if (st7789_cfg.gpio_cs > -1)
	{
		spi_set_format(st7789_cfg.spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
	}
	else
	{
		spi_set_format(st7789_cfg.spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
	}
	st7789_data_mode = false;

	sleep_us(1);
	if (st7789_cfg.gpio_cs > -1)
	{
		gpio_put(st7789_cfg.gpio_cs, 0);
	}
	gpio_put(st7789_cfg.gpio_dc, 0); //command mode
	sleep_us(1);

	spi_write_blocking(st7789_cfg.spi, &cmd, sizeof(cmd));

	if (len)
	{
		sleep_us(1);
		gpio_put(st7789_cfg.gpio_dc, 1); //data mode
		sleep_us(1);

		spi_write_blocking(st7789_cfg.spi, data, len);
	}

	sleep_us(1);
	if (st7789_cfg.gpio_cs > -1)
	{
		gpio_put(st7789_cfg.gpio_cs, 1);
	}
	gpio_put(st7789_cfg.gpio_dc, 1);
	sleep_us(1);
}

void st7789_caset(uint16_t xs, uint16_t xe)
{
	uint8_t data[] = {
		xs >> 8,
		xs & 0xff,
		xe >> 8,
		xe & 0xff,
	};

	// CASET (2Ah): Column Address Set
	st7789_cmd(CASET, data, sizeof(data));
}

void st7789_raset(uint16_t ys, uint16_t ye)
{
	uint8_t data[] = {
		ys >> 8,
		ys & 0xff,
		ye >> 8,
		ye & 0xff,
	};

	// RASET (2Bh): Row Address Set
	st7789_cmd(RASET, data, sizeof(data));
}

void st7789_init(const struct st7789_config* config, uint16_t width, uint16_t height)
{
	memcpy(&st7789_cfg, config, sizeof(st7789_cfg));
	st7789_width = width;
	st7789_height = height;

	spi_init(st7789_cfg.spi, 62500000UL);
	if (st7789_cfg.gpio_cs > -1)
	{
		spi_set_format(st7789_cfg.spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
	}
	else
	{
		spi_set_format(st7789_cfg.spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
	}

	gpio_set_function(st7789_cfg.gpio_cs, GPIO_FUNC_SIO);
	gpio_set_function(st7789_cfg.gpio_dc, GPIO_FUNC_SIO);
	gpio_set_function(st7789_cfg.gpio_din, GPIO_FUNC_SPI);
	gpio_set_function(st7789_cfg.gpio_clk, GPIO_FUNC_SPI);

	if (st7789_cfg.gpio_cs > -1)
	{
		gpio_init(st7789_cfg.gpio_cs);
	}
	gpio_init(st7789_cfg.gpio_dc);

#if (ST7789_NO_RST)
	gpio_init(st7789_cfg.gpio_rst);
#endif
	gpio_init(st7789_cfg.gpio_bl);

	if (st7789_cfg.gpio_cs > -1)
	{
		gpio_set_dir(st7789_cfg.gpio_cs, GPIO_OUT);
	}
	gpio_set_dir(st7789_cfg.gpio_dc, GPIO_OUT);
#if (ST7789_NO_RST)
	gpio_set_dir(st7789_cfg.gpio_rst, GPIO_OUT);
#endif
	gpio_set_dir(st7789_cfg.gpio_bl, GPIO_OUT);

	if (st7789_cfg.gpio_cs > -1)
	{
		gpio_put(st7789_cfg.gpio_cs, 1);
	}
	gpio_put(st7789_cfg.gpio_dc, 1);
#if (ST7789_NO_RST)
	gpio_put(st7789_cfg.gpio_rst, 1);
#endif
	sleep_ms(100);

	// SWRESET (01h): Software Reset
	st7789_cmd(SWRESET, NULL, 0);
	sleep_ms(150);

	// TEON: enable frame sync if used
	st7789_cmd(TEON, NULL, 0);

	// SLPOUT (11h): Sleep Out
	//st7789_cmd(SLPOUT, NULL, 0);
	//sleep_ms(50);

	// COLMOD (3Ah): Interface Pixel Format
	// - RGB interface color format     = 65K of RGB interface
	// - Control interface color format = 16bit/pixel
	st7789_cmd(COLMOD, (uint8_t[]){ 0x05 }, 1);
	//sleep_ms(10);

	// MADCTL (36h): Memory Data Access Control
	// - Page Address Order            = Top to Bottom
	// - Column Address Order          = Left to Right
	// - Page/Column Order             = Normal Mode
	// - Line Address Order            = LCD Refresh Top to Bottom
	// - RGB/BGR Order                 = RGB
	// - Display Data Latch Data Order = LCD Refresh Left to Right
	//st7789_cmd(MADCTL, (uint8_t[]){ 0x00 }, 1);

	st7789_cmd(PORCTRL, (uint8_t[]){ 0x0c, 0x0c, 0x00, 0x33, 0x33 }, 5);
	st7789_cmd(GCTRL, (uint8_t[]){ 0x35 }, 1);
	st7789_cmd(VCOMS, (uint8_t[]){ 0x1f }, 1);
	st7789_cmd(LCMCTRL, (uint8_t[]){ 0x2c }, 1);
	st7789_cmd(VDVVRHEN, (uint8_t[]){ 0x01 }, 1);
	st7789_cmd(VRHS, (uint8_t[]){ 0x12 }, 1);
	st7789_cmd(VDVS, (uint8_t[]){ 0x20 }, 1);
	st7789_cmd(FRCTRL2, (uint8_t[]){ 0x0f }, 1);
	st7789_cmd(PWCTRL1, (uint8_t[]){ 0xa4, 0xa1 }, 2);
	st7789_cmd(GMCTRP1, (uint8_t[]){ 0xD0, 0x08, 0x11, 0x08, 0x0C, 0x15, 0x39, 0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x2D }, 14);
	st7789_cmd(GMCTRN1, (uint8_t[]){ 0xD0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39, 0x44, 0x51, 0x0B, 0x16, 0x14, 0x2F, 0x31 }, 14);


	// INVON (21h): Display Inversion On
	st7789_cmd(INVON, NULL, 0);
	st7789_cmd(SLPOUT, NULL, 0);
	// NORON (13h): Normal Display Mode On
	st7789_cmd(NORON, NULL, 0);

	// DISPON (29h): Display On
	st7789_cmd(DISPON, NULL, 0);
	sleep_ms(100);

	st7789_caset(0, width);
	st7789_raset(0, height);

	st7789_cmd(MADCTL, (uint8_t[]){ 0x70 }, 1);

	gpio_put(st7789_cfg.gpio_bl, 1);
}

void st7789_ramwr()
{
	sleep_us(1);
	if (st7789_cfg.gpio_cs > -1)
	{
		gpio_put(st7789_cfg.gpio_cs, 0);
	}
	gpio_put(st7789_cfg.gpio_dc, 0);
	sleep_us(1);

	// RAMWR (2Ch): Memory Write
	uint8_t cmd = 0x2c;
	spi_write_blocking(st7789_cfg.spi, &cmd, sizeof(cmd));

	sleep_us(1);
	if (st7789_cfg.gpio_cs > -1)
	{
		gpio_put(st7789_cfg.gpio_cs, 0);
	}
	gpio_put(st7789_cfg.gpio_dc, 1);
	sleep_us(1);
}

void st7789_write(const void* data, size_t len)
{
	if (!st7789_data_mode)
	{
		st7789_ramwr();

		if (st7789_cfg.gpio_cs > -1)
		{
			spi_set_format(st7789_cfg.spi, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
		}
		else
		{
			spi_set_format(st7789_cfg.spi, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
		}

		st7789_data_mode = true;
	}

	spi_write16_blocking(st7789_cfg.spi, data, len / 2);
}

void st7789_put(uint16_t pixel)
{
	st7789_write(&pixel, sizeof(pixel));
}

void st7789_fill(uint16_t pixel)
{
	int num_pixels = st7789_width * st7789_height;

	st7789_set_cursor(0, 0);

	for (int i = 0; i < num_pixels; i++)
	{
		st7789_put(pixel);
	}
}

void st7789_set_cursor(uint16_t x, uint16_t y)
{
	st7789_caset(x, st7789_width);
	st7789_raset(y, st7789_height);
}

void st7789_vertical_scroll(uint16_t row)
{
	uint8_t data[] = { (row >> 8) & 0xff, row & 0x00ff };

	// VSCSAD (37h): Vertical Scroll Start Address of RAM
	st7789_cmd(0x37, data, sizeof(data));
}
