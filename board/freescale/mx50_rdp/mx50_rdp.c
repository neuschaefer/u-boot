/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2009-2010 Freescale Semiconductor, Inc.
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
#include <asm/arch/mx50.h>
#include <asm/arch/mx50_pins.h>
#include <asm/arch/iomux.h>
#include <asm/errno.h>

#ifdef CONFIG_IMX_CSPI
#include <imx_spi.h>
#include <asm/arch/imx_spi_pmic.h>
#endif

#if CONFIG_I2C_MXC
#include <i2c.h>
#endif

#ifdef CONFIG_CMD_MMC
#include <mmc.h>
#include <fsl_esdhc.h>
#endif

#ifdef CONFIG_ARCH_MMU
#include <asm/mmu.h>
#include <asm/arch/mmu.h>
#endif

#ifdef CONFIG_CMD_CLOCK
#include <asm/clock.h>
#endif

#ifdef CONFIG_MXC_EPDC
#include <lcd.h>
/* 2011/04/12 FY11 : Supported panel init flag. */
#include <malloc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* 2011/05/12 FY11 : Supported TPS65181. */
#define TPS65181_I2C_ADDR	0x48
#define TPS65185_I2C_ADDR	0x68
#define TPS6518X_REVID_MASK	0x0F
#define TPS65181_REVID		0x02
#define TPS65185_REVID		0x05
#define TPS65185_PORT_MASK	0x00200000

#define TPS65181_VNADJUST_REG	0x03
#define TPS65181_VCOM_REG	0x04
#define TPS65181_PWRSEQ0_REG	0x09

#define TPS65181_VNADJUST_VCOM	0x80

/* 2011/04/05 FY11 : Supported to display boot image. */
#define TPS65185_I2C_RETRY	3

#define TPS65185_UPSEQ0_REG	0x09
#define TPS65185_TMST1_REG	0x0D

#define TPS6518X_TMSTVAL_REG	0x00
#define TPS6518X_ENABLE_REG	0x01
/* 2011/05/25 FY11 : Changed the flagh to confirm completion of reading temperature. */
#define TPS6518X_INT_STATUS2	0x08

#define TPS6518X_ENABLE_V3P3	0x20
#define TPS6518X_ENABLE_ACTIVE	0x80
#define TPS6518X_ENABLE_STANDBY	0x40

/* 2011/05/25 FY11 : Changed the flagh to confirm completion of reading temperature. */
#define TPS6518X_INT_STATUS2_EOC	0x01

#define READ_THERM	0x80
#define CONV_END	0x20
#define MINUS_TEMPERATURE	128
#define MINUS_TEMP_ADJUST_VAL	256

#define TRC_ADDR	0x26
#define WF_HEADER_SIZE	0x30

/* 2011/06/06 FY11 : Modified to share static memory for EPD with kernel. */
#define WF_VER_SIZE	10
#define WF_VER_OFFSET	0x0C


/* 2011/04/12 FY11 : Supported panel init flag. */
struct epd_settings_in_emmc {
	unsigned long wfsize;
	unsigned long vcom;
	unsigned long flag;
};

/* 2011/05/12 FY11 : Supported TPS65181. */
struct epd_settings {
	unsigned char pmic_version;
	unsigned char pmic_i2c_addr;
	struct epd_settings_in_emmc settings;
};
struct epd_settings settings_for_epd;


static u32 system_rev;
static enum boot_device boot_dev;

#if 1 /* E_BOOK */
void setup_gpio(u32 gpio_base_addr, u32 bit_position, u32 cfg);
void set_gpio(u32 gpio_base_addr, u32 bit_position, u32 state);
#endif /* E_BOOK */


/* 2011/06/27 FY11 : Modified memcpy for copying waveform. */
void local_memcpy(void* pdst, const void* psrc, __kernel_size_t size)
{
	unsigned long ulCount = 0;
	int mod_dst, mod_src;
	unsigned char *pucdst = pdst;
	unsigned char *pucsrc = psrc;
	
//	printf("memcpy dst=0x%08lX, src=0x%08lX, size=0x%08X\n", (unsigned long)pdst, (unsigned long)psrc, size );
	if ( size < sizeof(unsigned long) )
	{
		memcpy( pdst, psrc, size );
		return ;	
	}

	mod_dst = sizeof(unsigned long) - ((unsigned long)pdst % (sizeof(unsigned long)));
	mod_src = sizeof(unsigned long) - ((unsigned long)psrc % (sizeof(unsigned long)));

	if ( mod_dst == mod_src )
	{
		memcpy( pdst, psrc, mod_dst );

		pucdst+=mod_dst;
		pucsrc+=mod_src;
		ulCount = (size - mod_dst) / sizeof(unsigned long);
		while ( ulCount )
		{
			*(unsigned long*)(pucdst) = *(unsigned long*)(pucsrc);
			pucsrc += sizeof(unsigned long);
			pucdst += sizeof(unsigned long);
			ulCount--;
		}
		ulCount = (size - mod_dst) % sizeof(unsigned long);
		if ( ulCount )
		{
			memcpy( pucdst, pucsrc, ulCount );
		}
	}
	else
	{
		unsigned long tmp1, tmp2;
		unsigned long tmpmask[4] = { 0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000 };
		int leftshiftbyte, rightshiftbyte, leftshiftbit, rightshiftbit, maskindex, i = 0;
		memcpy( pdst, psrc, mod_dst );
		pucdst+=mod_dst;
		pucsrc+=mod_dst;
		ulCount = (size - mod_dst) / sizeof(unsigned long);

		rightshiftbyte = (unsigned long)pucsrc % sizeof(unsigned long);
		leftshiftbyte = sizeof(unsigned long) - rightshiftbyte;
		maskindex = rightshiftbyte;
		rightshiftbit = rightshiftbyte*8;
		leftshiftbit = leftshiftbyte*8;

		tmp1 = *(unsigned long*)(pucsrc - rightshiftbyte);
		while ( ulCount )
		{
			tmp2 = *(unsigned long*)(pucsrc + leftshiftbyte);
			*(unsigned long*)pucdst = ( ((tmp1 & tmpmask[maskindex]) >> rightshiftbit) | ((tmp2 & ~(tmpmask[maskindex])) << leftshiftbit) );
			tmp1 = tmp2;
			pucsrc += sizeof(unsigned long);
			pucdst += sizeof(unsigned long);
			ulCount--;
		}

		ulCount = (size - mod_dst) % sizeof(unsigned long);
		if ( ulCount )
		{
			memcpy( pucdst, pucsrc, ulCount );
		}
	}
}


#if 1 /* E_BOOK *//* flash LED 2011/02/19 */
void startup_led(void)
{
  unsigned int ccm_gr2;
  /* set iomux to PWM2 */
  mxc_request_iomux(MX50_PIN_PWM2, IOMUX_CONFIG_ALT0);
  mxc_iomux_set_pad(MX50_PIN_PWM2, 0);
  
#if 0 /* delete clock enable. already clock on */
  /* clock enable PWM2 */
  //ccm_gr2 = readl(CCM_BASE_ADDR+0x70);
  //printf("led:ccm gr2 (%x)=%x\n",CCM_BASE_ADDR+0x70,ccm_gr2);
  //writel(ccm_gr2 | 0x0003C000 , CCM_BASE_ADDR+0x70);
#endif
  writel(1000,PWM2_BASE_ADDR+0x0c);/* PWMSAR  */
  writel(28000,PWM2_BASE_ADDR+0x10);/* PWMSPR  */
  writel(0x03c30011,PWM2_BASE_ADDR+0x00);/* PWMMCR *//* use 32K clock */
}
#endif
static inline void setup_boot_device(void)
{
	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	uint bt_mem_ctl = (soc_sbmr & 0x000000FF) >> 4 ;
	uint bt_mem_type = (soc_sbmr & 0x00000008) >> 3;

	switch (bt_mem_ctl) {
	case 0x0:
		if (bt_mem_type)
			boot_dev = ONE_NAND_BOOT;
		else
			boot_dev = WEIM_NOR_BOOT;
		break;
	case 0x2:
		if (bt_mem_type)
			boot_dev = SATA_BOOT;
		else
			boot_dev = PATA_BOOT;
		break;
	case 0x3:
		if (bt_mem_type)
			boot_dev = SPI_NOR_BOOT;
		else
			boot_dev = I2C_BOOT;
		break;
	case 0x4:
	case 0x5:
		boot_dev = SD_BOOT;
		break;
	case 0x6:
	case 0x7:
		boot_dev = MMC_BOOT;
		break;
	case 0x8 ... 0xf:
		boot_dev = NAND_BOOT;
		break;
	default:
		boot_dev = UNKNOWN_BOOT;
		break;
	}
}

enum boot_device get_boot_device(void)
{
	return boot_dev;
}

u32 get_board_rev(void)
{
	return system_rev;
}

static inline void setup_soc_rev(void)
{
	int reg = __REG(ROM_SI_REV);

	switch (reg) {
	case 0x10:
		system_rev = 0x50000 | CHIP_REV_1_0;
		break;
	case 0x11:
		system_rev = 0x50000 | CHIP_REV_1_1_1;
		break;
	default:
		system_rev = 0x50000 | CHIP_REV_1_1_1;
	}
}

static inline void setup_board_rev(int rev)
{
	system_rev |= (rev & 0xF) << 8;
}

inline int is_soc_rev(int rev)
{
	return (system_rev & 0xFF) - rev;
}

