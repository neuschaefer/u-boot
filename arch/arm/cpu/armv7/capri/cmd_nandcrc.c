/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <common.h>
//#include <linux/mtd/mtd.h>
#include <command.h>
#include <malloc.h>
//#include <asm/byteorder.h>
#include <nand.h>

int do_nandcrc(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	char *cmd;
	nand_info_t *nand;
	int dev = nand_curr_device;
	u_char *datbuf, *oobbuf, *p;

	/* at least four arguments please */
	if (argc < 4)
		goto usage;

	cmd = argv[1];

	nand = &nand_info[dev];

	/*
	 * Syntax is:
	 *   0       1    2   3    4
	 *   nandcrc dump off size [pages]
	 */
	if (strcmp(cmd, "dump") == 0 ) {
		int pages = argc > 4 && !strcmp("pages", argv[4]);
		loff_t off = simple_strtoull(argv[2], NULL, 16);
		loff_t size = simple_strtoull(argv[3], NULL, 16);
		int running_crc = 0;;
		int blknum = 0;

		//printf("pages = %d\n", pages);
		//printf("off = %llx\n", off);
		//printf("size = %llx\n", size);

		datbuf = malloc(nand->writesize + nand->oobsize);
		oobbuf = malloc(nand->oobsize);
		if (!datbuf || !oobbuf) {
			puts("No memory for page buffer\n");
			return 1;
		}

		off &= ~(nand->erasesize - 1);
		blknum = off / nand->erasesize;
		loff_t addr = (loff_t) off;
		struct mtd_oob_ops ops;
		memset(&ops, 0, sizeof(ops));
		ops.datbuf = datbuf;
		ops.oobbuf = oobbuf; /* must exist, but oob data will be appended to ops.datbuf */
		ops.len = nand->writesize;
		ops.ooblen = nand->oobsize;
		ops.mode = MTD_OOB_RAW;

		while (size > 0) {
			/* Do a block */
			int block_crc = 0;;
			int page_crc;
			int i;
			if (nand_block_isbad(nand, addr)) {
				printf("BAD BLOCK at address %08llx\n", addr);
				addr += nand->erasesize;
				continue;
			}
			for (i=0; i<nand->erasesize/nand->writesize; i++) {
				int rc = nand->read_oob(nand, addr, &ops);
				if (rc < 0) {
					printf("Error (%d) reading page %08llx\n", rc, off);
					free(datbuf);
					free(oobbuf);
					return 1;
				}
				p = datbuf;
				if (pages) {
					int j;
					page_crc = crc32(0, p, nand->writesize + nand->oobsize);
					printf("page 0x%08x  ", page_crc);
					for (j = 0; j < 16; j++) {
						printf("%02x ", *(p + j));
					}
					printf("\n");
				}	
				block_crc = crc32(block_crc, p, nand->writesize + nand->oobsize);
				running_crc = crc32(running_crc, p, nand->writesize + nand->oobsize);
				size -= nand->writesize;
				addr += nand->writesize;
			}
			printf("0x%08x block_crc 0x%08x  flash_crc 0x%08x\n", blknum++, block_crc, running_crc);
		}

		free(datbuf);
		free(oobbuf);
		return ret;
	}

usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	nandcrc, CONFIG_SYS_MAXARGS, 1, do_nandcrc,
	"dump crcs of nand pages/blocks",
	"dump off size [pages] - calculate crc of flash region plus optional page crcs\n"
);

