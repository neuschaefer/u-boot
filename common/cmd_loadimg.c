/*
 * (C) Copyright 2011
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
#include <command.h>
#include <mmc.h>

#ifdef CONFIG_CMD_LOADIMG

#ifdef CONFIG_ISLAND
#define VC_IMG_ADDR					0x6000
#define VC_IMG_SIZE					0x2000
#define KERN_IMG_ADDR				0x20000
#define RCV_KERN_IMG_ADDR			0x22800
#define RCV_RAM_IMG_ADDR			0x27000
#define BRCM_RAM_IMG_ADDR			0x29800
#define ANDROID_RAM_IMG_ADDR		0x25000
#define DT_IMG_ADDR					0xe00
#endif

#ifdef CONFIG_RHEA
#define VC_IMG_ADDR					0x6000
#define VC_IMG_SIZE					0x2000
#define KERN_IMG_ADDR				0x12000
#define RCV_KERN_IMG_ADDR			0x12000
#define RCV_RAM_IMG_ADDR			0x19000
#define BRCM_RAM_IMG_ADDR			0x1b800
#define ANDROID_RAM_IMG_ADDR		0x17000
#define DT_IMG_ADDR					0x11800
#endif

int do_loadimg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 3)
		return cmd_usage(cmdtp);

	int dev = simple_strtoul(argv[2], NULL, 10);
	void *addr;
	char *s, *tmp;
	u32 cnt, n, blk;
	char str[sizeof(unsigned long) + 1];

	struct mmc *mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("\n ERROR: No mmc device found \n");
		return 1;
	}
	if (strcmp(argv[1], "vc") == 0) {
		if((s = getenv("vc_loadaddr"))== NULL) {
			printf("\n ERROR: vc_loadaddr is not set \n");
			return 1;
		}
		addr = (void *)simple_strtoul(s, NULL, 16);
		cnt = VC_IMG_SIZE;
		blk = VC_IMG_ADDR;

		printf("Loading vc_firmware ...\n");
		n = mmc->block_dev.block_read(dev, blk, cnt, addr);

		/* flush cache after read */
		flush_cache((ulong)addr, cnt * 512); /* FIXME */

		printf("MMC read: dev # %d, block # %d, count %d\n", dev, blk, n);
		return (n == cnt) ? 0 : 1;
	} else if (strcmp(argv[1], "kern") == 0) {
		sprintf(str, "%x", 0);
		setenv("ramdisk_size", str);

		if((s = getenv("boot_mode"))== NULL) {
			printf("\n ERROR: boot_mode is not set \n");
			return 1;
		}

		if((tmp = getenv("loadaddr"))== NULL) {
			printf("\n ERROR: loadaddr is not set \n");
			return 1;
		}
		addr = (void *)simple_strtoul(tmp, NULL, 16);

		if (strcmp(s, "recovery") == 0) {
			if((s = getenv("loadaddr"))== NULL) {
				printf("\n ERROR: loadaddr is not set \n");
				return 1;
			}
			blk = RCV_KERN_IMG_ADDR;

			printf("Loading Recovery kernel ...\n");
			n = mmc->block_dev.block_read(dev, blk, 1, addr);
			cnt = (image_get_image_size((const image_header_t*) addr) + 511)/512;
			n = mmc->block_dev.block_read(dev, blk, cnt, addr);
			/* flush cache after read */
			flush_cache((ulong)addr, cnt * 512); /* FIXME */
			blk = RCV_RAM_IMG_ADDR;
			printf("MMC read: dev # %d, block # %d, count %d\n", dev, blk, n);

			printf("Loading Recovery Ramdisk ...\n");
		} else {
			blk = KERN_IMG_ADDR;
			printf("Loading kernel ...\n");
			n = mmc->block_dev.block_read(dev, blk, 1, addr);
			cnt = (image_get_image_size((const image_header_t*) addr) + 511)/512;
			n = mmc->block_dev.block_read(dev, blk, cnt, addr);
			/* flush cache after read */
			flush_cache((ulong)addr, cnt * 512); /* FIXME */
			printf("MMC read: dev # %d, block # %d, count %d\n", dev, blk, n);

			if (strcmp(s, "broadcom") == 0) {
				blk = BRCM_RAM_IMG_ADDR;
				printf("Loading BRCM Ramdisk ...\n");
			}
			else if (strcmp(s, "android") == 0) {
				blk = ANDROID_RAM_IMG_ADDR;
				printf("Loading Android Ramdisk ...\n");
			}
			else
				blk = 0;
		}
		if((s = getenv("ramdisk_loadaddr"))== NULL) {
			printf("\n ERROR: ramdisk_loadaddr is not set \n");
			return 1;
		}
		addr = (void *)simple_strtoul(s, NULL, 16);
		n = mmc->block_dev.block_read(dev, blk, 1, addr);

		cnt = (image_get_image_size((const image_header_t*) addr) + 511)/512;
		n = mmc->block_dev.block_read(dev, blk, cnt, addr);
		/* flush cache after read */
		flush_cache((ulong)addr, cnt * 512); /* FIXME */
		printf("MMC read: dev # %d, block # %d, count %d\n", dev, blk, n);

		sprintf(str, "%x", cnt);
		setenv("ramdisk_size", str);

		return (n == cnt) ? 0 : 1;
	} else if (strcmp(argv[1], "dt") == 0) {
		if((s = getenv("brcm_dt_enable"))== NULL) {
			printf("\n ERROR: brcm_dt_loadaddr is not set \n");
			return 1;
		}
		if(strcmp(s, "yes") != 0) {
			printf("ERROR: brcm_dt_enable is not set to yes\n");
			return 1;
		}

		if((s = getenv("brcm_dt_loadaddr"))== NULL) {
			printf("\n ERROR: brcm_dt_loadaddr is not set \n");
			return 1;
		}
		addr = (void *)simple_strtoul(s, NULL, 16);
		if((s = getenv("brcm_dt_size"))== NULL) {
			printf("\n ERROR: brcm_dt_size is not set \n");
			return 1;
		}
		cnt = simple_strtoul(s, NULL, 16);
		blk = DT_IMG_ADDR;

		printf("Loading DT Blob ...\n");
		n = mmc->block_dev.block_read(dev, blk, cnt, addr);

		/* flush cache after read */
		flush_cache((ulong)addr, cnt * 512); /* FIXME */

		printf("MMC read: dev # %d, block # %d, count %d\n", dev, blk, n);
		return (n == cnt) ? 0 : 1;
	}
	else if(strcmp(argv[1], "cp") == 0) {
		printf("ERROR: not supported\n");
		return 1;
	} else {
		return cmd_usage(cmdtp);
	}
}

U_BOOT_CMD(
	loadimg, 3, 1, do_loadimg,
	"Load Broadcom vc firmware, kernel. DT blob or cp images",
	"vc <dev num> - load vc firmware image\n"
	"loadimg kern <dev num> - load kernel images\n"
	"loadimg dt <dev num> - load dt blob images\n"
	"loadimg cp <dev num> - load cp images\n"
);

#endif
