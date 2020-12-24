/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 NAND flash driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */

/*
 * (C) Copyright 2006 Detlev Zundel, dzu@denx.de
 * (C) Copyright 2006 DENX Software Engineering
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

#if (CONFIG_COMMANDS & CFG_CMD_NAND)

#include <nand.h>
#include <linux/mtd/nand.h>
#include "com_defs.h"
#include "wpcm450_gctrl_regs.h"


/* These really don't belong here, as they are specific to the NAND Model */
static uint8_t scan_ff_pattern[] = {0xff, 0xff};

#ifdef CONFIG_WPCM450_SVB
static struct nand_bbt_descr wpcm450_badblock_pattern = {
	.options = 0,
	.offs = 5,
	.len = 1,
	.pattern = scan_ff_pattern
};
#else
#ifdef CONFIG_WPCM450_WHOVILLE
static struct nand_bbt_descr wpcm450_badblock_pattern = {
	.options = 0,
	.offs = 0,
	.len = 2,
	.pattern = scan_ff_pattern
};
#endif
#endif


static uint8_t bbt_pattern[] = {'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr wpcm450_bbt_main_descr = {
	.options = NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	8,
	.len = 4,
	.veroffs = 12,
	.maxblocks = 4,
	.pattern = bbt_pattern
};

static struct nand_bbt_descr wpcm450_bbt_mirror_descr = {
	.options = NAND_BBT_CREATE | NAND_BBT_WRITE
		| NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs =	8,
	.len = 4,
	.veroffs = 12,
	.maxblocks = 4,
	.pattern = mirror_pattern
};


/* hardwarespecific function for accesing device ready/busy line */
int wpcm450_dev_ready(struct mtd_info *mtd)
{
#ifdef CONFIG_WPCM450_SVB
    if (IS_BIT_SET(GP2DIN, GPIO35))
#else
#ifdef CONFIG_WPCM450_WHOVILLE
    if (IS_BIT_SET(GP0DIN, GPIOE8))
#endif
#endif
    {
        /* printf("NAND is ready!\n"); */
        return 1;
    }
    else
    {
        /* printf("NAND is busy!\n"); */
        return 0;
    }
}


/*
 *	hardware specific access to control-lines
 */
static void wpcm450_hwcontrol(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *this = mtd->priv;
    
	switch(cmd) {
	case NAND_CTL_SETCLE:
#ifdef CONFIG_WPCM450_SVB
		this->IO_ADDR_W += 1;
#else
#ifdef CONFIG_WPCM450_WHOVILLE
		this->IO_ADDR_W += 64;
#endif
#endif
		break;
		
	case NAND_CTL_CLRCLE:
#ifdef CONFIG_WPCM450_SVB
		this->IO_ADDR_W -= 1;
#else
#ifdef CONFIG_WPCM450_WHOVILLE
		this->IO_ADDR_W -= 64;
#endif
#endif
		break;
		
	case NAND_CTL_SETALE:
#ifdef CONFIG_WPCM450_SVB
		this->IO_ADDR_W += 2;
#else
#ifdef CONFIG_WPCM450_WHOVILLE
		this->IO_ADDR_W += 32;
#endif
#endif
		break;
		
	case NAND_CTL_CLRALE:
#ifdef CONFIG_WPCM450_SVB
		this->IO_ADDR_W -= 2;
#else
#ifdef CONFIG_WPCM450_WHOVILLE
		this->IO_ADDR_W -= 32;
#endif
#endif
		break;
		
	case NAND_CTL_SETNCE:
	case NAND_CTL_CLRNCE:
		/* nop */
		break;
		
#ifdef CONFIG_WPCM450_WHOVILLE
	case NAND_CTL_SETWP:
	    printf("NAND flash is write protected\n");
	    CLEAR_BIT(GP2DOUT, GPIO39); /* drive low */
	    break;
	    
	case NAND_CTL_CLRWP:
	    printf("NAND flash is NOT write protected\n");
		SET_BIT(GP2DOUT, GPIO39);   /* drive high */
		break;
#endif
		
	}
}


/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific (per include/linux/mtd/nand.h):
 * - IO_ADDR_R?: address to read the 8 I/O lines of the flash device
 * - IO_ADDR_W?: address to write the 8 I/O lines of the flash device
 * - hwcontrol: hardwarespecific function for accesing control-lines
 * - dev_ready: hardwarespecific function for  accesing device ready/busy line
 * - enable_hwecc?: function to enable (reset)  hardware ecc generator. Must
 *   only be provided if a hardware ECC is available
 * - eccmode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 * Members with a "?" were not set in the merged testing-NAND branch,
 * so they are not set here either.
 */
int board_nand_init(struct nand_chip *nand)
{
    /* enable nXCS2 */
    SET_BIT(MFSEL1, MF_XCS2SEL);
    
    /* X-Bus timing: 0x7FF77FFF is lowest, 0 is highest */ 
    
#ifdef CONFIG_WPCM450_SVB
    /* setup X-Bus timing */
	XBCR = 0;
	
	/* initiate ready/busy pin */
	CLEAR_BIT(MFSEL1, MF_XCS1SEL);  /* select GPIO */
	CLEAR_BIT(GP2CFG0, GPIO35);     /* input */
#else
#ifdef CONFIG_WPCM450_WHOVILLE
    /* setup X-Bus timing */
	XBCR = 0;
	
	/* initiate ready/busy pin */
	CLEAR_BIT(GP0CFG0, GPIOE8);      /* input */
	
	/* initiate write protection pin */
	CLEAR_BIT(GP2CFG1, GPIO39);     /* push-pull */
	CLEAR_BIT(GP2DOUT, GPIO39);     /* drive low */
	SET_BIT(GP2CFG0, GPIO39);       /* output */
#endif
#endif
    
    /* delay for waiting signal stable */
    udelay(1000);
    
    nand->dev_ready = wpcm450_dev_ready;
    
	nand->hwcontrol = wpcm450_hwcontrol;
	nand->eccmode = NAND_ECC_SOFT;
	/* nand->eccmode = NAND_ECC_NONE; */
	
	/* use a flash based bad block table */
	nand->options = NAND_USE_FLASH_BBT;
	
	nand->bbt_td = &wpcm450_bbt_main_descr;
	nand->bbt_md = &wpcm450_bbt_mirror_descr;
	nand->badblock_pattern = &wpcm450_badblock_pattern;
	
#ifdef CONFIG_WPCM450_SVB
	nand->chip_delay = 12;
#else
#ifdef CONFIG_WPCM450_WHOVILLE
	nand->chip_delay = 25;
#endif
#endif
	
/*	nand->options = NAND_SAMSUNG_LP_OPTIONS;*/
	return 0;
}
#endif /* (CONFIG_COMMANDS & CFG_CMD_NAND) */