#ifdef CONFIG_ARCH_MMU
void board_mmu_init(void)
{
	unsigned long ttb_base = PHYS_SDRAM_1 + 0x4000;
	unsigned long i;

	/*
	* Set the TTB register
	*/
	asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

	/*
	* Set the Domain Access Control Register
	*/
	i = ARM_ACCESS_DACR_DEFAULT;
	asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

	/*
	* First clear all TT entries - ie Set them to Faulting
	*/
	memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);
	/* Actual   Virtual  Size   Attributes          Function */
	/* Base     Base     MB     cached? buffered?  access permissions */
	/* xxx00000 xxx00000 */
	X_ARM_MMU_SECTION(0x000, 0x000, 0x10,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* ROM, 16M */
	X_ARM_MMU_SECTION(0x070, 0x070, 0x010,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* IRAM */
	X_ARM_MMU_SECTION(0x100, 0x100, 0x040,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* SATA */
	X_ARM_MMU_SECTION(0x180, 0x180, 0x100,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* IPUv3M */
	X_ARM_MMU_SECTION(0x200, 0x200, 0x200,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* GPU */
	X_ARM_MMU_SECTION(0x400, 0x400, 0x300,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* periperals */
	X_ARM_MMU_SECTION(0x700, 0x700, 0x400,
			ARM_CACHEABLE, ARM_BUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* CSD0 1G */
	X_ARM_MMU_SECTION(0x700, 0xB00, 0x400,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* CSD0 1G */
	X_ARM_MMU_SECTION(0xF00, 0xF00, 0x100,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* CS1 EIM control*/
	X_ARM_MMU_SECTION(0xF80, 0xF80, 0x001,
			ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
			ARM_ACCESS_PERM_RW_RW); /* iRam */

	/* Workaround for arm errata #709718 */
	/* Setup PRRR so device is always mapped to non-shared */
	asm volatile ("mrc p15, 0, %0, c10, c2, 0" : "=r"(i) : /*:*/);
	i &= (~(3 << 0x10));
	asm volatile ("mcr p15, 0, %0, c10, c2, 0" : : "r"(i) /*:*/);

	/* Enable MMU */
	MMU_ON();
}
#endif

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	return 0;
}

static void setup_uart(void)
{
	/* UART5 RXD */
	mxc_request_iomux(MX50_PIN_ECSPI2_SS0, IOMUX_CONFIG_ALT4);
	mxc_iomux_set_pad(MX50_PIN_ECSPI2_SS0, 0x1E4);
	mxc_iomux_set_input(MUX_IN_UART5_IPP_UART_RXD_MUX_SELECT_INPUT, 0x5);

	/* UART5 TXD */
	mxc_request_iomux(MX50_PIN_ECSPI2_MISO, IOMUX_CONFIG_ALT4);
	mxc_iomux_set_pad(MX50_PIN_ECSPI2_MISO, 0x1E4);
}

#ifdef CONFIG_I2C_MXC
static void setup_i2c(unsigned int module_base)
{
	switch (module_base) {
	case I2C1_BASE_ADDR:
		/* i2c1 SDA */
		mxc_request_iomux(MX50_PIN_I2C1_SDA,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C1_SDA, PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);
		/* i2c1 SCL */
		mxc_request_iomux(MX50_PIN_I2C1_SCL,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C1_SCL, PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);
		break;
	case I2C2_BASE_ADDR:
		/* i2c2 SDA */
		mxc_request_iomux(MX50_PIN_I2C2_SDA,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C2_SDA,
				PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_LOW | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);

		/* i2c2 SCL */
		mxc_request_iomux(MX50_PIN_I2C2_SCL,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C2_SCL,
				PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_LOW | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);
		break;
	case I2C3_BASE_ADDR:
		/* i2c3 SDA */
		mxc_request_iomux(MX50_PIN_I2C3_SDA,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C3_SDA,
				PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);

		/* i2c3 SCL */
		mxc_request_iomux(MX50_PIN_I2C3_SCL,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
		mxc_iomux_set_pad(MX50_PIN_I2C3_SCL,
				PAD_CTL_SRE_FAST |
				PAD_CTL_ODE_OPENDRAIN_ENABLE |
				PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU |
				PAD_CTL_HYS_ENABLE);
		break;
	default:
		printf("Invalid I2C base: 0x%x\n", module_base);
		break;
	}
}

#endif

#ifdef CONFIG_IMX_CSPI
s32 spi_get_cfg(struct imx_spi_dev_t *dev)
{
	switch (dev->slave.cs) {
	case 0:
		/* PMIC */
		dev->base = CSPI3_BASE_ADDR;
		dev->freq = 25000000;
		dev->ss_pol = IMX_SPI_ACTIVE_HIGH;
		dev->ss = 0;
		dev->fifo_sz = 32;
		dev->us_delay = 0;
		break;
	case 1:
		/* SPI-NOR */
		dev->base = CSPI3_BASE_ADDR;
		dev->freq = 25000000;
		dev->ss_pol = IMX_SPI_ACTIVE_LOW;
		dev->ss = 1;
		dev->fifo_sz = 32;
		dev->us_delay = 0;
		break;
	default:
		printf("Invalid Bus ID!\n");
	}

	return 0;
}

void spi_io_init(struct imx_spi_dev_t *dev)
{
	switch (dev->base) {
	case CSPI3_BASE_ADDR:
		mxc_request_iomux(MX50_PIN_CSPI_MOSI, IOMUX_CONFIG_ALT0);
		mxc_iomux_set_pad(MX50_PIN_CSPI_MOSI, 0x4);

		mxc_request_iomux(MX50_PIN_CSPI_MISO, IOMUX_CONFIG_ALT0);
		mxc_iomux_set_pad(MX50_PIN_CSPI_MISO, 0x4);

#if 1 /* E_BOOK *//* for DEBUG 2011/06/01 */
		printf("spi_io_init:ss=%d\n",dev->ss);
#endif
		if (dev->ss == 0) {
			/* de-select SS1 of instance: cspi */
			mxc_request_iomux(MX50_PIN_ECSPI1_MOSI,
						IOMUX_CONFIG_ALT1);

			mxc_request_iomux(MX50_PIN_CSPI_SS0, IOMUX_CONFIG_ALT0);
			mxc_iomux_set_pad(MX50_PIN_CSPI_SS0, 0x04); /* IOMUX 2011/03/31 */
		} else if (dev->ss == 1) {
			/* de-select SS0 of instance: cspi */
			mxc_request_iomux(MX50_PIN_CSPI_SS0, IOMUX_CONFIG_ALT1);

			mxc_request_iomux(MX50_PIN_ECSPI1_MOSI,
						IOMUX_CONFIG_ALT2);
			mxc_iomux_set_pad(MX50_PIN_ECSPI1_MOSI, 0xE4);
			mxc_iomux_set_input(
			MUX_IN_CSPI_IPP_IND_SS1_B_SELECT_INPUT, 0x1);
		}

		mxc_request_iomux(MX50_PIN_CSPI_SCLK, IOMUX_CONFIG_ALT0);
		mxc_iomux_set_pad(MX50_PIN_CSPI_SCLK, 0x4);
		break;
	case CSPI2_BASE_ADDR:
	case CSPI1_BASE_ADDR:
		/* ecspi1-2 fall through */
		break;
	default:
		break;
	}
}
#endif

#ifdef CONFIG_NAND_GPMI
void setup_gpmi_nand(void)
{
	u32 src_sbmr = readl(SRC_BASE_ADDR + 0x4);

	/* Fix for gpmi gatelevel issue */
	mxc_iomux_set_pad(MX50_PIN_SD3_CLK, 0x00e4);

	/* RESETN,WRN,RDN,DATA0~7 Signals iomux*/
	/* Check if 1.8v NAND is to be supported */
	if ((src_sbmr & 0x00000004) >> 2)
		*(u32 *)(IOMUXC_BASE_ADDR + PAD_GRP_START + 0x58) = (0x1 << 13);

	/* RESETN */
	mxc_request_iomux(MX50_PIN_SD3_WP, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_WP, PAD_CTL_DRV_HIGH);

	/* WRN */
	mxc_request_iomux(MX50_PIN_SD3_CMD, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_CMD, PAD_CTL_DRV_HIGH);

	/* RDN */
	mxc_request_iomux(MX50_PIN_SD3_CLK, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_CLK, PAD_CTL_DRV_HIGH);

	/* D0 */
	mxc_request_iomux(MX50_PIN_SD3_D4, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D4, PAD_CTL_DRV_HIGH);

	/* D1 */
	mxc_request_iomux(MX50_PIN_SD3_D5, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D5, PAD_CTL_DRV_HIGH);

	/* D2 */
	mxc_request_iomux(MX50_PIN_SD3_D6, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D6, PAD_CTL_DRV_HIGH);

	/* D3 */
	mxc_request_iomux(MX50_PIN_SD3_D7, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D7, PAD_CTL_DRV_HIGH);

	/* D4 */
	mxc_request_iomux(MX50_PIN_SD3_D0, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D0, PAD_CTL_DRV_HIGH);

	/* D5 */
	mxc_request_iomux(MX50_PIN_SD3_D1, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D1, PAD_CTL_DRV_HIGH);

	/* D6 */
	mxc_request_iomux(MX50_PIN_SD3_D2, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D2, PAD_CTL_DRV_HIGH);

	/* D7 */
	mxc_request_iomux(MX50_PIN_SD3_D3, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_SD3_D3, PAD_CTL_DRV_HIGH);

	/*CE0~3,and other four controls signals muxed on KPP*/
	switch ((src_sbmr & 0x00000018) >> 3) {
	case  0:
		/* Muxed on key */
		if ((src_sbmr & 0x00000004) >> 2)
			*(u32 *)(IOMUXC_BASE_ADDR + PAD_GRP_START + 0x20) =
								(0x1 << 13);

		/* CLE */
		mxc_request_iomux(MX50_PIN_KEY_COL0, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_COL0, PAD_CTL_DRV_HIGH);

		/* ALE */
		mxc_request_iomux(MX50_PIN_KEY_ROW0, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_ROW0, PAD_CTL_DRV_HIGH);

		/* READY0 */
		mxc_request_iomux(MX50_PIN_KEY_COL3, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_COL3,
				PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL |
				PAD_CTL_100K_PU);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_RDY0_IN_SELECT_INPUT,
			INPUT_CTL_PATH0);

		/* DQS */
		mxc_request_iomux(MX50_PIN_KEY_ROW3, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_ROW3, PAD_CTL_DRV_HIGH);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_DQS_IN_SELECT_INPUT,
			INPUT_CTL_PATH0);

		/* CE0 */
		mxc_request_iomux(MX50_PIN_KEY_COL1, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_COL1, PAD_CTL_DRV_HIGH);

		/* CE1 */
		mxc_request_iomux(MX50_PIN_KEY_ROW1, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_ROW1, PAD_CTL_DRV_HIGH);

		/* CE2 */
		mxc_request_iomux(MX50_PIN_KEY_COL2, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_COL2, PAD_CTL_DRV_HIGH);

		/* CE3 */
		mxc_request_iomux(MX50_PIN_KEY_ROW2, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_KEY_ROW2, PAD_CTL_DRV_HIGH);

		break;
	case 1:
		if ((src_sbmr & 0x00000004) >> 2)
			*(u32 *)(IOMUXC_BASE_ADDR + PAD_GRP_START + 0xc) =
								(0x1 << 13);

		/* CLE */
		mxc_request_iomux(MX50_PIN_EIM_DA8, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA8, PAD_CTL_DRV_HIGH);

		/* ALE */
		mxc_request_iomux(MX50_PIN_EIM_DA9, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA9, PAD_CTL_DRV_HIGH);

		/* READY0 */
		mxc_request_iomux(MX50_PIN_EIM_DA14, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA14,
				PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL |
				PAD_CTL_100K_PU);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_RDY0_IN_SELECT_INPUT,
			INPUT_CTL_PATH2);

		/* DQS */
		mxc_request_iomux(MX50_PIN_EIM_DA15, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA15, PAD_CTL_DRV_HIGH);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_DQS_IN_SELECT_INPUT,
			INPUT_CTL_PATH2);

		/* CE0 */
		mxc_request_iomux(MX50_PIN_EIM_DA10, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA10, PAD_CTL_DRV_HIGH);

		/* CE1 */
		mxc_request_iomux(MX50_PIN_EIM_DA11, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA11, PAD_CTL_DRV_HIGH);

		/* CE2 */
		mxc_request_iomux(MX50_PIN_EIM_DA12, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA12, PAD_CTL_DRV_HIGH);

		/* CE3 */
		mxc_request_iomux(MX50_PIN_EIM_DA13, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA13, PAD_CTL_DRV_HIGH);

		break;
	case 2:
		if ((src_sbmr & 0x00000004) >> 2)
			*(u32 *)(IOMUXC_BASE_ADDR + PAD_GRP_START + 0x48) =
								(0x1 << 13);

		/* CLE */
		mxc_request_iomux(MX50_PIN_DISP_D8, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D8, PAD_CTL_DRV_HIGH);

		/* ALE */
		mxc_request_iomux(MX50_PIN_DISP_D9, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D9, PAD_CTL_DRV_HIGH);

		/* READY0 */
		mxc_request_iomux(MX50_PIN_DISP_D14, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D14,
				PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL |
				PAD_CTL_100K_PU);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_RDY0_IN_SELECT_INPUT,
			INPUT_CTL_PATH1);

		/* DQS */
		mxc_request_iomux(MX50_PIN_DISP_D15, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D15, PAD_CTL_DRV_HIGH);
		mxc_iomux_set_input(
			MUX_IN_RAWNAND_U_GPMI_INPUT_GPMI_DQS_IN_SELECT_INPUT,
			INPUT_CTL_PATH1);

		/* CE0 */
		mxc_request_iomux(MX50_PIN_DISP_D10, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D10, PAD_CTL_DRV_HIGH);

		/* CE1 */
		mxc_request_iomux(MX50_PIN_EIM_DA11, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_EIM_DA11, PAD_CTL_DRV_HIGH);

		/* CE2 */
		mxc_request_iomux(MX50_PIN_DISP_D12, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D12, PAD_CTL_DRV_HIGH);

		/* CE3 */
		mxc_request_iomux(MX50_PIN_DISP_D13, IOMUX_CONFIG_ALT2);
		mxc_iomux_set_pad(MX50_PIN_DISP_D13, PAD_CTL_DRV_HIGH);

		break;
	default:
		break;
	}
}
#endif

#if 1 /* E_BOOK *//* Add for SONY Board 2010/11/16 */
/* setup peripheral Voltage */
static void setup_hve(void)
{
  writel(1 << 13, IOMUXC_BASE_ADDR+0x0674);
  writel(1 << 13, IOMUXC_BASE_ADDR+0x067c);
  writel(1 << 13, IOMUXC_BASE_ADDR+0x0688);
  writel(1 << 13, IOMUXC_BASE_ADDR+0x06b8);
  writel(1 << 13, IOMUXC_BASE_ADDR+0x06c0);
  /* set to 1.8V for FEC */
  writel(1 << 13, IOMUXC_BASE_ADDR+0x690);
  writel(1 << 13, IOMUXC_BASE_ADDR+0x6b0);
#if 1 /* E_BOOK *//* for test 2011/05/26 */  
  writel(1 << 13, IOMUXC_BASE_ADDR+0x6bc);
#endif
}
#endif
#ifdef CONFIG_MXC_FEC

#ifdef CONFIG_GET_FEC_MAC_ADDR_FROM_IIM

#define HW_OCOTP_MACn(n)	(0x00000250 + (n) * 0x10)

int fec_get_mac_addr(unsigned char *mac)
{
	u32 *ocotp_mac_base =
		(u32 *)(OCOTP_CTRL_BASE_ADDR + HW_OCOTP_MACn(0));
	int i;

	for (i = 0; i < 6; ++i, ++ocotp_mac_base)
		mac[6 - 1 - i] = readl(++ocotp_mac_base);

	return 0;
}
#endif

static void setup_fec(void)
{
	volatile unsigned int reg;

#if defined(CONFIG_MX50_RDP)
	/* FEC_EN: gpio6-23 set to 0 to enable FEC */
	mxc_request_iomux(MX50_PIN_I2C3_SDA, IOMUX_CONFIG_ALT1);

	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg &= ~(1 << 23);
	writel(reg, GPIO6_BASE_ADDR + 0x0);

	reg = readl(GPIO6_BASE_ADDR + 0x4);
	reg |= (1 << 23);
	writel(reg, GPIO6_BASE_ADDR + 0x4);
#endif

	/*FEC_MDIO*/
	mxc_request_iomux(MX50_PIN_SSI_RXC, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX50_PIN_SSI_RXC, 0x4); /* change 2011/07/06 */
	mxc_iomux_set_input(MUX_IN_FEC_FEC_MDI_SELECT_INPUT, 0x1);

	/*FEC_MDC*/
	mxc_request_iomux(MX50_PIN_SSI_RXFS, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX50_PIN_SSI_RXFS, 0x004);

	/* FEC RXD1 */
	mxc_request_iomux(MX50_PIN_DISP_D3, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D3, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_RDATA_1_SELECT_INPUT, 0x0);

	/* FEC RXD0 */
	mxc_request_iomux(MX50_PIN_DISP_D4, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D4, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_RDATA_0_SELECT_INPUT, 0x0);

	 /* FEC TXD1 */
	mxc_request_iomux(MX50_PIN_DISP_D6, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D6, 0x004);

	/* FEC TXD0 */
	mxc_request_iomux(MX50_PIN_DISP_D7, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D7, 0x004);

	/* FEC TX_EN */
	mxc_request_iomux(MX50_PIN_DISP_D5, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D5, 0x004);

	/* FEC TX_CLK */
	mxc_request_iomux(MX50_PIN_DISP_D0, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D0, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_TX_CLK_SELECT_INPUT, 0x0);

	/* FEC RX_ER */
	mxc_request_iomux(MX50_PIN_DISP_D1, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D1, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_RX_ER_SELECT_INPUT, 0);

	/* FEC CRS */
	mxc_request_iomux(MX50_PIN_DISP_D2, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_DISP_D2, 0x0);
	mxc_iomux_set_input(MUX_IN_FEC_FEC_RX_DV_SELECT_INPUT, 0);

#if defined(CONFIG_MX50_RDP)
	/* FEC_RESET_B: gpio4-12 */
	mxc_request_iomux(MX50_PIN_ECSPI1_SCLK, IOMUX_CONFIG_ALT1);

	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg &= ~(1 << 12);
	writel(reg, GPIO4_BASE_ADDR + 0x0);

	reg = readl(GPIO4_BASE_ADDR + 0x4);
	reg |= (1 << 12);
	writel(reg, GPIO4_BASE_ADDR + 0x4);

	udelay(500);

	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg |= (1 << 12);
	writel(reg, GPIO4_BASE_ADDR + 0x0);
#elif defined(CONFIG_MX50_ARM2)
	/* phy reset: gpio6_28 */
	mxc_request_iomux(MX50_PIN_WDOG, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_WDOG, 0x04);/* IOMUX 2011/03/31 */
	setup_gpio(GPIO6_BASE_ADDR, 28, GPIO_CFG_LOW);
	udelay(500);

	set_gpio(GPIO6_BASE_ADDR, 28, 1);
	udelay(10);
#else
#	error "Unsupported board!"
#endif
}
#endif

#ifdef CONFIG_CMD_MMC

struct fsl_esdhc_cfg esdhc_cfg[3] = {
	{MMC_SDHC1_BASE_ADDR, 1, 1},
	{MMC_SDHC2_BASE_ADDR, 1, 1},
	{MMC_SDHC3_BASE_ADDR, 1, 1},
};


#ifdef CONFIG_DYNAMIC_MMC_DEVNO
int get_mmc_env_devno(void)
{
	uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	int mmc_devno = 0;

	switch (soc_sbmr & 0x00300000) {
	default:
	case 0x0:
		mmc_devno = 0;
		break;
	case 0x00100000:
		mmc_devno = 1;
		break;
	case 0x00200000:
		mmc_devno = 2;
		break;
	}

	return mmc_devno;
}
#endif


int esdhc_gpio_init(bd_t *bis)
{
	s32 status = 0;
	u32 index = 0;

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM;
		++index) {
		switch (index) {
		case 0:
			mxc_request_iomux(MX50_PIN_SD1_CMD, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_CLK, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_D0,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_D1,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_D2,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD1_D3,  IOMUX_CONFIG_ALT0);

#if 1 /* E_BOOK *//* Change Pull-UP 2011/02/23 */
			mxc_iomux_set_pad(MX50_PIN_SD1_CMD, 0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_CLK, 0x14);
			mxc_iomux_set_pad(MX50_PIN_SD1_D0,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D1,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D2,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D3,  0x1D4);
#else
			mxc_iomux_set_pad(MX50_PIN_SD1_CMD, 0x1E4);
			mxc_iomux_set_pad(MX50_PIN_SD1_CLK, 0xD4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D0,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D1,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D2,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD1_D3,  0x1D4);
#endif
#if 1 /* E_BOOK *//* for SD boot 2010/12/10 */
			setup_gpio(GPIO5_BASE_ADDR, 12, GPIO_CFG_LOW);
#endif


			break;
		case 1:
			mxc_request_iomux(MX50_PIN_SD2_CMD, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_CLK, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D0,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D1,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D2,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D3,  IOMUX_CONFIG_ALT0);
#if 0 /* E_BOOK *//* delete for Reader 2010/12/08 */
			mxc_request_iomux(MX50_PIN_SD2_D4,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D5,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D6,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD2_D7,  IOMUX_CONFIG_ALT0);
#endif

			mxc_iomux_set_pad(MX50_PIN_SD2_CMD, 0x1D4); /* change 2011/07/07 */
			mxc_iomux_set_pad(MX50_PIN_SD2_CLK, 0x14); /* change 2011/07/07 */
			mxc_iomux_set_pad(MX50_PIN_SD2_D0,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D1,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D2,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D3,  0x1D4);
#if 0 /* E_BOOK *//* delete for Reader 2010/12/08 */
			mxc_iomux_set_pad(MX50_PIN_SD2_D4,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D5,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D6,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD2_D7,  0x1D4);
#endif

			break;
		case 2:
#ifndef CONFIG_NAND_GPMI
			mxc_request_iomux(MX50_PIN_SD3_CMD, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_CLK, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D0,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D1,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D2,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D3,  IOMUX_CONFIG_ALT0);
#if 0 /* E_BOOK *//* delete for Reader 2010/12/09 */
			mxc_request_iomux(MX50_PIN_SD3_D4,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D5,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D6,  IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX50_PIN_SD3_D7,  IOMUX_CONFIG_ALT0);
#endif

			mxc_iomux_set_pad(MX50_PIN_SD3_CMD, 0x1D4); /* change to PU47K 2011/07/05 */
			mxc_iomux_set_pad(MX50_PIN_SD3_CLK, 0x4); /* change to OFF 2011/07/05 */
			mxc_iomux_set_pad(MX50_PIN_SD3_D0,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D1,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D2,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D3,  0x1D4);
#if 0 /* E_BOOK *//* delete for Reader 2010/12/09 */
			mxc_iomux_set_pad(MX50_PIN_SD3_D4,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D5,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D6,  0x1D4);
			mxc_iomux_set_pad(MX50_PIN_SD3_D7,  0x1D4);
#endif
#endif
			break;
		default:
			printf("Warning: you configured more ESDHC controller"
				"(%d) as supported by the board(2)\n",
				CONFIG_SYS_FSL_ESDHC_NUM);
			return status;
			break;
		}
		status |= fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
	}

	return status;
}

int board_mmc_init(bd_t *bis)
{
	if (!esdhc_gpio_init(bis))
		return 0;
	else
		return -1;
}

#endif

#ifdef CONFIG_MXC_EPDC
#ifdef CONFIG_SPLASH_SCREEN
int setup_splash_img(void)
{
#ifdef CONFIG_SPLASH_IS_IN_MMC
	int mmc_dev = get_mmc_env_devno();
	ulong offset = CONFIG_SPLASH_IMG_OFFSET;
	ulong size = CONFIG_SPLASH_IMG_SIZE;
	ulong addr = 0;
	char *s = NULL;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

#if 1 /* E_BOOK */
	addr = CONFIG_FB_BASE;
#else /* E_BOOK */
	s = getenv("splashimage");

	if (NULL == s) {
		puts("env splashimage not found!\n");
		return -1;
	}
	addr = simple_strtoul(s, NULL, 16);
#endif /* E_BOOK */

	if (!mmc) {
		printf("MMC Device %d not found\n",
			mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return  -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
/* 2011/04/11 FY11 : Fixed the roudup bug. */
	if ( offset % (mmc->read_bl_len) )
	{
		blk_start--;
	}
	blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
					blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	return (n == blk_cnt) ? 0 : -1;
#endif
}
#endif

vidinfo_t panel_info = {
	.vl_refresh = 85,
	.vl_col = 800,
	.vl_row = 600,
	.vl_pixclock = 30000000,
	.vl_left_margin = 8,
	.vl_right_margin = 164,
	.vl_upper_margin = 4,
	.vl_lower_margin = 8,
	.vl_hsync = 4,
	.vl_vsync = 1,
	.vl_sync = 0,
	.vl_mode = 0,
	.vl_flag = 0,
	.vl_bpix = 3,
	cmap:0,
};


/* 2011/04/05 FY11 : Supported to display boot image. */
static void setup_epdc_power(void)
{

#if 1 /* E_BOOK */
	int retry, wakeup_retry = 1;
	int ret = 0, i, reg_num;
	unsigned char buf[4];
	unsigned int addr;

/* 2011/06/30 FY11 : Added wait for VSYS_EPD_ON. */
	/* wait VSYS */
	udelay(20000);

	/* wake up EPDPMIC and set power sequence. */
retry_wakeup:
	retry = TPS65185_I2C_RETRY;
	/* Set PMIC Wakeup to high - enable Display power */
	set_gpio( GPIO3_BASE_ADDR, 8, 1 );

	udelay(15000);

/* 2011/05/12 FY11 : Supported TPS65181. */
	switch (settings_for_epd.pmic_version & TPS6518X_REVID_MASK )
	{
		case TPS65181_REVID:
			addr = TPS65181_PWRSEQ0_REG;
			reg_num = 3;
			buf[0] = 0xE4;
			buf[1] = 0x2F;
			buf[2] = 0xF5;
			break;
		case TPS65185_REVID:
			addr = TPS65185_UPSEQ0_REG;
			reg_num = 4;
			buf[0] = 0xE4;
			buf[1] = 0x55;
			buf[2] = 0x1E;
			buf[3] = 0xE0;
			break;
	}

	/* Set power up/down sequence. */
	for ( i=0; i < reg_num && retry > 0; )
	{
/* 2011/05/12 FY11 : Supported TPS65181. */
		ret = i2c_write(settings_for_epd.pmic_i2c_addr, addr, 1, &buf[i], 1);
		if ( ret < 0 )
		{
			printf( "i2c_write failed.\n" );
			retry--;
			udelay(100000);
			continue;
		}
		i++;
		addr++;
	}
	if ( retry == 0 )
	{
		printf( "i2c_write retry out.\n" );
		set_gpio( GPIO3_BASE_ADDR, 8, 0 );
		if ( wakeup_retry )
		{
			wakeup_retry = 0;
			udelay(1000);
			goto retry_wakeup;
		}
		return ;
	}


#else /* E_BOOK */
	/* EPDC PWRSTAT - GPIO3[28] for PWR_GOOD status */
	mxc_request_iomux(MX50_PIN_EPDC_PWRSTAT, IOMUX_CONFIG_ALT1);

	/* EPDC VCOM0 - GPIO4[21] for VCOM control */
	mxc_request_iomux(MX50_PIN_EPDC_VCOM0, IOMUX_CONFIG_ALT1);
	/* Set as output */
	reg = readl(GPIO4_BASE_ADDR + 0x4);
	reg |= (1 << 21);
	writel(reg, GPIO4_BASE_ADDR + 0x4);

	/* UART4 TXD - GPIO6[16] for EPD PMIC WAKEUP */
	mxc_request_iomux(MX50_PIN_UART4_TXD, IOMUX_CONFIG_ALT1);
	/* Set as output */
	reg = readl(GPIO6_BASE_ADDR + 0x4);
	reg |= (1 << 16);
	writel(reg, GPIO6_BASE_ADDR + 0x4);
#endif /* E_BOOK */
}

/* 2011/04/05 FY11 : Supported to display boot image. */
void epdc_power_on(void)
{
	unsigned int reg;
#if 1 /* E_BOOK */
	int ret, retry;
	uchar buf;
	unsigned int addr;

/* 2011/05/12 FY11 : Supported TPS65181. */
	switch (settings_for_epd.pmic_version & TPS6518X_REVID_MASK )
	{
		case TPS65181_REVID:
			/* enable v3p3 using gpio. */
			set_gpio( GPIO3_BASE_ADDR, 9, 0);

			/* set VCOM adjustment method. */
			addr = TPS65181_VNADJUST_REG;
			ret = i2c_read(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s vn adjust reg i2c_read failed.\n", __func__ );
				return ;
			}
			buf |= TPS65181_VNADJUST_VCOM;
			ret = i2c_write(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s vn adjust reg i2c_write failed.\n", __func__ );
				return ;
			}

			/* set vcom value. */
			addr = TPS65181_VCOM_REG;
			buf = settings_for_epd.settings.vcom & 0xFF;
			ret = i2c_write(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1); 
			if ( ret < 0 )
			{
				printf("%s vcom reg i2c_write failed.\n", __func__ );
				return ;
			}

			/* Transition from STANDBY to ACTIVE and rails power up. */
			addr = TPS6518X_ENABLE_REG;
			ret = i2c_read(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s enable reg i2c_read failed.\n", __func__ );
				return ;
			}
			buf |= TPS6518X_ENABLE_ACTIVE;
			buf &= ~TPS6518X_ENABLE_STANDBY;
			ret = i2c_write(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s enable reg i2c_write failed.\n", __func__ );
				return ;
			}
			break;
		case TPS65185_REVID:
			addr = TPS6518X_ENABLE_REG;

			/* enable v3p3 through I2C. */
			ret = i2c_read(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s v3p3 i2c_read failed.\n", __func__ );
				return ;
			}
			buf |= TPS6518X_ENABLE_V3P3;
			ret = i2c_write(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s v3p3 i2c_write failed.\n", __func__ );
				return ;
			}

			/* Set PMIC Power up */
			set_gpio( GPIO3_BASE_ADDR, 9, 1);

			break;
	}

	/* Wait for PWRGOOD == 1 */
/* 2011/08/25 FY11 : Added retry out for PowerGood. */
	retry = 1500;
	while (retry--) {
		reg = readl(GPIO3_BASE_ADDR + 0x0);
/* 2011/08/25 FY11 : Fixed the bug that does not wait PowerGood. */
		if (reg & (1 << 28))
			break;

		udelay(100);
	}

	if ( retry <= 0 )
	{
		printf("%s timeout for PowerGood.\n", __func__ );
	}

	/* Enable VCOM */
	set_gpio( GPIO3_BASE_ADDR, 27, 1 );

#else /* E_BOOK */
	/* Set PMIC Wakeup to high - enable Display power */
	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg |= (1 << 16);
	writel(reg, GPIO6_BASE_ADDR + 0x0);

	/* Wait for PWRGOOD == 1 */
	while (1) {
		reg = readl(GPIO3_BASE_ADDR + 0x0);
		if (!(reg & (1 << 28)))
			break;

		udelay(100);
	}

	/* Enable VCOM */
	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg |= (1 << 21);
	writel(reg, GPIO4_BASE_ADDR + 0x0);

	reg = readl(GPIO4_BASE_ADDR + 0x0);

	udelay(500);
#endif /* E_BOOK */
}


static void remove_epdc(void);
void  epdc_power_off(void)
{
#if 1 /* E_BOOK */
	unsigned int addr;
	int ret = 0;
	uchar buf;

/* 2011/06/09 FY11 : Disable ports for EPD. */
	remove_epdc();

	/* Disable VCOM */
	set_gpio( GPIO3_BASE_ADDR, 27, 0 );

/* 2011/06/06 FY11 : Support to turn off EPD power. */
	switch (settings_for_epd.pmic_version & TPS6518X_REVID_MASK )
	{
		case TPS65181_REVID:
			/* transition to standby */
			addr = TPS6518X_ENABLE_REG;
			ret = i2c_read(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s enable reg i2c_read failed.\n", __func__ );
				return ;
			}
			buf |= TPS6518X_ENABLE_STANDBY;
			buf &= ~TPS6518X_ENABLE_ACTIVE;
			ret = i2c_write(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s enable reg i2c_write failed.\n", __func__ );
				return ;
			}

/* 2011/06/08 FY11 : Modified wait time. (80msec -> 200msec) */
			udelay(200000);

			/* Disable V3P3 */
			set_gpio( GPIO3_BASE_ADDR, 9, 1);
			break;
		case TPS65185_REVID:
			/* Disable PWR0 */
			set_gpio( GPIO3_BASE_ADDR, 9, 0);

			udelay(80000);

			/* Disable V3P3 */
			addr = TPS6518X_ENABLE_REG;
			ret = i2c_read(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s v3p3 i2c_read failed.\n", __func__ );
				return ;
			}
			buf &= ~TPS6518X_ENABLE_V3P3;
			ret = i2c_write(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s v3p3 i2c_write failed.\n", __func__ );
				return ;
			}
			break;
	}

	/* Set PMIC Wakeup to low - disable Display power */
	set_gpio( GPIO3_BASE_ADDR, 8, 0 );
#else /* E_BOOK */
	/* Set PMIC Wakeup to low - disable Display power */
	reg = readl(GPIO6_BASE_ADDR + 0x0);
	reg |= 0 << 16;
	writel(reg, GPIO6_BASE_ADDR + 0x0);

	/* Disable VCOM */
	reg = readl(GPIO4_BASE_ADDR + 0x0);
	reg |= 0 << 21;
	writel(reg, GPIO4_BASE_ADDR + 0x0);
#endif /* E_BOOK */
}


/* 2011/04/05 FY11 : Supported temperature range table. */
int setup_waveform_file( u_char *pucTrt, ulong *pulTrtSize )
{
#ifdef CONFIG_WAVEFORM_FILE_IN_MMC
	int mmc_dev = get_mmc_env_devno();
	ulong offset = CONFIG_WAVEFORM_FILE_OFFSET;
	ulong size = CONFIG_WAVEFORM_FILE_SIZE;
	ulong addr = CONFIG_FB_BASE;
	ulong wfdata_offset = 0;
	u_char *pWaveform = NULL;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

	if (!mmc) {
		printf("MMC Device %d not found\n",
			mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
/* 2011/04/11 FY11 : Fixed the roudup bug. */
	if ( offset % (mmc->read_bl_len) )
	{
		blk_start--;
	}
	blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
		blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

/* 2011/04/05 FY11 : Supported temperature range table. */
	*pulTrtSize = ((u_char*)addr)[TRC_ADDR] + 1;
	wfdata_offset = WF_HEADER_SIZE + (*pulTrtSize) + 1;
	pWaveform = (uchar*)addr + wfdata_offset;
	local_memcpy( (char*)CONFIG_WAVEFORM_BUF_ADDR, pWaveform, size - wfdata_offset );
	local_memcpy( pucTrt, (uchar*)addr + WF_HEADER_SIZE, *pulTrtSize );

/* 2011/06/06 FY11 : Supported to share static memory for EPD with kernel. */
	{
		char * pDst;
		pDst = (char*)(CONFIG_WAVEFORM_BUF_ADDR + CONFIG_WAVEFORM_BUF_SIZE - sizeof(struct epd_settings_in_emmc));
		local_memcpy( pDst,
			(u_char*)(addr + size - sizeof(struct epd_settings_in_emmc)), 
			sizeof(struct epd_settings_in_emmc) );
		pDst -= sizeof(unsigned long);
		local_memcpy( pDst, (char*)pulTrtSize, sizeof(unsigned long) );
		pDst -= (*pulTrtSize);
		local_memcpy( pDst, (uchar*)addr + WF_HEADER_SIZE, *pulTrtSize );
		pDst -= WF_VER_SIZE;
		local_memcpy( pDst, (uchar*)addr + WF_VER_OFFSET, WF_VER_SIZE ); 
	}

/* 2011/05/12 FY11 : Supported TPS65181. */
	local_memcpy( (u_char*)&(settings_for_epd.settings),
		(u_char*)(addr + size - sizeof(struct epd_settings_in_emmc)),
		sizeof(struct epd_settings_in_emmc) );


	return (n == blk_cnt) ? 0 : -1;
#else
	return -1;
#endif
}



/* 2011/04/12 FY11 : Supported to read and write panel init flag. */
#define EMMC_READ	1
#define EMMC_WRITE	0
/* size must be smaller than mmc block size. */
static int read_write_mmc_data( u_char *pBuf, ulong offset, ulong size , int read_flag )
{
	int ret = 0;
	uint blk_start, n;
	int mmc_dev = get_mmc_env_devno();
	struct mmc *mmc = find_mmc_device(mmc_dev);
	u_char *pTempBuf;

	if (!mmc) {
		printf("MMC Device %d not found\n",
			mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	if ( (size > mmc->read_bl_len) &&  
	     ( (offset/(mmc->read_bl_len)) == ((offset+size)/(mmc->read_bl_len)) ) )
	{
		printf("%s area is over block !\n", __func__ );
		return -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	if ( offset % (mmc->read_bl_len) )
	{
		blk_start--;
	}

	pTempBuf = malloc( mmc->read_bl_len );
	if ( pTempBuf == NULL )
	{
		printf("%s malloc failed.\n", __func__ );
		return -1;
	}

	n = mmc->block_dev.block_read(mmc_dev, blk_start,
		1, pTempBuf);
	if ( n != 1 )
	{
		printf("%s mmc read failed.\n", __func__ );
		free(pTempBuf);
		return -1;
	}

	if ( read_flag )
	{
		memcpy( pBuf, pTempBuf + (offset%(mmc->read_bl_len)),  size);
	}
	else /* write */
	{
		memcpy( pTempBuf + (offset%(mmc->read_bl_len)), pBuf, size );
		n = mmc->block_dev.block_write(mmc_dev, blk_start,
						1, pTempBuf);
		if ( n != 1 )
		{
			printf("%s mmc write failed.\n", __func__ );
			ret = -1;
		}
	}
	
	free(pTempBuf);

	return ret;
	
} 

/* 2011/04/12 FY11 : Supported panel init flag. */
int get_epd_setting_flag( unsigned long *pulFlag )
{
	int ret = 0;
/* 2011/05/12 FY11 : Supported TPS65181. */
	*pulFlag = settings_for_epd.settings.flag;

	return ret;
}


/* 2011/04/12 FY11 : Supported panel init flag. */
int set_epd_setting_flag( unsigned long ulFlag )
{
	int ret = 0;

	ulong offset = CONFIG_WAVEFORM_FILE_OFFSET + CONFIG_WAVEFORM_FILE_SIZE - sizeof(struct epd_settings_in_emmc) + offsetof(struct epd_settings_in_emmc, flag);

	ret = read_write_mmc_data( (u_char*)&ulFlag, offset, sizeof(ulFlag), EMMC_WRITE );

	return ret;
}


/* 2011/04/05 FY11 : Supported to get temperature. */
int get_temperature( int *pTemp )
{
	int ret = 0, retry = 100;
	uchar buf;
	uint addr;

/* 2011/05/12 FY11 : Supported TPS65181. */
	switch (settings_for_epd.pmic_version & TPS6518X_REVID_MASK )
	{
		case TPS65185_REVID:
			addr = TPS65185_TMST1_REG;
			ret = i2c_read(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
			if ( ret < 0 )
			{
				printf("%s set READ_THERM i2c read failed.\n", __func__ );
				return -1;
			}
			buf |= READ_THERM;
			ret = i2c_write(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1);
			if ( ret < 0 )
			{
				printf("%s set READ_THERM i2c write failed.\n", __func__ );
				return -1;
			}

/* 2011/05/25 FY11 : Changed the flagh to confirm completion of reading temperature. */
			addr = TPS6518X_INT_STATUS2;
			while ( retry > 0 )
			{
				ret = i2c_read( settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
				if ( ret < 0 )
				{
					printf("%s read ADC i2c read failed.\n", __func__ );
					return -1;
				}
				if ( buf & TPS6518X_INT_STATUS2_EOC )
				{
					break;
				}
				udelay(10);
				retry--;
			}

			if ( retry <= 0 )
			{
				printf("%s retry out.\n", __func__ );
				return -1;
			}

			break;
	}

	addr = TPS6518X_TMSTVAL_REG;
	ret = i2c_read(settings_for_epd.pmic_i2c_addr, addr, 1, &buf, 1 );
	if ( ret < 0 )
	{
		printf( "%s i2c_read failed\n", __func__ );
		return -1;
	}
	
	*pTemp = buf;
	if ( (*pTemp) >= MINUS_TEMPERATURE )
	{
		(*pTemp) -= MINUS_TEMP_ADJUST_VAL;
	}

	return ret;
}


static void setup_epdc(void)
{
	unsigned int reg;

	/* epdc iomux settings */
	mxc_request_iomux(MX50_PIN_EPDC_D0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D0, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_D1, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D1, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_D2, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D2, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_D3, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D3, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_D4, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D4, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_D5, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D5, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_D6, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D6, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_D7, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D7, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_GDCLK, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_GDCLK, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_GDSP, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_GDSP, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_GDOE, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_GDOE, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_SDCLK, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDCLK, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_SDOE, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDOE, PAD_CTL_DRV_LOW);
	mxc_request_iomux(MX50_PIN_EPDC_SDLE, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDLE, PAD_CTL_DRV_LOW);
#if 0 /* E_BOOK *//* delete for Reader 2010/12/08 */
	mxc_request_iomux(MX50_PIN_EPDC_SDSHR, IOMUX_CONFIG_ALT0);
#endif
	mxc_request_iomux(MX50_PIN_EPDC_SDCE0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDCE0, PAD_CTL_DRV_LOW);
#if 0 /* E_BOOK *//* delete for Reader 2010/12/08 */
	mxc_request_iomux(MX50_PIN_EPDC_SDCE1, IOMUX_CONFIG_ALT0);
#endif

	/*** epdc Maxim PMIC settings ***/

#if 1 /* E_BOOK */
	/* EPDC D8 - GPIO3[8] for EPD PMIC WAKEUP */
	mxc_request_iomux(MX50_PIN_EPDC_D8, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D8, PAD_CTL_DRV_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 8, GPIO_CFG_LOW);

	/* EPDC D9 - GPIO3[9] for EPD PMIC PWRUP */
	mxc_request_iomux(MX50_PIN_EPDC_D9, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D9, PAD_CTL_DRV_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 9, GPIO_CFG_LOW);

/* 2011/05/09 FY11 : Added function and pad setting for D10. */
	/* EPDC D10 - GPIO3[10] */
	mxc_request_iomux(MX50_PIN_EPDC_D10, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D10, 
			PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU|PAD_CTL_DRV_HIGH);
	setup_gpio(GPIO3_BASE_ADDR, 10, GPIO_CFG_INPUT);

/* 2011/05/09 FY11 : Added function and pad setting for D11. */
	/* EPDC D11 - GPIO3[11] */
	mxc_request_iomux(MX50_PIN_EPDC_D11, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D11, PAD_CTL_DRV_HIGH); 
	setup_gpio(GPIO3_BASE_ADDR, 11, GPIO_CFG_HIGH);

/* 2011/05/09 FY11 : Added function and pad setting for SDOEZ. */
	mxc_request_iomux(MX50_PIN_EPDC_SDOEZ, IOMUX_CONFIG_ALT1);
/* 2011/05/27 FY11 : Fixed the false recognition of EPD PMIC type. */
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDOEZ, 
			PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_22K_PU|PAD_CTL_DRV_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 21, GPIO_CFG_INPUT);

	/* EPDC PWRCOM - GPIO3[27] for VCOM control */
	mxc_request_iomux(MX50_PIN_EPDC_PWRCOM, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCOM, PAD_CTL_DRV_HIGH); 
	setup_gpio(GPIO3_BASE_ADDR, 27, GPIO_CFG_LOW);

	/* EPDC PWRSTAT - GPIO3[28] for PWR_GOOD status */
	mxc_request_iomux(MX50_PIN_EPDC_PWRSTAT, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRSTAT, 
			PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU|PAD_CTL_DRV_HIGH);
	setup_gpio(GPIO3_BASE_ADDR, 28, GPIO_CFG_INPUT);
#else /* E_BOOK */
	/* EPDC PWRSTAT - GPIO3[28] for PWR_GOOD status */
	mxc_request_iomux(MX50_PIN_EPDC_PWRSTAT, IOMUX_CONFIG_ALT1);

	/* EPDC VCOM0 - GPIO4[21] for VCOM control */
	mxc_request_iomux(MX50_PIN_EPDC_VCOM0, IOMUX_CONFIG_ALT1);

	/* UART4 TXD - GPIO6[16] for EPD PMIC WAKEUP */
	mxc_request_iomux(MX50_PIN_UART4_TXD, IOMUX_CONFIG_ALT1);
#endif /* E_BOOK */


/* 2011/05/27 FY11 : Fixed the false recognition of EPD PMIC type. */
	udelay(10);
/* 2011/05/12 FY11 : Supported TPS65181. */
	reg = readl(GPIO3_BASE_ADDR + GPIO_PSR);
	if ( reg & TPS65185_PORT_MASK )
	{
		settings_for_epd.pmic_version = TPS65185_REVID;
		settings_for_epd.pmic_i2c_addr = TPS65185_I2C_ADDR;
	}
	else
	{
		settings_for_epd.pmic_version = TPS65181_REVID;
		settings_for_epd.pmic_i2c_addr = TPS65181_I2C_ADDR;
	}
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDOEZ, 
			PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PD|PAD_CTL_DRV_LOW);

	switch (settings_for_epd.pmic_version & TPS6518X_REVID_MASK )
	{
		case TPS65181_REVID:
			setup_gpio(GPIO3_BASE_ADDR, 9, GPIO_CFG_HIGH);
			break;
		case TPS65185_REVID:
			setup_gpio(GPIO3_BASE_ADDR, 9, GPIO_CFG_LOW);
			break;
	}


	/*** Set pixel clock rates for EPDC ***/

	/* EPDC AXI clk from PFD3 and EPDC PIX clk from PFD5 */
	reg = readl(CCM_BASE_ADDR + CLKCTL_CLKSEQ_BYPASS);
	reg &= ~((0x3 << 4) | (0x3 << 12));
	reg |= (0x1 << 4) | (0x1 << 12);
	writel(reg, CCM_BASE_ADDR + CLKCTL_CLKSEQ_BYPASS);

	/* Set bit to flush multiple edges out of PLL vco */
	writel((1 << 7), ANATOP_BASE_ADDR + 0x64);

	/* APLL enable */
	writel(1, ANATOP_BASE_ADDR + 0x64);

	/* Wait for relock (ANADIG_APLL_LOCK bit) */
	while (1) {
		reg = readl(ANATOP_BASE_ADDR + 0x70);
		if (!(reg & (1 << 31)))
			break;

		udelay(100);
	}

	/* Clear after relocking, then wait 10 us */
	writel((1 << 7), ANATOP_BASE_ADDR + 0x68);
	udelay(10);

	/* PFD3 enable */
	writel(1 << 31, ANATOP_BASE_ADDR + 0x18);

	/* PFD5 enable */
	writel(1 << 15, ANATOP_BASE_ADDR + 0x28);

	/* EPDC AXI clk enable and set to 160MHz (480/3) */
	reg = readl(CCM_BASE_ADDR + 0xA8);
	reg &= ~((0x3 << 30) | 0x3F);
	reg |= (0x2 << 30) | 0x3;
	writel(reg, CCM_BASE_ADDR + 0xA8);

	/* EPDC PIX clk enable and set to 30MHz (480/16) */
	reg = readl(CCM_BASE_ADDR + 0xA0);
	reg &= ~((0x3 << 30) | (0x3 << 12) | 0xFFF);
	reg |= (0x2 << 30) | (0x1 << 12) | 0x10;
	writel(reg, CCM_BASE_ADDR + 0xA0);

	panel_info.epdc_data.working_buf_addr = CONFIG_WORKING_BUF_ADDR;
	panel_info.epdc_data.waveform_buf_addr = CONFIG_WAVEFORM_BUF_ADDR;

	panel_info.epdc_data.wv_modes.mode_init = 0;
	panel_info.epdc_data.wv_modes.mode_du = 1;
	panel_info.epdc_data.wv_modes.mode_gc4 = 3;
	panel_info.epdc_data.wv_modes.mode_gc8 = 2;
	panel_info.epdc_data.wv_modes.mode_gc16 = 2;
	panel_info.epdc_data.wv_modes.mode_gc32 = 2;

	setup_epdc_power();

	/* Assign fb_base */
	gd->fb_base = CONFIG_FB_BASE;
}

/* 2011/06/09 FY11 : Disable ports for EPD. */
static void remove_epdc(void)
{
	mxc_request_iomux(MX50_PIN_EPDC_D0, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_D1, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_D2, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_D3, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_D4, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_D5, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_D6, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_D7, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_GDCLK, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_GDSP, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_GDOE, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_SDCLK, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_SDOE, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_SDLE, IOMUX_CONFIG_ALT1);
	mxc_request_iomux(MX50_PIN_EPDC_SDCE0, IOMUX_CONFIG_ALT1);

	setup_gpio(GPIO3_BASE_ADDR, 0, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 1, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 2, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 3, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 4, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 5, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 6, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 7, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 16, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 17, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 18, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 20, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 23, GPIO_CFG_LOW);
	setup_gpio(GPIO3_BASE_ADDR, 24, GPIO_CFG_LOW);
	setup_gpio(GPIO4_BASE_ADDR, 25, GPIO_CFG_LOW);
}
#endif


void set_gpio(u32 gpio_base_addr, u32 bit_position, u32 state)
{
	u32 bit_mask = 1<<bit_position;
	u32 dr = readl(gpio_base_addr+GPIO_DR);
	
	if(state)
		dr |= bit_mask;
	else
		dr &= ~bit_mask;
	writel(dr, gpio_base_addr+GPIO_DR);
}

void setup_gpio(u32 gpio_base_addr, u32 bit_position, u32 cfg)
{
	u32 bit_mask = 1<<bit_position;
	u32 gdir = readl(gpio_base_addr + GPIO_GDIR);

	if(cfg==GPIO_CFG_INPUT)
		gdir &= ~bit_mask;
	else
	{
		u32 dr = readl(gpio_base_addr+GPIO_DR);
		if(cfg==GPIO_CFG_HIGH)
			dr |= bit_mask;
		else
			dr &= ~bit_mask;
		writel(dr, gpio_base_addr+GPIO_DR);

		gdir |= bit_mask;
	}
	writel(gdir, gpio_base_addr+GPIO_GDIR);
}

void setup_iomux(void)
{
	const u32 PAD_CTL_PKE_DISABLE=0;

#if 1 /* E_BOOK *//* for POWER3 2011/07/05 */
	mxc_request_iomux(MX50_PIN_CSPI_MOSI, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_CSPI_MOSI, 0x4);
	
	mxc_request_iomux(MX50_PIN_CSPI_MISO, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_CSPI_MISO, 0x4);

	mxc_request_iomux(MX50_PIN_CSPI_SS0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_CSPI_SS0, 0x04); 

	mxc_request_iomux(MX50_PIN_ECSPI1_MOSI,	IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_ECSPI1_MOSI, 0x04); 
#endif
#if 1 /* E_BOOK *//* IOMUX 2011/03/31 */
	mxc_iomux_set_pad(MX50_PIN_PMIC_BOOT_MODE0, PAD_CTL_PKE_DISABLE);
	mxc_iomux_set_pad(MX50_PIN_PMIC_BOOT_MODE1, PAD_CTL_PKE_DISABLE);
#endif
	mxc_request_iomux(MX50_PIN_DISP_RD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_RD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 19, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_SD2_CD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD2_CD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 17, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_PWM1, IOMUX_CONFIG_ALT1);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_PWM1, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PU);
#else
	mxc_iomux_set_pad(MX50_PIN_PWM1, PAD_CTL_PKE_DISABLE);
#endif
	setup_gpio(GPIO6_BASE_ADDR, 24, GPIO_CFG_INPUT);

#if 0 /* E_BOOK *//* delete for LED 2011/02/19 */
	mxc_request_iomux(MX50_PIN_PWM2, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_PWM2, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 25, GPIO_CFG_LOW);
#endif

	mxc_request_iomux(MX50_PIN_DISP_WR, IOMUX_CONFIG_ALT1);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_DISP_WR, PAD_CTL_PKE_DISABLE);
#else
	mxc_iomux_set_pad(MX50_PIN_DISP_WR, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
#endif
#if 1 /* E_BOOK *//* for ET2 2011/04/07 */
	setup_gpio(GPIO2_BASE_ADDR, 16, GPIO_CFG_LOW);
#else
	setup_gpio(GPIO2_BASE_ADDR, 16, GPIO_CFG_HIGH);
#endif

/* 2011/05/08 FY11 : Initialized this port in setup_epdc() */
//	mxc_request_iomux(MX50_PIN_EPDC_D10, IOMUX_CONFIG_ALT1);
//	mxc_iomux_set_pad(MX50_PIN_EPDC_D10, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
//	setup_gpio(GPIO3_BASE_ADDR, 10, GPIO_CFG_INPUT);

#if 0 /* E_BOOK */
	mxc_request_iomux(MX50_PIN_EPDC_D8, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D8, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO3_BASE_ADDR, 8, GPIO_CFG_LOW);
#endif /* E_BOOK */

#if 1 /* E_BOOK *//* for POWER 3 2011/05/23 */
	mxc_request_iomux(MX50_PIN_EPITO, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPITO, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 27, GPIO_CFG_INPUT);
#else
	mxc_request_iomux(MX50_PIN_EPITO, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPITO, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 27, GPIO_CFG_LOW);
#endif

	mxc_request_iomux(MX50_PIN_OWIRE, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX50_PIN_OWIRE, PAD_CTL_PKE_DISABLE);

	mxc_request_iomux(MX50_PIN_ECSPI2_MISO, IOMUX_CONFIG_ALT4);
	mxc_iomux_set_pad(MX50_PIN_ECSPI2_MISO, PAD_CTL_PKE_DISABLE);

	mxc_request_iomux(MX50_PIN_ECSPI2_SS0, IOMUX_CONFIG_ALT4);
#if 1 /* E_BOOK *//* for power 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_ECSPI2_SS0, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PU);
#else
	mxc_iomux_set_pad(MX50_PIN_ECSPI2_SS0, PAD_CTL_PKE_DISABLE);
#endif

	mxc_request_iomux(MX50_PIN_EPDC_D12, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D12, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 12, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_D13, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D13, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 13, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_D14, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D14, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 14, GPIO_CFG_INPUT);
	
	mxc_request_iomux(MX50_PIN_EPDC_D15, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_D15, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 15, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_SDCE2, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDCE2, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 27, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_SDCE3, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDCE3, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 28, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_SDCE4, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDCE4, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 29, GPIO_CFG_INPUT);
	
	mxc_request_iomux(MX50_PIN_EPDC_SDCE5, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDCE5, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 30, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_SDOED, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDOED, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 22, GPIO_CFG_INPUT);

#if 0 /* E_BOOK */
/* Not support WWAN, WLAN */
	mxc_request_iomux(MX50_PIN_EPDC_SDOEZ, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDOEZ, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 21, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_SDOED, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDOED, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 22, GPIO_CFG_INPUT);
#else /* E_BOOK */
/* Support key8-11 GPIO */
	mxc_request_iomux(MX50_PIN_EPDC_PWRCTRL0, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCTRL0, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 29, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_PWRCTRL1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCTRL1, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 30, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_PWRCTRL2, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCTRL2, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 31, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_PWRCTRL3, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCTRL3, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 20, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_VCOM0, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_VCOM0, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 21, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_VCOM1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_VCOM1, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 22, GPIO_CFG_INPUT);

	/* Add for Reader by SONY 2010/12/08 */
	/*
	 * GPIO setting 
	 */
	mxc_request_iomux(MX50_PIN_KEY_COL0, IOMUX_CONFIG_ALT1);
#if 1 /* E_BOOK *//* for POWER3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_KEY_COL0, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PD);
#else
	mxc_iomux_set_pad(MX50_PIN_KEY_COL0, PAD_CTL_PKE_DISABLE);
#endif
	setup_gpio(GPIO4_BASE_ADDR, 0, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_KEY_ROW0, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_KEY_ROW0, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 1, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_KEY_COL1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_KEY_COL1, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 2, GPIO_CFG_HIGH);
	
	mxc_request_iomux(MX50_PIN_KEY_ROW1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_KEY_ROW1, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 3, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_KEY_COL2, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_KEY_COL2, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 4, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_KEY_ROW2, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_KEY_ROW2, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 5, GPIO_CFG_LOW);
	
	mxc_request_iomux(MX50_PIN_KEY_COL3, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_KEY_COL3, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 6, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_KEY_ROW3, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_KEY_ROW3, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 7, GPIO_CFG_LOW);
	
	mxc_request_iomux(MX50_PIN_DISP_RS, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_RS, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 17, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_DISP_CS, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_CS, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO2_BASE_ADDR, 21, GPIO_CFG_INPUT);
	
	mxc_request_iomux(MX50_PIN_DISP_BUSY, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_BUSY, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO2_BASE_ADDR, 18, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_DISP_RESET, IOMUX_CONFIG_ALT1);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_DISP_RESET, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PU);
#else
	mxc_iomux_set_pad(MX50_PIN_DISP_RESET, PAD_CTL_PKE_DISABLE);
#endif
	setup_gpio(GPIO2_BASE_ADDR, 20, GPIO_CFG_INPUT); /* change 2011/07/06 */
	
/* 2011/04/05 FY11 : Initialized this port in setup_epdc() */
//	mxc_request_iomux(MX50_PIN_EPDC_D9, IOMUX_CONFIG_ALT1);
//	mxc_iomux_set_pad(MX50_PIN_EPDC_D9, PAD_CTL_PKE_DISABLE);
//	setup_gpio(GPIO3_BASE_ADDR,  9, GPIO_CFG_LOW);

/* 2011/05/08 FY11 : Initialized this port in setup_epdc() */
//	mxc_request_iomux(MX50_PIN_EPDC_D11, IOMUX_CONFIG_ALT1);
//	mxc_iomux_set_pad(MX50_PIN_EPDC_D11, PAD_CTL_PKE_DISABLE);
//	setup_gpio(GPIO3_BASE_ADDR, 11, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_EPDC_SDCLKN, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDCLKN, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO3_BASE_ADDR, 25, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_EPDC_SDSHR, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDSHR, PAD_CTL_PKE_DISABLE);
#if 1 /* E_BOOK *//* for POWER 3 2011/05/23 */
	setup_gpio(GPIO3_BASE_ADDR, 26, GPIO_CFG_INPUT);
#else
	setup_gpio(GPIO3_BASE_ADDR, 26, GPIO_CFG_LOW);
#endif
#if 1 /* E_BOOK *//* for POWER 3 2011/05/23 */
	mxc_request_iomux(MX50_PIN_EPDC_GDRL, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_GDRL, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO3_BASE_ADDR, 19, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_EPDC_BDR0, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_BDR0, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 23, GPIO_CFG_INPUT);
#endif	

#if 0 /* E_BOOK *//* delete 2011/07/06 */
	mxc_request_iomux(MX50_PIN_EPDC_PWRCTRL0, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCTRL0, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO3_BASE_ADDR, 29, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_PWRCTRL1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCTRL1, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO3_BASE_ADDR, 30, GPIO_CFG_INPUT);
#endif

	mxc_request_iomux(MX50_PIN_EPDC_PWRCTRL2, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCTRL2, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO3_BASE_ADDR, 31, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_PWRCTRL3, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCTRL3, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 20, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_VCOM1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_VCOM1, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
	setup_gpio(GPIO4_BASE_ADDR, 22, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EPDC_SDCE1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_SDCE1, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 26, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_EIM_DA0, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA0, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 0, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA1, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 1, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA2, IOMUX_CONFIG_ALT1);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_EIM_DA2, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PU);
#else
	mxc_iomux_set_pad(MX50_PIN_EIM_DA2, PAD_CTL_PKE_DISABLE);
#endif
	setup_gpio(GPIO1_BASE_ADDR, 2, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA3, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA3, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 3, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA4, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA4, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 4, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA5, IOMUX_CONFIG_ALT1);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_EIM_DA5, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PU);
#else
	mxc_iomux_set_pad(MX50_PIN_EIM_DA5, PAD_CTL_PKE_DISABLE);
#endif
	setup_gpio(GPIO1_BASE_ADDR, 5, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA6, IOMUX_CONFIG_ALT1);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_EIM_DA6, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PU);
#else
	mxc_iomux_set_pad(MX50_PIN_EIM_DA6, PAD_CTL_PKE_DISABLE);
#endif
	setup_gpio(GPIO1_BASE_ADDR, 6, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA7, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA7, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 7, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA7, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA7, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 7, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA8, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA8, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 8, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA9, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA9, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 9, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA10, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA10, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 10, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA11, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA11, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 11, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA12, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA12, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 12, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA13, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA13, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 13, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA14, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA14, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 14, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_DA15, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_DA15, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 15, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_CS2, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_CS2, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 16, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_CS1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_CS1, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 17, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_CS0, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_CS0, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 18, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_EB0, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_EB0, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 19, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_EB1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_EB1, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 20, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_WAIT, IOMUX_CONFIG_ALT1);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_EIM_WAIT, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PU);
#else
	mxc_iomux_set_pad(MX50_PIN_EIM_WAIT, PAD_CTL_PKE_DISABLE);
#endif
	setup_gpio(GPIO1_BASE_ADDR, 21, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_BCLK, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_BCLK, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 22, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_RDY, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_RDY, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 23, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_EIM_OE, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_OE, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 24, GPIO_CFG_LOW); /* change 2011/07/07 */

	mxc_request_iomux(MX50_PIN_EIM_RW, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_RW, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 25, GPIO_CFG_LOW); /* change 2011/07/07 */

	mxc_request_iomux(MX50_PIN_EIM_LBA, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_LBA, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 26, GPIO_CFG_LOW); /* change 2011/07/07 */

	mxc_request_iomux(MX50_PIN_EIM_CRE, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EIM_CRE, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO1_BASE_ADDR, 27, GPIO_CFG_LOW); /* change 2011/07/07 */

	mxc_request_iomux(MX50_PIN_ECSPI1_SCLK, IOMUX_CONFIG_ALT1);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_ECSPI1_SCLK, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PD);
#else
	mxc_iomux_set_pad(MX50_PIN_ECSPI1_SCLK, PAD_CTL_PKE_DISABLE);
#endif
	setup_gpio(GPIO4_BASE_ADDR, 12, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_ECSPI1_SS0, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_ECSPI1_SS0, PAD_CTL_PKE_DISABLE);
#if 1 /* E_BOOK *//* change initial value for signal leak at cold boot 2011/08/23 */
	setup_gpio(GPIO4_BASE_ADDR, 15, GPIO_CFG_LOW);
#else
	setup_gpio(GPIO4_BASE_ADDR, 15, GPIO_CFG_INPUT);
#endif

	mxc_request_iomux(MX50_PIN_ECSPI2_SCLK, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_ECSPI2_SCLK, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 16, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_ECSPI2_MOSI, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_ECSPI2_MOSI, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO4_BASE_ADDR, 17, GPIO_CFG_HIGH);

	mxc_request_iomux(MX50_PIN_SD2_D4, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD2_D4, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 12, GPIO_CFG_HIGH);

	mxc_request_iomux(MX50_PIN_SD2_D5, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD2_D5, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 13, GPIO_CFG_HIGH);

	mxc_request_iomux(MX50_PIN_SD2_D6, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD2_D6, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 14, GPIO_CFG_LOW); /* change 2011/07/07 */

	mxc_request_iomux(MX50_PIN_SD2_D7, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD2_D7, 
			  PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL |
			  PAD_CTL_100K_PU|PAD_CTL_DRV_HIGH);
	setup_gpio(GPIO5_BASE_ADDR, 15, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_SD2_WP, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD2_WP, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 16, GPIO_CFG_HIGH);

	mxc_request_iomux(MX50_PIN_SD2_CD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD2_CD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 17, GPIO_CFG_INPUT);

#if 1 /* E_BOOK *//* for POWER 3 2011/05/23 */
	mxc_request_iomux(MX50_PIN_SD3_D4, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD3_D4, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 24, GPIO_CFG_INPUT);
#else
	mxc_request_iomux(MX50_PIN_SD3_D4, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD3_D4, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PD);
	setup_gpio(GPIO5_BASE_ADDR, 24, GPIO_CFG_HIGH);
#endif

	mxc_request_iomux(MX50_PIN_SD3_D5, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD3_D5, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 25, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_SD3_D6, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD3_D6, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 26, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_SD3_D7, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD3_D7, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 27, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_SD3_WP, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SD3_WP, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO5_BASE_ADDR, 28, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_DISP_D14, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_D14, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 14, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_DISP_D15, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_D15, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 15, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_SSI_RXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_SSI_RXD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 3, GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_UART3_TXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART3_TXD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 14, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_UART3_RXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART3_RXD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 15, GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_UART4_TXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART4_TXD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 16, GPIO_CFG_INPUT);

#if 1 /* E_BOOK *//* for POWER 3 2011/05/23 */
	mxc_request_iomux(MX50_PIN_UART4_RXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART4_RXD, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PD);
	setup_gpio(GPIO6_BASE_ADDR, 17, GPIO_CFG_INPUT);
#else
	mxc_request_iomux(MX50_PIN_UART4_RXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART4_RXD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 17, GPIO_CFG_LOW);
#endif

	/* 
	 * EPDC setting 
	 */
/* 2011/04/05 FY11 : Initialized these ports in setup_epdc(). */
//	mxc_request_iomux(MX50_PIN_EPDC_SDOEZ, IOMUX_CONFIG_ALT0);
//	mxc_iomux_set_pad(MX50_PIN_EPDC_SDOEZ, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);

/* 2011/04/05 FY11 : Initialized these ports in setup_epdc(). */
//	mxc_request_iomux(MX50_PIN_EPDC_PWRCOM, IOMUX_CONFIG_ALT0);
//	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRCOM, PAD_CTL_PKE_DISABLE);
//	mxc_request_iomux(MX50_PIN_EPDC_PWRSTAT, IOMUX_CONFIG_ALT0);
//	mxc_iomux_set_pad(MX50_PIN_EPDC_PWRSTAT, PAD_CTL_PKE_DISABLE);
	
#if 1 /* E_BOOK *//* for POWER 3 2011/05/23 */
	mxc_request_iomux(MX50_PIN_EPDC_BDR1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_EPDC_BDR1, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PD);
	setup_gpio(GPIO4_BASE_ADDR, 24 , GPIO_CFG_INPUT);
#else
	mxc_request_iomux(MX50_PIN_EPDC_BDR1, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_EPDC_BDR1, PAD_CTL_PKE_DISABLE);
#endif
	
	
	/* 
	 * SPI setting 
	 */
	mxc_request_iomux(MX50_PIN_CSPI_SCLK, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_CSPI_SCLK, PAD_CTL_PKE_DISABLE);

#if 1 /* E_BOOK *//* for POWER 3 2011/07/04 */
	mxc_request_iomux(MX50_PIN_ECSPI1_MOSI, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX50_PIN_ECSPI1_MOSI, PAD_CTL_PKE_DISABLE|PAD_CTL_DRV_HIGH);
#else
	mxc_request_iomux(MX50_PIN_ECSPI1_MOSI, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_ECSPI1_MOSI, PAD_CTL_PKE_DISABLE);
#endif

#if 1 /* E_BOOK *//* for POWER 3 2011/05/23 */
	mxc_request_iomux(MX50_PIN_ECSPI1_MISO, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_ECSPI1_MISO, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PD);
	setup_gpio(GPIO4_BASE_ADDR, 14 , GPIO_CFG_INPUT);
#else
	mxc_request_iomux(MX50_PIN_ECSPI1_MISO, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_ECSPI1_MISO, PAD_CTL_PKE_DISABLE);
#endif


	/* 
	 * ESDHC4 setting
	 */
#if 1 /* E_BOOK *//* change to GPIO 2011/07/05 */
	mxc_request_iomux(MX50_PIN_DISP_D8, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_D8, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 8 , GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_DISP_D9, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_D9, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 9 , GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_DISP_D10, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_D10, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 10 , GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_DISP_D11, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_D11, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 11 , GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_DISP_D12, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_D12, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 12 , GPIO_CFG_LOW);

	mxc_request_iomux(MX50_PIN_DISP_D13, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_DISP_D13, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO2_BASE_ADDR, 13 , GPIO_CFG_LOW);
#else
	mxc_request_iomux(MX50_PIN_DISP_D8, IOMUX_CONFIG_ALT0);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_DISP_D8, PAD_CTL_PKE_DISABLE);
#else
	mxc_iomux_set_pad(MX50_PIN_DISP_D8, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
#endif

	mxc_request_iomux(MX50_PIN_DISP_D9, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_DISP_D9, PAD_CTL_PKE_DISABLE);

	mxc_request_iomux(MX50_PIN_DISP_D10, IOMUX_CONFIG_ALT0);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_DISP_D10, PAD_CTL_PKE_DISABLE);
#else
	mxc_iomux_set_pad(MX50_PIN_DISP_D10, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
#endif

	mxc_request_iomux(MX50_PIN_DISP_D11, IOMUX_CONFIG_ALT0);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_DISP_D11, PAD_CTL_PKE_DISABLE);
#else
	mxc_iomux_set_pad(MX50_PIN_DISP_D11, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
#endif

	mxc_request_iomux(MX50_PIN_DISP_D12, IOMUX_CONFIG_ALT0);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_DISP_D12, PAD_CTL_PKE_DISABLE);
#else
	mxc_iomux_set_pad(MX50_PIN_DISP_D12, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
#endif

	mxc_request_iomux(MX50_PIN_DISP_D13, IOMUX_CONFIG_ALT0);
#if 1 /* E_BOOK *//* for POWER 3 2011/06/18 */
	mxc_iomux_set_pad(MX50_PIN_DISP_D13, PAD_CTL_PKE_DISABLE);
#else
	mxc_iomux_set_pad(MX50_PIN_DISP_D13, PAD_CTL_PKE_ENABLE|PAD_CTL_PUE_PULL|PAD_CTL_100K_PU);
#endif
#endif

	/*
	 * SOUND setting
	 */
	mxc_request_iomux(MX50_PIN_SSI_TXFS, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_SSI_TXFS, PAD_CTL_PKE_DISABLE);
	
	mxc_request_iomux(MX50_PIN_SSI_TXC, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_SSI_TXC, PAD_CTL_PKE_DISABLE);
	
	mxc_request_iomux(MX50_PIN_SSI_TXD, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_SSI_TXD, PAD_CTL_PKE_DISABLE);
	
#if 1 /* E_BOOK *//* IOMUX 2011/03/31 */
	/*
	 * UART1 setting
	 */
	mxc_request_iomux(MX50_PIN_UART1_TXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART1_TXD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 6 , GPIO_CFG_HIGH);

	mxc_request_iomux(MX50_PIN_UART1_RXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART1_RXD, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PU);
	setup_gpio(GPIO6_BASE_ADDR, 7 , GPIO_CFG_INPUT);

	mxc_request_iomux(MX50_PIN_UART1_CTS, IOMUX_CONFIG_ALT1);/* IOMUX 2011/03/31 */
	mxc_iomux_set_pad(MX50_PIN_UART1_CTS, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 8 , GPIO_CFG_HIGH);/* IOMUX 2011/03/31 */

	mxc_request_iomux(MX50_PIN_UART1_RTS, IOMUX_CONFIG_ALT1);/* IOMUX 2011/03/31 */
	mxc_iomux_set_pad(MX50_PIN_UART1_RTS, PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_100K_PU);
	setup_gpio(GPIO6_BASE_ADDR, 9 , GPIO_CFG_INPUT);/* IOMUX 2011/03/31 */
#endif
	
	/*
	 * UART2 setting
	 */
#if 1 /* E_BOOK *//* disable UART2 on boot 2011/07/06 */
	mxc_request_iomux(MX50_PIN_UART2_TXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART2_TXD, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 10 , GPIO_CFG_LOW); /* bugfix 2011/08/22 */

	mxc_request_iomux(MX50_PIN_UART2_RXD, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX50_PIN_UART2_RXD, PAD_CTL_PKE_DISABLE); /* change 2011/07/06 */
	setup_gpio(GPIO6_BASE_ADDR, 11 , GPIO_CFG_INPUT);
#else	
	mxc_request_iomux(MX50_PIN_UART2_TXD, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_UART2_TXD, PAD_CTL_PKE_DISABLE);

	mxc_request_iomux(MX50_PIN_UART2_RXD, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_UART2_RXD, PAD_CTL_PKE_DISABLE); /* change 2011/07/06 */
#endif

	mxc_request_iomux(MX50_PIN_UART2_CTS, IOMUX_CONFIG_ALT1);/* IOMUX 2011/03/31 */
	mxc_iomux_set_pad(MX50_PIN_UART2_CTS, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 12 , GPIO_CFG_LOW);/* bugfix 2011/08/22 */

	mxc_request_iomux(MX50_PIN_UART2_RTS, IOMUX_CONFIG_ALT1);/* IOMUX 2011/03/31 */
	mxc_iomux_set_pad(MX50_PIN_UART2_RTS, PAD_CTL_PKE_DISABLE);
	setup_gpio(GPIO6_BASE_ADDR, 13 , GPIO_CFG_INPUT);/* IOMUX 2011/03/31 */

	/* 
	 * I2C setting
	 */
#if 1 /* E_BOOK *//* for POWER 3 2011/06/17 */
	mxc_request_iomux(MX50_PIN_I2C1_SCL, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_I2C1_SCL, PAD_CTL_ODE_OPENDRAIN_ENABLE | PAD_CTL_PKE_DISABLE);

	mxc_request_iomux(MX50_PIN_I2C1_SDA, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_I2C1_SDA, PAD_CTL_ODE_OPENDRAIN_ENABLE | PAD_CTL_PKE_DISABLE);
#else
	mxc_request_iomux(MX50_PIN_I2C1_SCL, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_I2C1_SCL, PAD_CTL_PKE_DISABLE);

	mxc_request_iomux(MX50_PIN_I2C1_SDA, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_I2C1_SDA, PAD_CTL_PKE_DISABLE);
#endif

/* 2011/04/05 FY11 : Initialized these ports in setup_i2c() if CONFIG_I2C_MXC is defined. */
#ifndef	CONFIG_I2C_MXC
	mxc_request_iomux(MX50_PIN_I2C2_SCL, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_I2C2_SCL, PAD_CTL_ODE_OPENDRAIN_ENABLE |PAD_CTL_PKE_DISABLE);

	mxc_request_iomux(MX50_PIN_I2C2_SDA, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_I2C2_SDA, PAD_CTL_ODE_OPENDRAIN_ENABLE |PAD_CTL_PKE_DISABLE);
#endif

	mxc_request_iomux(MX50_PIN_I2C3_SCL, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_I2C3_SCL, PAD_CTL_ODE_OPENDRAIN_ENABLE | PAD_CTL_PKE_DISABLE);

	mxc_request_iomux(MX50_PIN_I2C3_SDA, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX50_PIN_I2C3_SDA, PAD_CTL_ODE_OPENDRAIN_ENABLE | PAD_CTL_PKE_DISABLE);
#endif /* E_BOOK */
}

int board_init(void)
{
#if 1 /* E_BOOK *//* Add for SONY Board 2010/11/16 */
  	setup_hve();
#endif
#ifdef CONFIG_MFG
/* MFG firmware need reset usb to avoid host crash firstly */
#define USBCMD 0x140
	int val = readl(OTG_BASE_ADDR + USBCMD);
	val &= ~0x1; /*RS bit*/
	writel(val, OTG_BASE_ADDR + USBCMD);
#endif
#if 1 /* E_BOOK *//* flash LED 2011/02/19 */
	/* start up blink LED */
	startup_led();
#endif
	/* boot device */
	setup_boot_device();

	/* soc rev */
	setup_soc_rev();

	/* arch id for linux */
#if defined(CONFIG_MX50_RDP)
	gd->bd->bi_arch_number = MACH_TYPE_MX50_RDP;
#elif defined(CONFIG_MX50_ARM2)
	gd->bd->bi_arch_number = MACH_TYPE_MX50_ARM2;
#else
#	error "Unsupported board!"
#endif

	/* boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	/* iomux for uart */
	setup_uart();

#ifdef CONFIG_MXC_FEC
	/* iomux for fec */
	setup_fec();
#endif

#ifdef CONFIG_NAND_GPMI
	setup_gpmi_nand();
#endif

/* 2011/04/05 FY11 : Supported to access to EPDPMIC through I2C. */
#ifdef CONFIG_I2C_MXC
	setup_i2c(CONFIG_SYS_I2C_PORT);
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE );
#endif

#ifdef CONFIG_MXC_EPDC
	setup_epdc();
#endif

	setup_iomux();
	return 0;
}

int board_late_init(void)
{
	return 0;
}

int checkboard(void)
{
#if defined(CONFIG_MX50_RDP)
	printf("Board: MX50 RDP board\n");
#elif defined(CONFIG_MX50_ARM2)
	printf("Board: MX50 ARM2 board\n");
#else
#	error "Unsupported board!"
#endif

	printf("Boot Reason: [");

	switch (__REG(SRC_BASE_ADDR + 0x8)) {
	case 0x0001:
		printf("POR");
		break;
	case 0x0009:
		printf("RST");
		break;
	case 0x0010:
	case 0x0011:
		printf("WDOG");
		break;
	default:
		printf("unknown");
	}
	printf("]\n");

	printf("Boot Device: ");
	switch (get_boot_device()) {
	case WEIM_NOR_BOOT:
		printf("NOR\n");
		break;
	case ONE_NAND_BOOT:
		printf("ONE NAND\n");
		break;
	case PATA_BOOT:
		printf("PATA\n");
		break;
	case SATA_BOOT:
		printf("SATA\n");
		break;
	case I2C_BOOT:
		printf("I2C\n");
		break;
	case SPI_NOR_BOOT:
		printf("SPI NOR\n");
		break;
	case SD_BOOT:
		printf("SD\n");
		break;
	case MMC_BOOT:
		printf("MMC\n");
		break;
	case NAND_BOOT:
		printf("NAND\n");
		break;
	case UNKNOWN_BOOT:
	default:
		printf("UNKNOWN\n");
		break;
	}

	return 0;
}

#if 1 /* E_BOOK */
typedef enum tagGPIOKEY {
	GPIOKEY_HOME,	/* Key5 */
	GPIOKEY_BACK,	/* Key7 */
	GPIOKEY_LEFT,	/* Key8 */
	GPIOKEY_RIGHT,	/* Key10 */
	GPIOKEY_OPTION,	/* Key11 */
	GPIOKEY_MAX
} GPIOKEY;

typedef struct tagGPIOKeyEntry {
	unsigned long	address;
	unsigned int	number;
} GPIOKEYENTRY;

static	GPIOKEYENTRY	keyEntry[GPIOKEY_MAX] = {
	{	GPIO4_BASE_ADDR,	28	},
	{	GPIO4_BASE_ADDR,	30	},
	{	GPIO4_BASE_ADDR,	20	},
	{	GPIO4_BASE_ADDR,	21	},
	{	GPIO3_BASE_ADDR,	31	}
};

static	unsigned int	get_input_gpiokey(void)
{
	unsigned int	key = 0;
	int				mask = 1;
	int				i = 0;

	for(; i < GPIOKEY_MAX; i++, mask <<= 1) {
		unsigned int reg = readl(keyEntry[i].address) & (1 << keyEntry[i].number);
		key |= (reg? 0: mask);
	}

	return key;
}

int is_recovery_boot(void)
{
	unsigned int	key = get_input_gpiokey();
	return (key == ((1 << GPIOKEY_HOME) | (1 << GPIOKEY_OPTION)));
}
#endif /* E_BOOK */
