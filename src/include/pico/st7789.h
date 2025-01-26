/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef _PICO_ST7789_H_
#define _PICO_ST7789_H_

#include "hardware/spi.h"

struct st7789_config
{
	spi_inst_t* spi;
	uint gpio_din;
	uint gpio_clk;
	int gpio_cs;
	uint gpio_dc;
	uint gpio_rst;
	uint gpio_bl;
};

enum reg
{
	SWRESET = 0x01,
	TEOFF = 0x34,
	TEON = 0x35,
	MADCTL = 0x36,
	COLMOD = 0x3A,
	GCTRL = 0xB7,
	VCOMS = 0xBB,
	LCMCTRL = 0xC0,
	VDVVRHEN = 0xC2,
	VRHS = 0xC3,
	VDVS = 0xC4,
	FRCTRL2 = 0xC6,
	PWCTRL1 = 0xD0,
	PORCTRL = 0xB2,
	GMCTRP1 = 0xE0,
	GMCTRN1 = 0xE1,
	INVOFF = 0x20,
	SLPOUT = 0x11,
	DISPON = 0x29,
	GAMSET = 0x26,
	DISPOFF = 0x28,
	RAMWR = 0x2C,
	INVON = 0x21,
	CASET = 0x2A,
	RASET = 0x2B,
	PWMFRSEL = 0xCC,
    NORON = 0x13
};

void st7789_init(const struct st7789_config* config, uint16_t width, uint16_t height);
void st7789_write(const void* data, size_t len);
void st7789_put(uint16_t pixel);
void st7789_fill(uint16_t pixel);
void st7789_set_cursor(uint16_t x, uint16_t y);
void st7789_vertical_scroll(uint16_t row);

#endif
