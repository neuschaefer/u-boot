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
#include <../disk/part_efi.h>

#define EFI_ENTRIES	120
#define EFI_NAMELEN	36
#define SIZEOF_GPT	(2 + EFI_ENTRIES / 4)

/* Convert to char[2] in little endian format from the short integer */
#define SHORT_TO_LE16(x) { (x) & 0xff, ((x) >> 8) & 0xff }

/* Convert to char[4] in little endian format from the 32-bit integer */
#define INT_TO_LE32(x) \
		{ (x) & 0xff, ((x) >> 8) & 0xff, \
		((x) >> 16) & 0xff, ((x) >> 24) & 0xff }

/* Convert to char[8] in little endian format from the 64-bit integer */
#define LONG_TO_LE64(x) \
		{ (x) & 0xff, ((x) >> 8) & 0xff, \
		((x) >> 16) & 0xff, ((x) >> 24) & 0xff, \
		((x) >> 32) & 0xff, ((x) >> 40) & 0xff, \
		((x) >> 48) & 0xff, ((x) >> 56) & 0xff }

/*
 * Reference:
 * http://en.wikipedia.org/wiki/GUID_Partition_Table#Partition_type_GUIDs
 *
 * The GUIDs in this table are written assuming a little-endian byte order.
 * For example, the GUID for an EFI System partition is written as
 * C12A7328-F81F-11D2-BA4B-00A0C93EC93B here, which corresponds to the 16 byte
 * sequence 28 73 2A C1 1F F8 D2 11 BA 4B 00 A0 C9 3E C9 3B . Only the first
 * three blocks are byte-swapped.
 *
 * Linux and Windows use the same GUID for their respective data partitions,
 * which is: EBD0A0A2-B9E5-4433-87C0-68B6B72699C7
 */
/*
 * Disk GUID (also referred as UUID on UNIX) used in partition table header
 * This is based on Version 4 (random) UUIDs use a scheme relying only on random
 * numbers. This algorithm sets the version number as well as two reserved bits.
 * All other bits are set using a random or pseudorandom data source. They have
 * the form: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx where x is any hexadecimal
 * digit and y is one of 8, 9, A, or B. e.g.f47ac10b-58cc-4372-a567-0e02b2c3d479
 * http://en.wikipedia.org/wiki/UUID#Version_4_.28random.29
 *
 */
static const u8 random_uuid[16] = {
	0x3a, 0xf6, 0x33, 0x93, 0x25, 0x2f, 0x22, 0x40,
	0xab, 0x5b, 0x49, 0x54, 0xe1, 0x3a, 0x85, 0x8b,
};

typedef union _guid_ptable {
	struct {
		legacy_mbr mbr;
		gpt_header header;
		gpt_entry entry[EFI_ENTRIES];
	} primary_ptable;
	struct {
		legacy_mbr mbr; /* Not Used */
		gpt_entry entry[EFI_ENTRIES];
		gpt_header header;
	} secondary_ptable;
} guid_ptable;

const static legacy_mbr g_mbr = {
	.partition_record[0] = {
		.boot_ind = 0, /* non-bootable */
		.head = 0xFF, /* bogus CHS */
		.sector = 0xFF,
		.cyl = 0xFF,
		.sys_ind = EFI_PMBR_OSTYPE_EFI_GPT,
		.end_head = 0xFF, /* bogus CHS */
		.end_sector = 0xFF,
		.end_cyl = 0xFF,
	},
	.signature = SHORT_TO_LE16(MSDOS_MBR_SIGNATURE),
};

const static gpt_header g_gpt_header = {
	.signature = LONG_TO_LE64(GPT_HEADER_SIGNATURE),
	.revision = INT_TO_LE32(GPT_HEADER_REVISION_V1),
	.header_size = INT_TO_LE32(sizeof(gpt_header)),
	.my_lba = LONG_TO_LE64(GPT_PRIMARY_PARTITION_TABLE_LBA),
	.alternate_lba = LONG_TO_LE64(GPT_PRIMARY_PARTITION_TABLE_LBA),
	.first_usable_lba = LONG_TO_LE64((unsigned long long)SIZEOF_GPT),
	.partition_entry_lba
		= LONG_TO_LE64(GPT_PRIMARY_PARTITION_TABLE_LBA + 1),
	.num_partition_entries = INT_TO_LE32(EFI_ENTRIES),
	.sizeof_partition_entry = INT_TO_LE32(sizeof(gpt_entry)),
};

static guid_ptable gpt;

/* Convert char[8] in little endian format to the host format integer
 */
static inline unsigned long long le64_to_int(unsigned char *le64)
{
	return (((unsigned long long)le64[7] << 56) +
		((unsigned long long)le64[6] << 48) +
		((unsigned long long)le64[5] << 40) +
		((unsigned long long)le64[4] << 32) +
		((unsigned long long)le64[3] << 24) +
		((unsigned long long)le64[2] << 16) +
		((unsigned long long)le64[1] << 8) +
		(unsigned long long)le64[0]);
}

static void init_legacy_mbr(legacy_mbr *mbr, u32 start_sect, u32 nr_sects)
{
	memcpy(mbr, &g_mbr, sizeof(legacy_mbr));
	memcpy(mbr->partition_record[0].start_sect, &start_sect, sizeof(u32));
	memcpy(mbr->partition_record[0].nr_sects, &nr_sects, sizeof(u32));
}

