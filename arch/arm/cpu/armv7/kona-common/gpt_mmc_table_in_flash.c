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
#include <common.h>
#include <mmc.h>
#include <fastboot.h>
// #include <efi.h>
#include <gpt.h>

/* GPT table - malloc this in future? */
static efi_ptable gpt;

/* 
 * Check if the header is valid 
 * Returns 1 if valid, 0 if invalid. 
 */
int gpt_is_valid_hdr(efi_gpt_header_t *hdr)
{
	u32 n;
	u32 origcrc;
	u32 hdrsize = le32_to_cpu(hdr->header_size);
	/* Check the GUID Partition Table signature */
	if (le64_to_cpu(hdr->signature) != GPT_HEADER_SIGNATURE) {
		debug("%s: GPT Header signature is wrong 0x%llx != 0x%llx\n", __func__, 
			le64_to_cpu(hdr->signature), GPT_HEADER_SIGNATURE);
		return 0;
	}
	/* Check the CRCs */
	origcrc = le32_to_cpu(hdr->header_crc32);	/* Save original crc */
	memset(&hdr->header_crc32, 0, sizeof(hdr->header_crc32));  /* zero crc field as per spec */
	n = crc32(0, (void*) hdr, hdrsize);
	if (n != origcrc) {
		debug("%s: GPT Header CRC is wrong 0x%x != 0x%x\n", __func__, n, origcrc);
		return 0;
	}
	return 1;
}
/* 
 * Check if the entries array is  valid 
 * Returns 1 if valid, 0 if invalid. 
*/
int gpt_is_valid_entries(efi_gpt_header_t *hdr, u32 numEntries, efi_gpt_entry_t *entry)
{
	u32 n;
	n = crc32(0, (void*) entry, sizeof(*entry) * numEntries);
	if (n != le32_to_cpu(hdr->partition_entry_array_crc32)) {
		debug("%s: GPT Entries CRC is wrong  0x%x != 0x%x\n", __func__, 
			n, le32_to_cpu(hdr->partition_entry_array_crc32));
		return 0;
	}
	return 1;
}

/* 
 * Check if the primary or alternate GPT table is valid 
 * Returns 1 if valid, 0 if invalid. 
 * Returns the number of entries and the actual starting entry for use by the caller, 
 */
int gpt_is_valid_table(efi_ptable *ptbl, int primary, int blk_cnt, u32 *numEntriesp, efi_gpt_entry_t **entrypp)
{
	efi_gpt_header_t *hdr;
	efi_gpt_entry_t *entry;
	u32 numEntries;
	u32 entriesToBackup;

	if (primary) {
		debug("%s: Checking Primary\n", __func__);
		hdr = &ptbl->primary_ptable.header;
		numEntries = le32_to_cpu(hdr->num_partition_entries);
		entry = ptbl->primary_ptable.entry;
	} else {
		debug("%s: Checking Alternate\n", __func__);
		hdr = &ptbl->secondary_ptable.header;
		numEntries = le32_to_cpu(hdr->num_partition_entries);

		/* cast because we start at header and back up */
		entry = (efi_gpt_entry_t *)&ptbl->secondary_ptable.header; 

		/* 
 	 	 * We need to back up to the correct starting entry based
	 	 * on the actual number of partition entries in the header. 
		 * If the number of entries is a multiple of 4, then we
		 * backup an integral number of sectors. But if the number
		 * of entries doesn't fit in an integral number of sectors,
		 * we have to back up so that the first entry is on a 
		 * sector boundary, and we ignore the padding between the
		 * last entry and the start of the header.
	 	 */
		entriesToBackup = EFI_PART2SECT(numEntries)/sizeof(efi_gpt_entry_t); 

		entry -= entriesToBackup; /* pointer arithmetic */
	}
	*numEntriesp = numEntries;
	*entrypp = entry;
	return (gpt_is_valid_hdr(hdr) && gpt_is_valid_entries(hdr, numEntries, entry));
}

/* 
 * Get valid gpt table from emmc flash.
 */
int gpt_get_table(efi_ptable *ptbl, u32 *numEntriesp, efi_gpt_entry_t **entry)
{
	struct mmc *mmc;	/* mmc device */
	block_dev_desc_t *mmc_dev;	/* mmc device info */
	int blk_cnt;		/* number of blocks to read to get entire table */
	int ret;		/* return code */
	int isPrimary;		/* flag for primary or alternate identifier */

	mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);
	if (!mmc) {
		error("%s: no mmc devices available\n", __func__);
		return -1;
	}

	mmc_dev = mmc_get_dev(CONFIG_SYS_MMC_ENV_DEV);
	if (!mmc_dev) {
		error("%s: failed to get mmc device info\n", __func__);
		return -1;
	}

	blk_cnt = sizeof(gpt) / mmc_dev->blksz;

