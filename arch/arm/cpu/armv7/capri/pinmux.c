/*****************************************************************************
*
* Kona generic pinmux
*
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <asm/io.h>
#include <asm/arch/chip_pinmux.h>
#include <asm/arch/chip_pingroup.h>
#include <asm/arch/pinmux.h>
#include <errno.h>
#define __init
#include "common.h"
#include <config.h>

#ifdef CONFIG_PINMUX_DUMP 
#include <command.h>
#define GPIOPREFIX "GPIO_"

extern const struct pin_desc pin_desc_tbl[];
static int num_exposed_gpios;
static int exposed_gpios[256];
#if 0
#include <asm/gpio.h>
#include <asm/arch/brcm_rdb_gpio.h>
#include <asm/arch/brcm_rdb_sysmap.h>
#define GPIO_REG_WIDTH_SHIFT 2
int gpio_get_ctrl(unsigned gpio)
{
	unsigned int value, off;
	off = GPIO2_BASE_ADDR + GPIO_GPCTR0_OFFSET + (gpio << GPIO_REG_WIDTH_SHIFT);	
	value = readl(off);
	return value;
}
#endif

void pinmux_dump(void)
{
	/* Dump pinmux registers and print dts type output  */
	int i;
	void __iomem *base = g_chip_pin_desc.base;
	num_exposed_gpios = 0;
	for (i=0; i<PN_MAX; i++, base += 4)
	{
		union pinmux_reg reg;
		reg.val = readl(base);
		printf("        0x%08x   /* 0x%08x : %s-->%s: hys_en:%d pull_dn:%d pull_up:%d slew:%d input_dis:%d drv_sth:%dmA */\n", 
			reg.val, (int)base, pin_desc_tbl[i].nameStr, pin_desc_tbl[i].altStr[reg.b.sel],
			reg.b.hys_en, reg.b.pull_dn, reg.b.pull_up, reg.b.slew_rate_ctrl, reg.b.input_dis, 2*reg.b.drv_sth + 2);

		/* Find -->GPIO_ pattern to get list of exposed gpios */
		if (!strncmp(pin_desc_tbl[i].altStr[reg.b.sel], GPIOPREFIX, strlen(GPIOPREFIX)))
		{
			char *gpioNumStr = pin_desc_tbl[i].altStr[reg.b.sel];
			gpioNumStr += strlen(GPIOPREFIX);
			exposed_gpios[num_exposed_gpios++] = (int)simple_strtoul(gpioNumStr, NULL, 10);
		}
	}
	printf("\nexposed gpios...\n");
	for (i=0; i<num_exposed_gpios; i++)
	{
		int gpio;
		int val; 
		gpio = exposed_gpios[i];
#if 0
		gpio_request(gpio, NULL);

		/* 
		 * Just set all pins to input as the hardware defaults to this 
		 * - drivers will have changed some to outputs by the time we
		 * run this command. The hardware engineers should decide which 
		 * are better as outputs at some point.
		 */
		val = gpio_get_ctrl(gpio);
#else
		val = 1;
#endif

		printf("            %3u    0x%08x /* %d: %s */\n", 
			gpio,  val, (val & 1), (val & 1) ? "Input" : "Output");
	}
}	

/* Command to dump dts info for pinmux */
static int do_pinmux(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	pinmux_dump();

	return 0;
}

U_BOOT_CMD(
	pinmux, 1, 0, do_pinmux,
	"dump pinmux config\n",
	" - dump pinmux config");
#endif

int __init pinmux_init(void)
{
	pinmux_chip_init();
	pinmux_board_init();

	return 0;
}

/*
  get pin configuration at run time
  caller provides pin ball name
*/
int pinmux_get_pin_config(struct pin_config *config)
{
	int ret = 0;
	void __iomem *base = g_chip_pin_desc.base;
	enum PIN_NAME name;

	if(!config)
		return -EINVAL;
	name = config->name;
	if(!is_ball_valid(name))
		return -EINVAL;

	config->reg.val = readl(base+g_chip_pin_desc.desc_tbl[name].reg_offset);

	/* populate func */
	config->func = g_chip_pin_desc.desc_tbl[name].f_tbl[config->reg.b.sel];

	return ret;
}

/*
  set pin configuration at run time
  caller fills pin_configuration, except sel, which will derived from func in this routine.
*/
int pinmux_set_pin_config(struct pin_config *config)
{
	int ret = 0, i;
	void __iomem *base = g_chip_pin_desc.base;
	enum PIN_NAME name;

	if(!config)
		return -EINVAL;
	name = config->name;
	if(!is_ball_valid(name))
		return -EINVAL;

	/* get the sel bits */
	for (i=0; i<MAX_ALT_FUNC; i++) {
		if (g_chip_pin_desc.desc_tbl[name].f_tbl[i] == config->func) {
			config->reg.b.sel = i;
			break;
		}
	}
	if (i==MAX_ALT_FUNC) {
		printf("%s no matching function for pin %u\n", __func__, config->name);
		return -EINVAL;
	}

	writel(config->reg.val, base + g_chip_pin_desc.desc_tbl[name].reg_offset);

#ifdef CONFIG_PINMUX_VERIFY_WRITE
/* Add PINMUX_VERIFY_WRITE to the boards.cfg file board of interest to enable this code */
	{
		unsigned int rdval = readl(base + g_chip_pin_desc.desc_tbl[name].reg_offset);
		if (rdval != config->reg.val)
		{
			printf("%s: pin %u  offset 0x%x  wrote 0x%x  rdval 0x%x\n", 	
			__func__, name, g_chip_pin_desc.desc_tbl[name].reg_offset,
			config->reg.val, rdval); 

			return -EIO;
		}
	}	
#endif
	return ret;
}

