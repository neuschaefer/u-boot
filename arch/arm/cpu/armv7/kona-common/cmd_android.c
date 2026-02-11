/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 *
 */


/* NOTES:
 * This code disregards the commandline in the environment as well as the ATAGs
 * address specified in the board structure in favor of using the values found
 * in the boot.img header
 */


#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <fdt.h>
#include <mmc.h>
#include "bootimg.h"
#include <image.h>
#include <gpt.h>

#ifdef CONFIG_BRCM_KERNEL_ENTRY_HOOK
extern void kernel_entry_hook(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

static char boot_header[EFI_SECTORSIZE];

static void setup_start_tag (struct boot_img_hdr* header);
static void setup_memory_tags (bd_t *bd);
static void setup_commandline_tag (bd_t *bd, char *commandline);
static void setup_initrd_tag (bd_t *bd, ulong initrd_start, ulong initrd_end);
#ifdef CONFIG_BRCM_DTBLOB_TAG
static void setup_dtblob_tag (void* blob);
#endif
static void setup_end_tag (void);

static struct tag *params;

static void announce_and_cleanup(void)
{
	printf("\nStarting kernel ...\n\n");

#ifdef CONFIG_USB_DEVICE
	{
		extern void udc_disconnect(void);
		udc_disconnect();
	}
#endif
	cleanup_before_linux();
}

int do_android(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	bd_t	*bd = gd->bd;
	int	machid = bd->bi_arch_number;
	void	(*kernel_entry)(int zero, int arch, uint params);

	struct boot_img_hdr* header = (struct boot_img_hdr*) boot_header;

	struct mmc* mmc;

	int header_blk, kernel_blk, ramdisk_blk;

	if (argc != 2)
		return cmd_usage(cmdtp);

	header_blk = simple_strtoul(argv[1], NULL, 16);

	mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);

	if (!mmc) {
		printf("Could not find eMMC (%d)\n", CONFIG_SYS_MMC_ENV_DEV );
		return -1;
	}

	/* Read header into memory */

	mmc->block_dev.block_read(CONFIG_SYS_MMC_ENV_DEV, header_blk, 1, boot_header);

	if (strncmp((char*) header->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
		printf("No Android boot.img Magic Header\n");
		return -1;
	}

	/* Read kernel into memory */

	kernel_blk = header_blk + (header->page_size/EFI_SECTORSIZE);
	mmc->block_dev.block_read(CONFIG_SYS_MMC_ENV_DEV, kernel_blk, (header->kernel_size + EFI_SECTORSIZE - 1)/EFI_SECTORSIZE, (void*) header->kernel_addr);

	/* Read ramdisk into memory */

	ramdisk_blk = kernel_blk + (((header->kernel_size + header->page_size - 1)/header->page_size) * (header->page_size/EFI_SECTORSIZE));
	mmc->block_dev.block_read(CONFIG_SYS_MMC_ENV_DEV, ramdisk_blk, (header->ramdisk_size + EFI_SECTORSIZE - 1)/EFI_SECTORSIZE, (void*) header->ramdisk_addr);

	/* Setup entry address & ATAGs */

	show_boot_progress (15);

	/* Uboot header checking */
	/* TODO: Use the CRC check to determine if krenel is corrupted */
	image_header_t *hdr = (image_header_t *)header->kernel_addr;
	int verify = getenv_yesno ("verify");

	if (!image_check_magic(hdr)) {
		puts ("Bad Magic Number\n");
		show_boot_progress (-15);
		return -1;
	}
	
	if (!image_check_hcrc (hdr)) {
		puts ("Bad Header Checksum\n");
		show_boot_progress (-15);
		return -1;
	}

	image_print_contents ((void*) hdr);

	if (verify) {
		puts ("   Verifying Checksum ... ");
		if (!image_check_dcrc (hdr)) {
			printf ("Bad Data CRC\n");
			show_boot_progress (-15);
			return -1;
		}
		puts ("OK\n");
	}

	// Currently boot.img is using uImage for kernel
	// The Android boot mechanism is expecting a zImage for kernel
	// uImage is zImage with an extra uboot header file of 64 bytes
	// Added in offset to the kernel starting address to boot properly
	kernel_entry = (void (*)(int, int, uint))(header->kernel_addr + 0x40);

	debug ("## Transferring control to Linux (at address %08lx) ...\n",
	       (ulong) kernel_entry);

	setup_start_tag (header);

	setup_memory_tags (bd);

	char *uboot_cmdline = getenv ("bootargs");
	char *commandline = strncat(uboot_cmdline, (const char*) header->cmdline, strlen((const char*) header->cmdline));

	setup_commandline_tag (bd, commandline);

	int load_ramdisk = getenv_yesno ("load_android_ramdisk");

	if (load_ramdisk)
		puts ("Use Android standard ramdisk. \n");
	else
		puts ("Use debug root filesystem. \n");

	if ( load_ramdisk && header->ramdisk_size )
		setup_initrd_tag (bd, header->ramdisk_addr, header->ramdisk_addr + header->ramdisk_size);

#ifdef CONFIG_BRCM_DTBLOB_TAG
#ifdef CONFIG_BRCM_DT_LOADADDR
	setup_dtblob_tag ((void*) CONFIG_BRCM_DT_LOADADDR);
#else
	{
		/* Try to find an environment variable brcm_dt_loadaddr */
		/* Presumably this was allocated in u-boot to avoid hardcoding */
		char *loadaddrstr;
		int loadaddr;
		loadaddrstr = getenv ("brcm_dt_loadaddr");
		if (loadaddrstr == NULL) {
			printf("Env variable brcm_dt_loadaddr not set\n");
		} else {
			loadaddr = (int)simple_strtoul(loadaddrstr, NULL, 16);
			printf("loadaddrstr = \'%s\' loadaddr=0x%x\n", loadaddrstr, loadaddr);
			setup_dtblob_tag ((void*) loadaddr);
		}
	}

#endif
#endif
	setup_end_tag();

	announce_and_cleanup();

#ifdef CONFIG_BRCM_KERNEL_ENTRY_HOOK
	kernel_entry_hook(); /* For low level debug using LEDs for example to track progress without uarts */
#endif

	/* does not return */
	kernel_entry(0, machid, header->tags_addr);

	return 1;
}

static void setup_start_tag (struct boot_img_hdr* header)
{
	params = (struct tag *) header->tags_addr;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}

static void setup_memory_tags (bd_t *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);

		params->u.mem.start = bd->bi_dram[i].start;
		params->u.mem.size = bd->bi_dram[i].size;

		debug ("setup_memory_tags: u.mem.start: 0x%lx\n", (ulong) params->u.mem.start);
		debug ("setup_memory_tags: u.mem.size: 0x%lx\n", (ulong) params->u.mem.size);

		params = tag_next (params);
	}
}

