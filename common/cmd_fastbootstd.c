/*
 * Copyright 2008 - 2009 (C) Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
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
 *
 * Part of the rx_handler were copied from the Android project. 
 * Specifically rx command parsing in the  usb_rx_data_complete 
 * function of the file bootable/bootloader/legacy/usbloader/usbloader.c
 *
 * The logical naming of flash comes from the Android project
 * Thse structures and functions that look like fastboot_flash_* 
 * They come from bootable/bootloader/legacy/libboot/flash.c
 *
 * This is their Copyright:
 * 
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <asm/byteorder.h>
#include <common.h>
#include <command.h>
#include <nand.h>
#include <fastboot.h>
#include <image.h>
#include <environment.h>

#include <mmc.h>
#include <sparse_format.h>

/* watdog reset call */
#ifdef CONFIG_BCM59055_WDT
extern void bcm59055_wdt_reset(void);
#endif
/* Use do_reset for fastboot's 'reboot' command */
extern int do_bootm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

/* Use do_nand for fastboot's flash commands */
extern env_t *env_ptr;
/* Use do_env_set and do_env_save to permenantly save data */
int do_env_save (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_env_set ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
/* Use do_bootm and do_go for fastboot's 'boot' command */
int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/* Forward decl */
//static int tx_handler(void);
static int rx_handler (const unsigned char *buffer, unsigned int buffer_size);
static void reset_handler (void);
static int fastboot_oem(const char *cmd);

static struct cmd_fastboot_interface interface = 
{
	.rx_handler            = rx_handler,
	.reset_handler         = reset_handler,
	.product_name          = NULL,
	.serial_no             = NULL,
	.nand_block_size       = 0,
	.transfer_buffer       = (unsigned char *)0xffffffff,
	.transfer_buffer_size  = 0,
};

static unsigned int download_size;
static unsigned int download_bytes;
static unsigned int download_bytes_unpadded;
static unsigned int download_error;
static unsigned int continue_booting;
static unsigned int upload_size;
static unsigned int upload_bytes;
static unsigned int upload_error;

/* To support the Android-style naming of flash */
#define MAX_PTN 64
static fastboot_ptentry ptable[MAX_PTN];
static unsigned int pcount;
static int static_pcount = -1;

/* Helper functions to avoid duplicating string names everywhere */
static inline int memcompare(const char *cmdbuf, const char *token)
{
	return (memcmp)(cmdbuf, token, strlen(token)); 
}
static inline int strcompare(const char *s, const char *token)
{
	return (strncmp(s, token, strlen(token))); 
}

static void reset_handler ()
{
	/* If there was a download going on, bail */
	download_size = 0;
	download_bytes = 0;
	download_bytes_unpadded = 0;
	download_error = 0;
	continue_booting = 0;
	upload_size = 0;
	upload_bytes = 0;
	upload_error = 0;
}

static void fastboot_emmc_flash_gpt(char* response, const char* cmdbuf) {

	struct mmc *mmc;

	unsigned int blk, cnt;
	unsigned int img_addr, img_size;

	img_addr = (unsigned int) interface.transfer_buffer;
	img_size = (download_bytes + (512-1)) & ~(512-1);

	mmc = find_mmc_device(CFG_FASTBOOT_MMC_NO);

	if (!mmc) {
		printf("mmc device not found\n");
		sprintf(response, "FAILmmc device not found");
		return;
	}

#ifndef CONFIG_SKIP_PRIMARY_GPT
	cnt = img_size / 512;
	blk = 0; /* GPT at the start of eMMC */
#else
	cnt = img_size / 512;
	blk = (mmc->capacity - img_size) / 512; /* GPT at the end of eMMC */
#endif /* !CONFIG_SKIP_PRIMARY_GPT */

	printf("Writing GPT'\n");

	if (mmc->block_dev.block_write(CFG_FASTBOOT_MMC_NO, blk, cnt, (void*) img_addr) != cnt) {
		printf("Writing 'gpt' FAILED!\n");
		sprintf(response, "FAIL: Write 'gpt' partition");
		return;
	}

	printf("Writing GPT DONE!\n");
	sprintf(response, "OKAY");

	/* Clear out existing partitions and read new ones from the GPT */
	printf("Reading partitions from GPT\n");
	pcount = 0;
	fastboot_discover_gpt_tables();
}

static void fastboot_emmc_flash_sparse_image(char* response, const char* cmdbuf) {

	struct mmc* mmc;

	unsigned int blk, cnt;
	unsigned int remaining_chunks;

	struct fastboot_ptentry *ptn;

	sparse_header_t* s_header;
	chunk_header_t* c_header;


	mmc = find_mmc_device(CFG_FASTBOOT_MMC_NO);

	if (!mmc) {
		printf("mmc device not found\n");
		sprintf(response, "FAILmmc device not found");
		return;
	}

	ptn = fastboot_flash_find_ptn(cmdbuf + strlen("flash:"));

	if (ptn == 0) {
		printf("Partition %s does not exist\n", cmdbuf + strlen("flash:"));
		sprintf(response, "FAILpartition does not exist");
		return;
	}

	if (download_bytes > ptn->length) {
		printf("Image too large for the partition\n");
		sprintf(response, "FAILimage too large for partition");
		return;
	}

	s_header = (sparse_header_t*) interface.transfer_buffer;

	if (((unsigned long long) s_header->blk_sz * (unsigned long long) s_header->total_blks) > ptn->length) {
		printf("Image too large for the partition\n");
		sprintf(response, "FAILimage too large for partition");
		return;
	}

	printf("Flashing Sparse Image\n");

	remaining_chunks = s_header->total_chunks;
	c_header = (chunk_header_t*) (interface.transfer_buffer + s_header->file_hdr_sz);
	blk = ptn->start;

	while (remaining_chunks)
	{
		switch (c_header->chunk_type) {
		case CHUNK_TYPE_RAW:
			cnt = (((c_header->chunk_sz * s_header->blk_sz) + (512-1)) & ~(512-1)) / 512;
			void* addr = (uint8_t*) c_header + s_header->chunk_hdr_sz;

			int n = mmc->block_dev.block_write(CFG_FASTBOOT_MMC_NO, blk, cnt, addr);

			if (n != cnt) {
				printf("Write failed\n");
				sprintf(response, "FAILmmc failure prevented write");
				return;
			}
			break;

		case CHUNK_TYPE_DONT_CARE:
			// do nothing since this is a don't care chunk
			break;

		default:
			// error
			printf("Unknown chunk type\n");
			sprintf(response, "FAILunknown chunk type in sparse image");
			return;
		}

		blk += (c_header->chunk_sz * s_header->blk_sz) / 512;
		c_header = (chunk_header_t*) ((uint8_t*) c_header + c_header->total_sz);
		remaining_chunks--;
	}

	sprintf(response, "OKAY");

}

static void fastboot_emmc_flash_raw_image(char* response, const char* cmdbuf) {

	struct mmc* mmc;

	unsigned int blk, cnt;
	unsigned int img_addr, img_size;

	struct fastboot_ptentry *ptn;

	mmc = find_mmc_device(CFG_FASTBOOT_MMC_NO);

	if (!mmc) {
		printf("mmc device not found\n");
		sprintf(response, "FAILmmc device not found");
		return;
	}

	ptn = fastboot_flash_find_ptn(cmdbuf + strlen("flash:"));

	if (ptn == 0) {
		printf("Partition %s does not exist\n", cmdbuf + strlen("flash:"));
		sprintf(response, "FAILpartition does not exist");
		return;
	}

	img_addr = (unsigned int) interface.transfer_buffer;
	img_size = (download_bytes + (512-1)) & ~(512-1); 

	if (img_size > ptn->length) {
		printf("Image too large for the partition\n");
		sprintf(response, "FAILimage too large for partition");
		return;
	}

	cnt = img_size/ 512;
	blk = ptn->start;

	printf("Flashing Raw Image\n");
	printf("Writing to partition '%s'\n", ptn->name);

	if (mmc->block_dev.block_write(0, blk, cnt, (void*)img_addr) != cnt) {
		printf("Writing '%s' FAILED!\n", ptn->name);
		sprintf(response, "FAIL: Write '%s' partition", ptn->name);
		return;
	}

	printf("Writing '%s' DONE!\n", ptn->name);
	sprintf(response, "OKAY");
}

static int rx_handler (const unsigned char *buffer, unsigned int buffer_size)
{
	int ret = 1;
#ifdef CONFIG_BCM59055_WDT
	static volatile unsigned int wd=0;
#endif
	/* Use 65 instead of 64
	   null gets dropped  
	   strcpy's need the extra byte */
	char response[65];

	memset((void*) response, 0, 65);

	if (download_size) 
	{
		/* Something to download */

		if (buffer_size)
		{
			/* Handle possible overflow */
			unsigned int transfer_size = 
				download_size - download_bytes;

			if (buffer_size < transfer_size)
				transfer_size = buffer_size;
			
			/* Save the data to the transfer buffer */
			memcpy (interface.transfer_buffer + download_bytes, 
				buffer, transfer_size);

			download_bytes += transfer_size;
			
			/* Check if transfer is done */
			if (download_bytes >= download_size) {
				/* Reset global transfer variable,
				   Keep download_bytes because it will be
				   used in the next possible flashing command */
				download_size = 0;

				if (download_error) {
					/* There was an earlier error */
					sprintf(response, "ERROR");
				} else {
					/* Everything has transferred,
					   send the OK response */
					sprintf(response, "OKAY");
				}
				fastboot_tx_status(response, strlen(response));

				printf ("\ndownloading of %d bytes finished\n",
					download_bytes);
			}

			/* Provide some feedback */
			if (download_bytes &&
			    0 == (download_bytes %
				  (16 * interface.nand_block_size)))
			{
				/* Some feeback that the
				   download is happening */
				if (download_error)
					printf("X");
				else {
					printf(".");
#ifdef CONFIG_BCM59055_WDT
					wd++;
					if (wd == 2000) {
						wd = 0;
						bcm59055_wdt_reset();
						printf("*");
					}
#endif
				}
				if (0 == (download_bytes %
					  (80 * 16 *
					   interface.nand_block_size)))
					printf("\n");
				
			}
		}
		else
		{
			/* Ignore empty buffers */
			printf ("Warning empty download buffer\n");
			printf ("Ignoring\n");
		}
		ret = 0;
	}
	else
	{
		/* A command */

		/* Cast to make compiler happy with string functions */
		const char *cmdbuf = (char *) buffer;

		/* Generic failed response */
		sprintf(response, "FAIL");

		/* reboot 
		   Reboot the board. */

		if(memcompare(cmdbuf, "reboot") == 0) 
		{
			sprintf(response,"OKAY");
			fastboot_tx_status(response, strlen(response));
			udelay (1000000); /* 1 sec */
			
			do_reset (NULL, 0, 0, NULL);
			
			/* This code is unreachable,
			   leave it to make the compiler happy */
			return 0;
		}
		
		/* getvar
		   Get common fastboot variables
		   Board has a chance to handle other variables */
		else if(memcompare(cmdbuf, "getvar:") == 0) 
		{
			strcpy(response,"OKAY");
        
			if(!memcompare(cmdbuf + strlen("getvar:"), "version")) 
			{
				strcpy(response + strlen("OKAY"), FASTBOOT_VERSION);
			} 
			else if(!memcompare(cmdbuf + strlen("getvar:"), "product")) 
			{
				if (interface.product_name) 
					strcpy(response + strlen("OKAY"), interface.product_name);
			
			} else if(!memcompare(cmdbuf + strlen("getvar:"), "serialno")) {
				if (interface.serial_no) 
					strcpy(response + strlen("OKAY"), interface.serial_no);

			} else if(!memcompare(cmdbuf + strlen("getvar:"), "downloadsize")) {
				if (interface.transfer_buffer_size) 
					sprintf(response + strlen("OKAY"), "%08x", interface.transfer_buffer_size);
			} 
			else 
			{
				fastboot_getvar(cmdbuf + strlen("getvar:"), response + strlen("OKAY"));
			}
			ret = 0;

		}

		/* erase
		   Erase a register flash partition
		   Board has to set up flash partitions */

		else if(memcompare(cmdbuf, "erase:") == 0){
			sprintf(response,"FAILerase command not supported");
			ret = 0;
		}

		else if(memcompare(cmdbuf, "oem") == 0){
			ret = fastboot_oem(cmdbuf + strlen("oem") + 1);
			if (ret)
				sprintf(response,"FAIL: Unknown oem command");
			else
				sprintf(response, "OKAY");
		}

		/* download
		   download something .. 
		   What happens to it depends on the next command after data */

		else if(memcompare(cmdbuf, "download:") == 0) {

			/* save the size */
			download_size = simple_strtoul (cmdbuf + strlen("download:"), NULL, 16);
			/* Reset the bytes count, now it is safe */
			download_bytes = 0;
			/* Reset error */
			download_error = 0;

			//printf ("Starting download of %d bytes\n", download_size);

			if (0 == download_size)
			{
				/* bad user input */
				sprintf(response, "FAILdata invalid size");
			}
			else if (download_size > interface.transfer_buffer_size)
			{
				/* set download_size to 0 because this is an error */
				download_size = 0;
				sprintf(response, "FAILdata too large");
			}
			else
			{
				/* The default case, the transfer fits
				   completely in the interface buffer */
				sprintf(response, "DATA%08x", download_size);
			}
			ret = 0;
		}

		/* boot
		   boot what was downloaded

		   WARNING WARNING WARNING

		   This is not what you expect.
		   The fastboot client does its own packaging of the
		   kernel.  The layout is defined in the android header
		   file bootimage.h.  This layeout is copiedlooks like this,

		   **
		   ** +-----------------+
		   ** | boot header     | 1 page
		   ** +-----------------+
		   ** | kernel          | n pages
		   ** +-----------------+
		   ** | ramdisk         | m pages
		   ** +-----------------+
		   ** | second stage    | o pages
		   ** +-----------------+
		   **

		   We only care about the kernel.
		   So we have to jump past a page.

		   What is a page size ?
		   The fastboot client uses 2048

		   The is the default value of

		   CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE

		*/

		else if(memcompare(cmdbuf, "boot") == 0) {
			sprintf(response,"FAILboot command not supported");
			ret = 0;
		}

		/* flash
		   Flash what was downloaded */

		else if(memcompare(cmdbuf, "flash:") == 0) {
			if (download_bytes) {

				/* GPT */
				if (!strcmp(cmdbuf + strlen("flash:"), "gpt"))
					fastboot_emmc_flash_gpt(response, cmdbuf);

				/* Other Partitions */
				else {
					sparse_header_t* s_header = (sparse_header_t*) interface.transfer_buffer;

					if ((s_header->magic == SPARSE_HEADER_MAGIC) && (s_header->major_version == 1))
						fastboot_emmc_flash_sparse_image(response, cmdbuf);
					else
						fastboot_emmc_flash_raw_image(response, cmdbuf);
				}
			}
			else {
				sprintf(response, "FAILno image downloaded");
			}
			ret = 0;
		}

		/* continue
		   Stop doing fastboot */
		else if (memcompare(cmdbuf, "continue") == 0) {
			sprintf(response, "OKAY");
			continue_booting = 1;
			ret = 0;
		}

		/* upload
		   Upload just the data in a partition */
		else if ((memcompare(cmdbuf, "upload:") == 0) ||
		    (memcompare(cmdbuf, "uploadraw:") == 0)) {
				sprintf(response,"FAILupload command not supported");

			ret = 0;
		}

		else
		{
			sprintf(response, "FAILunknown command");
		}

		fastboot_tx_status(response, strlen(response));

	} /* End of command */
	
	return ret;
}

static int check_against_static_partition(struct fastboot_ptentry *ptn)
{
	int ret = 0;
	struct fastboot_ptentry *c;
	int i;

	for (i = 0; i < static_pcount; i++) {
		c = fastboot_flash_get_ptn((unsigned int) i);

		if (0 == ptn->length)
			break;

		if ((ptn->start >= c->start) &&
		    (ptn->start < c->start + c->length))
			break;

		if ((ptn->start + ptn->length > c->start) &&
		    (ptn->start + ptn->length <= c->start + c->length))
			break;

		if ((0 == strcmp(ptn->name, c->name)) &&
		    (0 == strcmp(c->name, ptn->name)))
			break;
	}

	if (i >= static_pcount)
		ret = 1;
	return ret;
}

static unsigned long long memparse(char *ptr, char **retptr)
{
	char *endptr;	/* local pointer to end of parsed string */

	unsigned long ret = simple_strtoul(ptr, &endptr, 0);

	switch (*endptr) {
	case 'M':
	case 'm':
		ret <<= 10;
	case 'K':
	case 'k':
		ret <<= 10;
		endptr++;
	default:
		break;
	}

	if (retptr)
		*retptr = endptr;

	return ret;
}

static int add_partition_from_environment(char *s, char **retptr)
{
	unsigned long size;
	unsigned long offset = 0;
	char *name;
	int name_len;
	int delim;
	unsigned int flags;
	struct fastboot_ptentry part;

	if (!s) {
		printf("Bad pointer\n");
		return 1;
	}

	size = memparse(s, &s);
	if (0 == size) {
		printf("Error:FASTBOOT size of partition is 0\n");
		return 1;
	}

	/* fetch partition name and flags */
	flags = 0; /* this is going to be a regular partition */
	delim = 0;
	/* check for offset */
	if (*s == '@') {
		s++;
		offset = memparse(s, &s);
	} else {
		printf("Error:FASTBOOT offset of partition is not given\n");
		return 1;
	}

	/* now look for name */
	if (*s == '(')
		delim = ')';

	if (delim) {
		char *p;

		name = ++s;
		p = strchr((const char *)name, delim);
		if (!p) {
			printf("Error:FASTBOOT no closing %c found in partition name\n", delim);
			return 1;
		}
		name_len = p - name;
		s = p + 1;
	} else {
		printf("Error:FASTBOOT no partition name for \'%s\'\n", s);
		return 1;
	}

	/* test for options */
	while (1) {
		if (strcompare(s, "i") == 0) {
			flags |= FASTBOOT_PTENTRY_FLAGS_WRITE_I;
			s += strlen("i");
		} else if (strcompare(s, "yaffs") == 0) {
			/* yaffs */
			flags |= FASTBOOT_PTENTRY_FLAGS_WRITE_YAFFS;
			s += strlen("yaffs");
		} else if (strcompare(s, "swecc") == 0) {
			/* swecc */
			flags |= FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC;
			s += strlen("swecc");
		} else if (strcompare(s, "hwecc") == 0) {
			/* hwecc */
			flags |= FASTBOOT_PTENTRY_FLAGS_WRITE_HW_ECC;
			s += strlen("hwecc");
		} else {
			break;
		}
		if (strcompare(s, "|") == 0)
			s += strlen("|");
	}

	/* enter this partition (offset will be calculated later if it is zero at this point) */
	part.length = size;
	part.start = offset;
	part.flags = flags;

	if (name) {
		if (name_len >= sizeof(part.name)) {
			printf("Error:FASTBOOT partition name is too long\n");
			return 1;
		}
		strncpy(&part.name[0], name, name_len);
		/* name is not null terminated */
		part.name[name_len] = '\0';
	} else {
		printf("Error:FASTBOOT no name\n");
		return 1;
	}


	/* Check if this overlaps a static partition */
	if (check_against_static_partition(&part)) {
		printf("Adding: %s, offset 0x%8.8x, size 0x%16.16llx, flags 0x%8.8x\n",
		       part.name, part.start, part.length, part.flags);
		fastboot_flash_add_ptn(&part);
	}

	/* return (updated) pointer command line string */
	*retptr = s;

	/* return partition table */
	return 0;
}



int do_fastboot_std (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 1;
	char fbparts[4096], *env;
	int check_timeout = 0;
	//uint64_t timeout_endtime = 0;
	//uint64_t timeout_ticks = 0;
	long timeout_seconds = -1;
	//int continue_from_disconnect = 0;

#ifdef CONFIG_BCM59055_WDT
	/* reset WDT once loop enter fastboot
	 * 127sec time to start flashing
	 * next reset will happen while flashing */
	bcm59055_wdt_reset();
#endif
#ifdef CONFIG_FASTBOOT_GPT_TABLE_IN_FLASH 
	/* Use GPT table already in flash to build data structures */
	fastboot_discover_gpt_tables();
#endif

	/*
	 * Place the runtime partitions at the end of the
	 * static partitions.  First save the start off so
	 * it can be saved from run to run.
	 */
	if (static_pcount >= 0) {
		/* Reset */
		pcount = static_pcount;
	} else {
		/* Save */
		static_pcount = pcount;
	}
	env = getenv("fbparts");
	if (env) {
		unsigned int len;
		len = strlen(env);
		if (len && len < 4096) {
			char *s, *e;

			memcpy(&fbparts[0], env, len + 1);
			printf("Fastboot: Adding partitions from environment\n");
			s = &fbparts[0];
			e = s + len;
			while (s < e) {
				if (add_partition_from_environment(s, &s)) {
					printf("Error:Fastboot: Abort adding partitions\n");
					/* reset back to static */
					pcount = static_pcount;
					break;
				}
				/* Skip a bunch of delimiters */
				while (s < e) {
					if ((' ' == *s) ||
					    ('\t' == *s) ||
					    ('\n' == *s) ||
					    ('\r' == *s) ||
					    (',' == *s)) {
						s++;
					} else {
						break;
					}
				}
			}
		}
	}

	/* Time out */
	if (2 == argc) {
		long try_seconds;
		char *try_seconds_end;
		/* Check for timeout */
		try_seconds = simple_strtol(argv[1],
					    &try_seconds_end, 10);
		if ((try_seconds_end != argv[1]) &&
		    (try_seconds >= 0)) {
			check_timeout = 1;
			timeout_seconds = try_seconds;
			printf("Fastboot inactivity timeout %ld seconds\n", timeout_seconds);
		}
	}
	fastboot_init(&interface);

	while(1) {
#ifdef CONFIG_BCM59055_WDT
		static int cnt = 0;
#endif
		fastboot_poll();
#ifdef CONFIG_BCM59055_WDT
		cnt++;
		if (cnt == 1000000) {
			cnt = 0;
			bcm59055_wdt_reset();
		}
#endif
		if (continue_booting) {
			run_command(getenv("bootcmd"),0);
			break;
		}
	}

	return ret;
}

U_BOOT_CMD(
	fastboot_std,	2,	1,	do_fastboot_std,
	"fastboot- use standard Android USB Fastboot protocol\n",
	"[inactive timeout]\n"
	"    - Run as a fastboot usb device.\n"
	"    - The optional inactive timeout is the decimal seconds before\n"
	"    - the normal console resumes\n"
);


static int fastboot_oem(const char *cmd)
{
	if (!strcompare(cmd,"broadcom"))
	{
		setenv("boot_mode","broadcom");
		saveenv();
	}
	else if (!strcompare(cmd,"android"))
	{
		setenv("boot_mode","android");
		saveenv();
	}
	else if (!strcompare(cmd,"defaultenv"))
	{
		set_default_env(0);
		saveenv();
	}
#ifdef CONFIG_FASTBOOT_BUILD_GPT
	else if (!strcompare(cmd,"format"))
	{
		fastboot_format(cmd + strlen("format") + 1);
	}
#endif

	else if (!strcompare(cmd,"wipe"))
	{
		mmc_wipe(0);
	}

	else
	{
		return -1;
	}

	return 0;
}
