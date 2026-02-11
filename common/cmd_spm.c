/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

/* Capri software performance monitor */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/brcm_rdb_sysmap.h>

#define INTERVAL_OFFSET 8
#define ENABLE_OFFSET 0
#define MONITOR_VALUE_0_OFFSET 0x44

static unsigned long spmtotal = 0;
static unsigned long spmval = 0;
static unsigned long spmi = 0;
static unsigned long usecs = 500000;
static int initialized = 0;

void spmInit(void)
{
	/* Setup interval registers (in 32KHz clocks) */
	writel(9600, VARSPM_BASE_ADDR + INTERVAL_OFFSET);
	writel(9600, SPM_BASE_ADDR + INTERVAL_OFFSET);
	initialized = 1;
}

void spmStart(void)
{
	/* Enable the silicon performance monitors */
	writel(1, VARSPM_BASE_ADDR + ENABLE_OFFSET);
	writel(1, SPM_BASE_ADDR + ENABLE_OFFSET);
}

void spmStop(void)
{
	/* Disable the silicon performance monitors */
	writel(0, VARSPM_BASE_ADDR + ENABLE_OFFSET);
	writel(0, SPM_BASE_ADDR + ENABLE_OFFSET);
}

void spmShow(void)
{
	printf("SPM samples: %lu usec window\n", usecs);
	printf("VARSPM: ");
	spmtotal = 0;
	spmval = 0;
	spmi = 0;

	/* Obtain and sum each of the 6 silicon performance monitors */
	for (spmi = 0; spmi < 6; spmi++) {
		spmval =
		    readl((VARSPM_BASE_ADDR + MONITOR_VALUE_0_OFFSET +
			   (spmi * 4)));
		spmtotal += spmval;
		printf("0x%lx ", spmval);
	}
	printf("\n");

	printf("SPM   : ");

	/* Obtain and sum each of the 6 silicon performance monitors */
	for (spmi = 0; spmi < 6; spmi++) {
		spmval =
		    readl((SPM_BASE_ADDR + MONITOR_VALUE_0_OFFSET +
			   (spmi * 4)));
		spmtotal += spmval;
		printf("0x%lx ", spmval);
	}
	printf("\n");

	printf("Total: 0x%lx = %ld\n", spmtotal, spmtotal);
}

static int do_spm(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	if (!initialized) {
		spmInit();
		initialized = 1;
	}

	if (argc == 2) {
		if (!strcmp(argv[1], "show")) {
			spmStart();
			udelay(usecs);
			spmShow();
			spmStop();
		}
		return 0;
	} else if (argc == 3) {
		if (!strcmp(argv[1], "usec")) {
			usecs = simple_strtoul(argv[2], NULL, 0);
			printf("%lu usec window\n", usecs);
		} else {
			return cmd_usage(cmdtp);
		}
	}
	else {
		return cmd_usage(cmdtp);
	}

	return 0;
}

U_BOOT_CMD(
	spm, 3, 0, do_spm,
	"software performance metrics",
	"show\n"
	"spm usec <usecs>");