static void start_ptbl(guid_ptable *ptbl, unsigned blocks, int primary)
{
	gpt_header *hdr;
	u64 temp;

	memset(ptbl, 0, sizeof(*ptbl));

	if (primary) {
		hdr = &ptbl->primary_ptable.header;
		init_legacy_mbr(&ptbl->primary_ptable.mbr, 1, blocks);
		memcpy(hdr, &g_gpt_header, sizeof(gpt_header));
		temp = blocks - 1; /* Last Block */
		memcpy(hdr->alternate_lba, &temp, 8);
	} else {
		hdr = &ptbl->secondary_ptable.header;
		memcpy(hdr, &g_gpt_header, sizeof(gpt_header));
		temp = blocks - 1; /* Last Block */
		memcpy(hdr->my_lba, &temp, 8);
		temp = blocks - SIZEOF_GPT + 1;
		memcpy(hdr->partition_entry_lba, &temp, 8);
	}

	temp = blocks - SIZEOF_GPT;
	memcpy(hdr->last_usable_lba, &temp, 8);
	memcpy(&hdr->disk_guid, random_uuid, 16);
}

static void end_ptbl(guid_ptable *ptbl, int primary)
{
	gpt_header *hdr;
	gpt_entry *entry;
	u32 n;

	if (primary) {
		hdr = &ptbl->primary_ptable.header;
		entry = ptbl->primary_ptable.entry;
	} else {
		hdr = &ptbl->secondary_ptable.header;
		entry = ptbl->secondary_ptable.entry;
	}
	n = crc32(0, 0, 0);	/* unnecessary call */
	n = crc32(n, (void*) entry, sizeof(*entry) * EFI_ENTRIES);
	memcpy(hdr->partition_entry_array_crc32, &n, 4);

	n = crc32(0, 0, 0);	/* unnecessary call */
	n = crc32(0, (void*) hdr, sizeof(*hdr));
	memcpy(hdr->header_crc32, &n, 4);
}

static int add_ptn(guid_ptable *ptbl, u64 first, u64 last,
		const char *name, int primary)
{
	gpt_header *hdr;
	gpt_entry *entry;
	u32 n;

	if (primary) {
		hdr = &ptbl->primary_ptable.header;
		entry = ptbl->primary_ptable.entry;
	} else {
		hdr = &ptbl->secondary_ptable.header;
		entry = ptbl->secondary_ptable.entry;
	}

	if (first < SIZEOF_GPT) {
		error("%s: partition '%s' overlaps table\n", __func__, name);
		return -1;
	}

	if (last > le64_to_int(hdr->last_usable_lba)) {
		error("%s: partition '%s' does not fit\n", __func__, name);
		return -1;
	}

	for (n = 0; n < EFI_ENTRIES; n++, entry++) {
		if (le64_to_int(entry->ending_lba))
			continue;
		entry->partition_type_guid = PARTITION_BASIC_DATA_GUID;
		memcpy(&entry->unique_partition_guid, random_uuid, 16);
		memcpy(entry->starting_lba, &first, 8);
		memcpy(entry->ending_lba, &last, 8);
		for (n = 0; (n < EFI_NAMELEN) && *name; n++)
			entry->partition_name[n] = *name++;
		return 0;
	}
	error("%s: out of partition table entries\n", __func__);
	return -1;
}

static int populate_partitions(guid_ptable *ptbl, int blksz, int primary)
{
	int n;
	for (n = 0; n < fastboot_flash_get_ptn_count(); n++) {
		unsigned end_blk;
		fastboot_ptentry *p = fastboot_flash_get_ptn(n);
		end_blk = (p->start + p->length / blksz) - 1;
		if (!(p->flags & FASTBOOT_PTENTRY_FLAGS_GPT_ENTRY))
			continue;
		if (p->start < SIZEOF_GPT)
			continue;
		if (add_ptn(ptbl, p->start, end_blk, p->name, primary))
			return -1;
	}
	return 0;
}

int fastboot_format(const char *cmd)
{
	guid_ptable *ptbl = &gpt;
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);
	block_dev_desc_t *mmc_dev;
	int blk_cnt, ret;

	if (!mmc) {
		error("%s: no mmc devices available\n", __func__);
		return -1;
	}
	mmc_dev = mmc_get_dev(CONFIG_SYS_MMC_ENV_DEV);
	if (!mmc_dev) {
		error("%s: failed to get mmc device info\n", __func__);
		return -1;
	}

	blk_cnt = sizeof(guid_ptable) / mmc_dev->blksz;
#ifdef FASTBOOT_DUMP_PTN
	fastboot_flash_dump_ptn();
#endif

	start_ptbl(ptbl, mmc_dev->lba, 1);
	if (populate_partitions(ptbl, mmc_dev->blksz, 1) != 0) {
		error("%s: failed to construct primary GPT\n", __func__);
		return -1;
	}
	end_ptbl(ptbl, 1);

#ifndef CONFIG_SKIP_PRIMARY_GPT
	ret = mmc_dev->block_write(CONFIG_SYS_MMC_ENV_DEV, 0x0,
						blk_cnt, (void*) ptbl);
	debug("%s: %d blks write: %s\n", __func__, blk_cnt,
				ret == blk_cnt ? "OK" : "ERROR");
	if(ret != blk_cnt) {
		error("%s: failed to flash primary GPT\n", __func__);
		return -1;
	}
#endif
	start_ptbl(ptbl, mmc_dev->lba, 0);
	if (populate_partitions(ptbl, mmc_dev->blksz, 0) != 0) {
		error("%s: failed to construct secondary GPT\n", __func__);
		return -1;
	}
	end_ptbl(ptbl, 0);

	ret = mmc_dev->block_write(CONFIG_SYS_MMC_ENV_DEV,
			mmc_dev->lba - SIZEOF_GPT, blk_cnt, (void*) ptbl);
	debug("%s: %d blks write: %s\n", __func__, blk_cnt,
				ret == blk_cnt ? "OK" : "ERROR");
	if(ret != blk_cnt) {
		error("%s: failed to flash secondary GPT\n", __func__);
		return -1;
	}

	return 0;
}