static void setup_commandline_tag (bd_t *bd, char *commandline)
{
	char *p;

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;

	strcpy (params->u.cmdline.cmdline, p);

	params = tag_next (params);
}

static void setup_initrd_tag (bd_t *bd, ulong initrd_start, ulong initrd_end)
{
	/* an ATAG_INITRD node tells the kernel where the compressed
	 * ramdisk can be found. ATAG_RDIMG is a better name, actually.
	 */
	params->hdr.tag = ATAG_INITRD2;
	params->hdr.size = tag_size (tag_initrd);

	params->u.initrd.start = initrd_start;
	params->u.initrd.size = initrd_end - initrd_start;

	params = tag_next (params);
}

#ifdef CONFIG_BRCM_DTBLOB_TAG
void setup_dtblob_tag(void* blob)
{
	struct fdt_header *dt;
	uint32_t size;

	printf("Setting up dt-blob tag ...@0x%x from 0x%x\n", (unsigned int) params, (unsigned int) blob);

	/* check device tree validity */
	dt = (struct fdt_header *)blob;
	if (be32_to_cpu(dt->magic) != FDT_MAGIC) {
		printf("Invalid dt-blob!\n");
		return;
	}

	size = be32_to_cpu(dt->totalsize);

	params->hdr.tag = ATAG_DTBLOB;
	/* size is for tag hd and dt blob */
	params->hdr.size = (size >> 2) + 1 + 2;

	/* copy the embedded blob */
	printf("dt-blob size: %d bytes\n", size);
	memcpy(params->u.blob.blob, (unsigned char *)blob, size);

	params = tag_next (params);
	printf("Done dt-blob tag, 0x%x\n", (unsigned int) params);
}
#endif

static void setup_end_tag (void)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}


U_BOOT_CMD(android, 2, 1, do_android,
	"launch android",
	"[eMMC block] - run android from boot.img in eMMC"
);