#ifndef CONFIG_SKIP_PRIMARY_GPT
	/* Check primary GPT table */
	isPrimary = 1;
	ret =
	    mmc_dev->block_read(CONFIG_SYS_MMC_ENV_DEV, 0x0, blk_cnt,
				(void *)ptbl);
	debug("%s: %d blks read: %s\n", __func__, blk_cnt,
	      ret == blk_cnt ? "OK" : "ERROR");
	if (ret != blk_cnt) {
		error("%s: failed to read primary GPT ret=%d\n", __func__, ret);
		return -1;
	}
	if (gpt_is_valid_table(ptbl, isPrimary, blk_cnt, numEntriesp, entry)) {
		debug("%s: Primary GPT valid\n", __func__);
		return 0;
	}
#endif

	/* Check alternate GPT table */
	/* Read more than is necessary for simplicity but there may be less than 120 entries */
	isPrimary = 0;
	ret =
	    mmc_dev->block_read(CONFIG_SYS_MMC_ENV_DEV,
				mmc_dev->lba - SIZEOF_GPT, blk_cnt,
				(void *)ptbl);
	debug("%s: %d blks read: %s\n", __func__, blk_cnt,
	      ret == blk_cnt ? "OK" : "ERROR");
	if (ret != blk_cnt) {
		error("%s: failed to read alternate GPT ret=%d\n", __func__,
		      ret);
		return -1;
	}
	if (gpt_is_valid_table(ptbl, isPrimary, blk_cnt, numEntriesp, entry)) {
		debug("%s: Alternate GPT valid\n", __func__);
		return 0;
	} else {
		error("%s: cannot find valid GPT\n", __func__);
	}
	return -1;
}

/* 
 * Dynamically register partitions discovered in GPT table 
*/
static void register_partitions(u32 numEntries, efi_gpt_entry_t *entry)
{
	int i,j;		/* loop counters */
	u64 starting_lba;	/* starting sector */
	u64 ending_lba;		/* ending sector */
	fastboot_ptentry ptn;	/* partition structure to fill in and register */
	
	/* 
	 * For all entries, fill in the name/start/length/flags fields.
	 * Future enhancement might be to only fill in certain partitions
	 * that have one or more specific flags set. But currently we
	 * don't use the flags consistently so for now all partitions will
	 * be registered.
	 */
	for (i=0; i<numEntries; entry++, i++)
	{
		/* Set the name */
		memset(ptn.name, 0, sizeof(ptn.name));
		/* Convert unicode name to simple char * name */
		for (j=0; j<(sizeof(ptn.name)-1); j++) 
		{
			if (entry->partition_name[j]) 
			{
				ptn.name[j] = entry->partition_name[j];
			}
		}
		ptn.name[sizeof(ptn.name)-1] = '\0'; /* force terminating null */

		/* Get the start/end sectors. Note the benefit of the __le64 type */
		starting_lba = le64_to_cpu(entry->starting_lba);
		ending_lba = le64_to_cpu(entry->ending_lba);

		/* Set the starting sector */
		ptn.start = starting_lba;

		/* Calculate the total length */	
		ptn.length = (ending_lba + 1 - starting_lba) * EFI_SECTORSIZE;

		/* 
		 * FIXME fastboot.h defines the flags as 32 bits but they are actually 64 bits.
		 * There is also C code (e.g. cmd_fastboot.c) that assumes 32 bits so I didn't 
		 * change this until we can talk about it and figure out what to do.
		 * 
		 * FIXME Another problem is that the ptn.name field is only 16 bytes but the EFI spec allows
		 * for 36 bytes. This should also be fixed.
		 */
		ptn.flags = (u32)(le64_to_cpu(entry->attributes));	/* Cast to get around flags mistyped */ 

                printf("Adding: %32s, offset 0x%8.8x, size 0x%16.16llx, flags 0x%8.8x\n",
                       ptn.name, ptn.start, ptn.length, ptn.flags);

                fastboot_flash_add_ptn(&ptn);
	}
}


/* 
 * Function to find a valid GPT table and use it to populate the partition data structure
 * and register these with fastboot so the tables need not be hardcoded in the custom board 
 * configuration files. The rationale here is that customers may already have a gpt image
 * built with a layout file, and duplicating that layout file in u-boot is bad. Customers
 * probably won't run the "fastboot oem format" command either since they typically want to
 * avoid human intervention as part of the system build process.
 * Returns 0 on success, -1 on failure.
 */
int fastboot_discover_gpt_tables(void)
{
	u32 numEntries;			/* Number of partition entries */
	efi_gpt_entry_t *entry;		/* partition entry */
	efi_ptable *ptbl = &gpt;	/* gpt table image in ram */
	int ret;			/* return code */

	ret = gpt_get_table(ptbl, &numEntries, &entry);
	if (ret)
	{
		error("%s: failed to get valid gpt table from device\n", __func__);
		return ret;
	}
	register_partitions(numEntries, entry);
	return 0;
}
