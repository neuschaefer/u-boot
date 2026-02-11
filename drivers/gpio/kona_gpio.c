/*
 * Copyright (C) 2011
 * Broadcom Inc
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/brcm_rdb_sysmap.h>
#include <asm/arch/brcm_rdb_gpio.h>


#define KONA_GPIO_BASE_ADDR    GPIO2_BASE_ADDR
#define GPIO_PASSWORD 0xa5a501
#define GPIO_REG_WIDTH_SHIFT 2
#define GPIO_BITS_PER_REG_SHIFT 5
#define GPIO_BITS_PER_REG 32
#define AUSTIN_LED_GPIO 12	/* MJF: see comment in 3rdParty/linux/arch/arm/plat-kona/gpio.c */

int gpio_request(unsigned gpio, const char *label)
{
	unsigned int value, off;

	writel(GPIO_PASSWORD, KONA_GPIO_BASE_ADDR + GPIO_GPPWR_OFFSET);
	off = KONA_GPIO_BASE_ADDR + GPIO_GPPLSR0_OFFSET +((gpio >> GPIO_BITS_PER_REG_SHIFT) << GPIO_REG_WIDTH_SHIFT);
	value = readl(off) & (~(1 << (gpio % GPIO_BITS_PER_REG)));
	writel(value, off);

	return 0;
}

int gpio_free(unsigned gpio)
{
	unsigned int value, off;

	writel(GPIO_PASSWORD, KONA_GPIO_BASE_ADDR + GPIO_GPPWR_OFFSET);
	off = KONA_GPIO_BASE_ADDR + GPIO_GPPLSR0_OFFSET +((gpio >> GPIO_BITS_PER_REG_SHIFT) << GPIO_REG_WIDTH_SHIFT);
	value = readl(off) | (1 << (gpio % GPIO_BITS_PER_REG));
	writel(value, off);

	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	unsigned int value, off;
	off = KONA_GPIO_BASE_ADDR + GPIO_GPCTR0_OFFSET + (gpio << GPIO_REG_WIDTH_SHIFT);	
	value = readl(off);
	writel(value | 0x1, off);

	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	unsigned int off;

	off = KONA_GPIO_BASE_ADDR + GPIO_GPCTR0_OFFSET + (gpio << GPIO_REG_WIDTH_SHIFT);	
	writel(readl(off) & (~0x1), off);

	if (value)
		off = KONA_GPIO_BASE_ADDR + GPIO_GPORS0_OFFSET + ((gpio >>  GPIO_BITS_PER_REG_SHIFT) << GPIO_REG_WIDTH_SHIFT);
	else
		off = KONA_GPIO_BASE_ADDR + GPIO_GPORC0_OFFSET + ((gpio >>  GPIO_BITS_PER_REG_SHIFT) << GPIO_REG_WIDTH_SHIFT);
	writel(1 << (gpio % GPIO_BITS_PER_REG), off);

	return 0;
}

int gpio_get_direction(unsigned gpio)
{
	unsigned int value, off;
	off = KONA_GPIO_BASE_ADDR + GPIO_GPCTR0_OFFSET + (gpio << GPIO_REG_WIDTH_SHIFT);	
	value = readl(off);
	return value & GPIO_GPCTR0_IOTR_MASK;
}

int gpio_get_value(unsigned gpio)
{
	unsigned int off;
	int val;

	if (gpio == AUSTIN_LED_GPIO) {
		return gpio_get_direction(gpio);
	} else {
		off = KONA_GPIO_BASE_ADDR + GPIO_GPIR0_OFFSET + ((gpio >>  GPIO_BITS_PER_REG_SHIFT) << GPIO_REG_WIDTH_SHIFT);
		val = (readl(off) >> (gpio % GPIO_BITS_PER_REG)) & 0x1;
		return val;
	}
}

void gpio_set_value(unsigned gpio, int value)
{
	unsigned int off;

	if (gpio == AUSTIN_LED_GPIO) {
		if (value)
			gpio_direction_input(gpio);
		else
			gpio_direction_output(gpio, 1);

	} else {
	if (value)
		off = KONA_GPIO_BASE_ADDR + GPIO_GPORS0_OFFSET + ((gpio >>  GPIO_BITS_PER_REG_SHIFT) << GPIO_REG_WIDTH_SHIFT);
	else
		off = KONA_GPIO_BASE_ADDR + GPIO_GPORC0_OFFSET + ((gpio >>  GPIO_BITS_PER_REG_SHIFT) << GPIO_REG_WIDTH_SHIFT);

	writel(1 << (gpio % GPIO_BITS_PER_REG), off);
	}
}

