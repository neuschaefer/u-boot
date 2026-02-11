/*
 *  linux/drivers/mmc/host/bcm2708_mci.c - Broadcom BCM2708 MCI driver
 *
 *  Copyright (C) 2010 Broadcom, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

struct clk;

#define BCM2708_MCI_COMMAND	0x00

#define BCM2708_MCI_READ		(1 << 6)
#define BCM2708_MCI_WRITE		(1 << 7)
#define BCM2708_MCI_LONGRESP		(1 << 9)
#define BCM2708_MCI_NORESP		(1 << 10)
#define BCM2708_MCI_BUSY		(1 << 11)
#define BCM2708_MCI_ENABLE		(1 << 15)

#define BCM2708_MCI_ARGUMENT	0x04

#define BCM2708_MCI_TIMEOUT	0x08
#define BCM2708_MCI_CDIV 0x0c

#define BCM2708_MCI_RESPONSE0	0x10
#define BCM2708_MCI_RESPONSE1	0x14
#define BCM2708_MCI_RESPONSE2	0x18
#define BCM2708_MCI_RESPONSE3	0x1c

#define BCM2708_MCI_STATUS	0x20
#define BCM2708_MCI_VDD 0x30
#define BCM2708_MCI_VDD_ENABLE	(1 << 0)

#define BCM2708_MCI_EDM	0x34

#define BCM2708_MCI_HCFG 0x38

#define BCM2708_MCI_HCFG_WIDE_INT_BUS 0x2
#define BCM2708_MCI_HCFG_WIDEEXT_4BIT 0x4
#define BCM2708_MCI_HCFG_SLOW_CARD 0x8
#define BCM2708_MCI_HCFG_BLOCK_IRPT_EN (1<<8)
#define BCM2708_MCI_HCFG_BUSY_IRPT_EN (1<<10)
#define BCM2708_MCI_HCFG_WIDEEXT_CLR 0xFFFFFFFB

#define BCM2708_MCI_DATAFLAG	(1 << 0)
#define BCM2708_MCI_CMDTIMEOUT	(1 << 6)
#define BCM2708_MCI_FIFOERR	(1 << 3)
#define BCM2708_MCI_CRC7ERR	(1 << 4)
#define BCM2708_MCI_BUSYCLR	(1 <<10)

#define BCM2708_MCI_HBCT	0x3c
#define BCM2708_MCI_DATA	0x40
#define BCM2708_MCI_HBLC	0x50



#define NR_SG		16

typedef struct bulk_data_struct
{
   unsigned long info;
   unsigned long src;
   unsigned long dst;
   unsigned long length;
   unsigned long stride;
   unsigned long next;
   unsigned long pad[2];
} BCM2708_DMA_CB_T;

