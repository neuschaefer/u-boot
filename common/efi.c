/************************************************************
 * EFI GUID Partition Table handling
 *
 * http://www.uefi.org/specs/
 * http://www.intel.com/technology/efi/
 *
 * efi.[ch] by Matt Domsch <Matt_Domsch@dell.com>
 *   Copyright 2000,2001,2002,2004 Dell Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 ************************************************************/
/* Portions copyright Broadcom 2010 */
/* based on linux/fs/partitions/efi.c */
#include <common.h>
#include <efi.h>
#include <gpt.h>

#ifndef efi_debug
/* Or disable or add your own low level print function here. */
#include <common.h>
#define efi_debug(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#endif

/* 
 * Simple pseudo random number generation to avoid bring in large amounts of library code.
 * These can be replaced with rngHw functions if desired, but not important as the GUID
 * fields are not used in booting the system.
 */
static uint32_t rand_ms = 0x1234;
static uint32_t rand_ls = 0x8765;

#if 0
static void srand(uint32_t seed)
{
   rand_ms = seed >> 16;
   rand_ls = seed & 0xffffffff;
}
#endif
static uint32_t rand(void)
{
    rand_ms = 21717 * (rand_ms & 0xffff) + (rand_ms >> 16);
    rand_ls = 30003 * (rand_ls & 0xffff) + (rand_ls >> 16);
    return (rand_ms << 16) + rand_ls;
}

/* Randomize a guid structure */
void efi_randomize_guid(efi_guid_t *gp)
{
	int i;
	for (i=0; i<16; i++)
	{
		gp->b[i] = i;
	}
}
/**
 * efi_crc32() - EFI version of crc32 function
 * @buf: buffer to calculate crc32 of
 * @len - length of buf
 *
 * Description: Returns EFI-style CRC32 value for @buf
 * 
 * This function uses the little endian Ethernet polynomial
 * but seeds the function with ~0, and xor's with ~0 at the end.
 * Note, the EFI Specification, v1.02, has a reference to
 * Dr. Dobbs Journal, May 1994 (actually it's in May 1992).
 */
uint32_t efi_crc32(const void *buf, unsigned long len)
{
	return crc32(0, buf, len);
}

/**
 * is_gpt_header_valid() - tests one GPT header and PTEs for 
 * validity 
 * @gpt is a GPT header ptr. 
 * @lba is lba that this gpt header resides in 
 * @gpte is an GPT partition entry ptr 
 * @total_disk_size if the usable disk size in bytes 
 *
 * Description: returns 1 if valid,  0 on error.
 */
int efi_is_gpt_header_valid(efi_gpt_header_t *gpt, uint64_t lba, efi_gpt_entry_t *gpte, uint64_t total_disk_size)
{
	uint32_t crc, origcrc;
	uint64_t lastlba;
	uint32_t i;

	if (!gpt)
		return 0;
	
	/* Check the GUID Partition Table signature */
	if (le64_to_cpu(gpt->signature) != EFI_GPT_HEADER_SIGNATURE) {
		efi_debug("GUID Partition Table Header signature is wrong for lba 0x%llx:"
			 "%lld != %lld\n", lba,
			 (unsigned long long)le64_to_cpu(gpt->signature),
			 (unsigned long long)EFI_GPT_HEADER_SIGNATURE);
		goto fail;
	}

	/* Check the GUID Partition Table CRC */
	origcrc = le32_to_cpu(gpt->header_crc32);
	gpt->header_crc32 = 0;
	crc = efi_crc32((const unsigned char *) gpt, le32_to_cpu(gpt->header_size));

	if (crc != origcrc) {
		efi_debug("GUID Partition Table Header CRC is wrong: %x != %x\n",
			 crc, origcrc);
		goto fail;
	}
	gpt->header_crc32 = cpu_to_le32(origcrc);

	/* Check that the my_lba entry points to the LBA that contains
	 * the GUID Partition Table */
	if (le64_to_cpu(gpt->my_lba) != lba) {
		efi_debug("GPT my_lba incorrect: %lld != %lld\n",
			 (unsigned long long)le64_to_cpu(gpt->my_lba),
			 (unsigned long long)lba);
		goto fail;
	}

	/* Check the first_usable_lba and last_usable_lba are
	 * within the disk.
	 */
	lastlba = (EFI_SECTORADDR(total_disk_size) - 1);
	if (le64_to_cpu(gpt->first_usable_lba) > lastlba) {
		efi_debug("GPT: first_usable_lba incorrect: %lld > %lld\n",
			 (unsigned long long)le64_to_cpu(gpt->first_usable_lba),
			 (unsigned long long)lastlba);
		goto fail;
	}
	if (le64_to_cpu(gpt->last_usable_lba) > lastlba) {
		efi_debug("GPT: last_usable_lba incorrect: %lld > %lld\n",
			 (unsigned long long)le64_to_cpu(gpt->last_usable_lba),
			 (unsigned long long)lastlba);
		goto fail;
	}

	/* Check the GUID Partition Entry Array CRC */
	crc = efi_crc32((const unsigned char *) gpte,
			le32_to_cpu(gpt->num_partition_entries) *
			le32_to_cpu(gpt->sizeof_partition_entry));

	if (crc != le32_to_cpu(gpt->partition_entry_array_crc32)) {
		efi_debug("GUID Partitition Entry Array CRC check failed. crc 0x%08x != given 0x%08x\n", crc, 
					le32_to_cpu(gpt->partition_entry_array_crc32));
		goto fail;
	}

	for (i=0; i<gpt->num_partition_entries; i++) 
	{
		if (!efi_is_pte_valid(&gpte[i], lastlba))
			efi_debug("GUID Partition Entry %d invalid\n", i);	
	}

	/* We're done, all's well */
	return 1;

 fail:
	return 0;
}

/* returns 0 if memory matches */
int efi_guidcmp (efi_guid_t left, efi_guid_t right)
{
	unsigned int i;
	for (i=0; i<sizeof(efi_guid_t); i++) 
	{
		if (left.b[i] != right.b[i]) 
		{
			return -1;
		}
	}
	return 0;
}

/**
 * efi_is_pte_valid() - tests one PTE for validity
 * @pte is the pte to check
 * @lastlba is last lba of the disk
 *
 * Description: returns 1 if valid,  0 on error.
 */
int efi_is_pte_valid(const efi_gpt_entry_t *pte, const uint64_t lastlba)
{
	uint64_t starting_lba = le64_to_cpu(pte->starting_lba);
	uint64_t ending_lba = le64_to_cpu(pte->ending_lba);
	
	if ((!efi_guidcmp(pte->partition_type_guid, NULL_GUID)) ||
	    (starting_lba > lastlba)     ||
	    (ending_lba   > lastlba)		||
		 (starting_lba > ending_lba))
	{
		return 0;
	}
	return 1;
}

/**
 * efi_compare_gpts() - Compare GPT headers
 * @pgpt is the primary GPT header
 * @agpt is the alternate GPT header
 * @lastlba is the last LBA number
 * Description: Returns errors found.  Sanity checks pgpt and
 * agpt fields and prints warnings on discrepancies. 
 * 
 */
int efi_compare_gpts(efi_gpt_header_t *pgpt, efi_gpt_header_t *agpt, uint64_t lastlba)
{
	int error_found = 0;
	if (!pgpt || !agpt) {
		error_found++;
		return error_found;
	}

	if (le64_to_cpu(pgpt->my_lba) != le64_to_cpu(agpt->alternate_lba)) {
		efi_debug("GPT:Primary header LBA != Alt. header alternate_lba\n");
		efi_debug("GPT:%lld != %lld\n",
		         (unsigned long long)le64_to_cpu(pgpt->my_lba),
               (unsigned long long)le64_to_cpu(agpt->alternate_lba));
		error_found++;
	}
	if (le64_to_cpu(pgpt->alternate_lba) != le64_to_cpu(agpt->my_lba)) {
		efi_debug("GPT:Primary header alternate_lba != Alt. header my_lba\n");
		efi_debug("GPT:%lld != %lld\n",
		         (unsigned long long)le64_to_cpu(pgpt->alternate_lba),
               (unsigned long long)le64_to_cpu(agpt->my_lba));
		error_found++;
	}
	if (le64_to_cpu(pgpt->first_usable_lba) !=
            le64_to_cpu(agpt->first_usable_lba)) {
		efi_debug("GPT:first_usable_lbas don't match.\n");
		efi_debug("GPT:%lld != %lld\n",
		         (unsigned long long)le64_to_cpu(pgpt->first_usable_lba),
               (unsigned long long)le64_to_cpu(agpt->first_usable_lba));
		error_found++;
	}
	if (le64_to_cpu(pgpt->last_usable_lba) !=
            le64_to_cpu(agpt->last_usable_lba)) {
		efi_debug("GPT:last_usable_lbas don't match.\n");
		efi_debug("GPT:%lld != %lld\n",
		       (unsigned long long)le64_to_cpu(pgpt->last_usable_lba),
                       (unsigned long long)le64_to_cpu(agpt->last_usable_lba));
		error_found++;
	}
	if (efi_guidcmp(pgpt->disk_guid, agpt->disk_guid)) {
		efi_debug("GPT:disk_guids don't match.\n");
		error_found++;
	}
	if (le32_to_cpu(pgpt->num_partition_entries) !=
            le32_to_cpu(agpt->num_partition_entries)) {
		efi_debug("GPT:num_partition_entries don't match: "
		         "0x%x != 0x%x\n",
		         le32_to_cpu(pgpt->num_partition_entries),
		         le32_to_cpu(agpt->num_partition_entries));
		error_found++;
	}
	if (le32_to_cpu(pgpt->sizeof_partition_entry) !=
            le32_to_cpu(agpt->sizeof_partition_entry)) {
		efi_debug("GPT:sizeof_partition_entry values don't match: "
		         "0x%x != 0x%x\n",
               le32_to_cpu(pgpt->sizeof_partition_entry),
		         le32_to_cpu(agpt->sizeof_partition_entry));
		error_found++;
	}
	if (le32_to_cpu(pgpt->partition_entry_array_crc32) !=
            le32_to_cpu(agpt->partition_entry_array_crc32)) {
		efi_debug("GPT:partition_entry_array_crc32 values don't match: "
		         "0x%x != 0x%x\n",
               le32_to_cpu(pgpt->partition_entry_array_crc32),
		         le32_to_cpu(agpt->partition_entry_array_crc32));
		error_found++;
	}
	if (le64_to_cpu(pgpt->alternate_lba) != lastlba) {
		efi_debug("GPT:Primary header thinks Alt. header is not at the end of the disk.\n");
		efi_debug("GPT:%lld != %lld\n",
			      (unsigned long long)le64_to_cpu(pgpt->alternate_lba),
			      (unsigned long long)lastlba);
		error_found++;
	}

	if (le64_to_cpu(agpt->my_lba) != lastlba) {
		efi_debug("GPT:Alternate GPT header not at the end of the disk.\n");
		efi_debug("GPT:%lld != %lld\n",
			      (unsigned long long)le64_to_cpu(agpt->my_lba),
			      (unsigned long long)lastlba);
		error_found++;
	}
	return error_found;
}

/* 
 * Write the protective MBR to offset 0 on the disk. This information is 
 * just filled in to keep 3rd party partition programs happy. This
 * should show up as an unbootable, unrecognized hard disk to legacy
 * programs like fdisk.
 * @pmbr is the protective MBR
 * @total_disk_size is the disk size in bytes
 */
void efi_populate_pmbr(efi_legacy_mbr_t *pmbr, uint64_t total_disk_size)
{
  memset (pmbr->partition_record, 0, sizeof pmbr->partition_record);

  pmbr->signature = cpu_to_le16(MSDOS_MBR_SIGNATURE);
  pmbr->partition_record[0].sys_ind = EFI_PMBR_OSTYPE_EFI_GPT;
  pmbr->partition_record[0].sector = 1;
  pmbr->partition_record[0].end_head = 0xFE;
  pmbr->partition_record[0].end_sector = 0xFF;
  pmbr->partition_record[0].end_cyl = 0xFF;
  pmbr->partition_record[0].start_sect = cpu_to_le32(1);
  pmbr->partition_record[0].nr_sects = ((total_disk_size - 1ULL) > 0xFFFFFFFFULL) 
	? cpu_to_le32(0xFFFFFFFF) : cpu_to_le32(total_disk_size - 1UL);
}

/* 
 * Write the GPT header 
 * @gpt is the GPT Header
 * @numEntries is the number of partitions
 * @entries_crc is the crc over all the entries
 * @total_disk_size is the disk size in bytes
 * @is_alternate_gpt_table is 1 for alternate, 0 for primary GPT table
 * @disk_guidp is the GUID for the disk (same for both primary and alternate tables)
 */
int efi_populate_header(efi_gpt_header_t *gpt, uint32_t numEntries, uint32_t entries_crc, 
								uint64_t total_disk_size, uint64_t gpt_lba, 
								int is_alternate_gpt_table, efi_guid_t *disk_guidp)
{
	uint64_t total_disk_sectors = EFI_SECTORADDR(total_disk_size);
	uint64_t images_start = sizeof(efi_legacy_mbr_t) + sizeof(efi_gpt_header_t) + EFI_PART2SECT(numEntries);
	uint64_t images_end = total_disk_size - sizeof(efi_gpt_header_t) - EFI_PART2SECT(numEntries) - 1;
	uint32_t header_crc32;

	memset(gpt, 0, sizeof(*gpt));
	gpt->signature = cpu_to_le64(EFI_GPT_HEADER_SIGNATURE);
	gpt->revision = cpu_to_le32(EFI_GPT_HEADER_REVISION_V1);
	gpt->header_size = cpu_to_le32(EFI_GPT_HEADER_SIZE);
	gpt->header_crc32 = 0;
	gpt->reserved1 = 0;

	if (is_alternate_gpt_table)
	{
      uint32_t gpte_sectors = EFI_SECTORADDR(EFI_PART2SECT(numEntries));
      gpt->my_lba = cpu_to_le64(total_disk_sectors - 1);
      gpt->alternate_lba = cpu_to_le64(gpt_lba);
      gpt->partition_entry_lba = cpu_to_le64(total_disk_sectors - 1 - gpte_sectors);
   }
	else
	{
		gpt->my_lba = cpu_to_le64(gpt_lba);
		gpt->alternate_lba = cpu_to_le64(total_disk_sectors - 1);
		gpt->partition_entry_lba = cpu_to_le64(gpt_lba+1);
	}

	gpt->first_usable_lba = cpu_to_le64(EFI_SECTORADDR(images_start));
	gpt->last_usable_lba = cpu_to_le64(EFI_SECTORADDR(images_end));

	gpt->disk_guid = *disk_guidp;
	gpt->num_partition_entries = cpu_to_le32(numEntries);
	gpt->sizeof_partition_entry = cpu_to_le32(sizeof(efi_gpt_entry_t));
	gpt->partition_entry_array_crc32 = cpu_to_le32(entries_crc);

	/* Make sure crc field in header is 0 when crc is run */
	header_crc32 = cpu_to_le32(efi_crc32(gpt, le32_to_cpu(gpt->header_size)));
	gpt->header_crc32 = header_crc32;

	return 0;
}

/* 
 * Write the GPT partition entry 
 * @gpte is the GPT partition entry
 * @infop is the information about each partition
 */
void	efi_populate_entry(efi_gpt_entry_t *gpte, efi_partition_info_t *infop)
{
  unsigned int i;
  unsigned int last_char_idx;
  
  memset (gpte, 0, sizeof(*gpte));

  
  efi_randomize_guid(&gpte->partition_type_guid);
  efi_randomize_guid(&gpte->unique_partition_guid);
  gpte->starting_lba = cpu_to_le64(EFI_SECTORADDR(infop->start));
  gpte->ending_lba = cpu_to_le64(EFI_SECTORADDR(infop->end));
  gpte->attributes = cpu_to_le64(infop->attributes);

  last_char_idx = (EFI_GPT_NAMELEN_BYTES-sizeof(efi_char16_t)/sizeof(efi_char16_t));  /* e.g. index 35 is the last 2 unicode bytes */
  for (i = 0; i < last_char_idx; i++)
  {
	  if (infop->name[i]) 
	  {
		 gpte->partition_name[i] = (efi_char16_t) cpu_to_le16((uint16_t) infop->name[i]);
		 continue;
	  }
	  break;
  }
}

/* 
 * Write the PMBR, GPT Header, and all partition entries 
 * @gpt is the GPT header
 * @agpt is the alternate GPT header
 * @numEntries is the number of partitions
 * @pmbr is the protective MBR to be filled in
 * @entries is the partition data an array to be filled in
 * @info is the information about each partition in an array
 * @total_disk_size is the disk size in bytes
 * @gpt_lba is the primary partition table base in bytes (after pmbr)
 */
void efi_populate_tables(efi_gpt_header_t *gpt, efi_gpt_header_t *agpt, uint32_t numEntries, efi_legacy_mbr_t *pmbr, 
								 efi_gpt_entry_t *entries, efi_partition_info_t *info, 
								 uint64_t total_disk_size, uint64_t gpt_lba)
{
	uint32_t i;
	uint32_t crc;
	efi_guid_t disk_guid;

	/* fill in the protective MBR structure */
	efi_populate_pmbr(pmbr, total_disk_size);

	/* fill in the partition entries information */
	for (i=0; i<numEntries; i++) 
	{
		efi_populate_entry(&entries[i], info++);
	}

	/* Calculate crc32 over the partition entries */
	crc = efi_crc32(entries, numEntries * sizeof(efi_gpt_entry_t));

	efi_randomize_guid(&disk_guid);
	
	/* fill in the gpt header */
	efi_populate_header(gpt, numEntries, crc,	total_disk_size, gpt_lba, 0, &disk_guid);

	/* fill in the alternate gpt header */
	efi_populate_header(agpt, numEntries, crc, total_disk_size, gpt_lba, 1, &disk_guid);

	
}
/* 
 * Dump the GPT Header 
 * @banner is a banner prefix string 
 * @gpt is the GPT header
 */
void efi_dump_gpt(char *banner, efi_gpt_header_t *gpt)
{
	int i;
	efi_debug("%s\n", banner);
	efi_debug("signature                    0x%llx\n", gpt->signature);
	efi_debug("revision                     0x%08x\n", gpt->revision);
	efi_debug("header_size                  %d\n", le32_to_cpu(gpt->header_size));
	efi_debug("header_crc32                 0x%08x\n", le32_to_cpu(gpt->header_crc32));
	efi_debug("my_lba                       0x%08llx  (0x%08llx)\n", le64_to_cpu(gpt->my_lba), EFI_SECTORSIZE * le64_to_cpu(gpt->my_lba));
	efi_debug("alternate_lba                0x%08llx  (0x%08llx)\n", le64_to_cpu(gpt->alternate_lba), EFI_SECTORSIZE * le64_to_cpu(gpt->alternate_lba));
	efi_debug("first_usable_lba             0x%08llx  (0x%08llx)\n", le64_to_cpu(gpt->first_usable_lba), EFI_SECTORSIZE * le64_to_cpu(gpt->first_usable_lba));
	efi_debug("last_usable_lba              0x%08llx  (0x%08llx)\n", le64_to_cpu(gpt->last_usable_lba), EFI_SECTORSIZE * le64_to_cpu(gpt->last_usable_lba));
	efi_debug("disk GUID                    ");
	for (i=0; i<16; i++) 
	{
		efi_debug("%02x", gpt->disk_guid.b[i]);
	}
	efi_debug("\n");
	efi_debug("partition_entry_lba          0x%08llx  (0x%08llx\n", le64_to_cpu(gpt->partition_entry_lba), EFI_SECTORSIZE * le64_to_cpu(gpt->partition_entry_lba));
	efi_debug("num_partition_entries        %d\n", le32_to_cpu(gpt->num_partition_entries));
	efi_debug("sizeof_partition_entry       0x%x\n", le32_to_cpu(gpt->sizeof_partition_entry));
	efi_debug("partition_entry_array_crc32  0x%08llx\n", le64_to_cpu(gpt->partition_entry_array_crc32));
}
/* 
 * Dump a GPT partition entry 
 * @entries is a partition entry 
 */
void efi_dump_entry(efi_gpt_entry_t *entries)
{
	int i;
	efi_debug("Partition Type   GUID ");
	for (i=0; i<16; i++) 
	{
		efi_debug("%02x", entries->partition_type_guid.b[i]);
	}
	efi_debug("\n");
	efi_debug("Unique Partition GUID ");
	for (i=0; i<16; i++) 
	{
		efi_debug("%02x", entries->unique_partition_guid.b[i]);
	}
	efi_debug("\n");
	efi_debug("starting_lba     0x%08llx   (0x%08llx)\n", le64_to_cpu(entries->starting_lba), EFI_SECTORSIZE * le64_to_cpu(entries->starting_lba));
	efi_debug("ending_lba       0x%08llx   (0x%08llx)\n", le64_to_cpu(entries->ending_lba), EFI_SECTORSIZE * le64_to_cpu(entries->ending_lba));
	efi_debug("name             '");
	for (i=0; i<35; i++) 
	{
		if (entries->partition_name[i]) 
		{
			efi_debug("%c", entries->partition_name[i]);
		}
	}
	efi_debug("'\n");
}

/* 
 * Dump all GPT tables 
 * @gpt is the GPT header
 * @agpt is the alternate GPT header
 * @entries is an array of partition entries 
 */
void efi_dump_tables(efi_gpt_header_t *gpt, efi_gpt_header_t *agpt, efi_gpt_entry_t *entries)
{
	uint32_t i;
	efi_dump_gpt("\nPrimary GPT", gpt);
	efi_dump_gpt("\nAlternate GPT", agpt);
	for (i=0; i<le32_to_cpu(gpt->num_partition_entries); i++) 
	{
		efi_debug("\nPartition p%d\n", i+1);
		efi_dump_entry(&entries[i]);
	}
}
#if 0
/* 
 * Test main() - populates and verifies data structures.
 * Writes gpt.bin file to disk for inspection.
 * This is an example of how the bootstrap might use the
 * functions above for a different disk e.g. eMMC.
 */
static efi_gpt_entry_t entries[16];
int main(int argc, char **argv)
{
	/* Write a partition table to gpt.bin and read back and validate it. */
	FILE *ifp;
	FILE *ofp;
	char *fname = "gpt.bin";
	int rc;
	efi_legacy_mbr_t pmbr;
	efi_gpt_header_t gpt;
	efi_gpt_header_t agpt;
	efi_partition_info_t partinfo[16];
	uint32_t numEntries;

	uint64_t i,total_disk_size = 0x10000;
	uint64_t last_lba;

	memset(&pmbr, 0, sizeof(pmbr));
	memset(&gpt, 0, sizeof(gpt));
	memset(&agpt, 0, sizeof(agpt));

	ofp = fopen(fname, "w+");
	if (!ofp) 
	{
		efi_debug("Could not open %s for writing\n", fname);
		exit(1);
	}

	// fill in start and end values from the user specification
	numEntries = 5;
	partinfo[0].start = 0x1000;
	partinfo[0].end  = 0x2000;
	partinfo[0].attributes = EFI_GPT_ATTR_REQ_TO_FUNCTION;
	strncpy(partinfo[0].name, "partition_0", sizeof(partinfo[0].name));
	
	partinfo[1].start = 0x3000;
	partinfo[1].end   = 0x4000;
	partinfo[1].attributes = EFI_GPT_ATTR_REQ_TO_FUNCTION + 0x100000000ULL;
	strncpy(partinfo[1].name, "partition_1", sizeof(partinfo[0].name));
	
	partinfo[2].start = 0x5000;
	partinfo[2].end   = 0x6000;
	partinfo[2].attributes = EFI_GPT_ATTR_REQ_TO_FUNCTION + 0x200000000ULL;
	strncpy(partinfo[2].name, "partition_2", sizeof(partinfo[0].name));
	
	partinfo[3].start = 0x7000;
	partinfo[3].end   = 0x8000;
	partinfo[3].attributes = EFI_GPT_ATTR_REQ_TO_FUNCTION + 0x300000000ULL;
	strncpy(partinfo[3].name, "partition_3", sizeof(partinfo[0].name));
	
	partinfo[4].start = 0x9000;
	partinfo[4].end   = 0xa000;
	partinfo[4].attributes = EFI_GPT_ATTR_REQ_TO_FUNCTION + 0x400000000ULL;
	strncpy(partinfo[4].name, "partition_4", sizeof(partinfo[0].name));

	efi_populate_tables(&gpt, &agpt, numEntries, &pmbr, entries, partinfo, total_disk_size);

	/* 'create' the disk image */
	for (i=0; i<total_disk_size; i++) 
	{
		char ch = 0xff;
		fwrite(&ch, 1, 1, ofp);
	}
	fseek(ofp, 0, SEEK_SET);
	fwrite(&pmbr, sizeof(pmbr), 1, ofp);
	fwrite(&gpt, sizeof(gpt), 1, ofp);
	fwrite(entries, numEntries * sizeof(efi_gpt_entry_t), 1, ofp);

	fseek(ofp, total_disk_size - EFI_PART2SECT(numEntries) - sizeof(gpt), SEEK_SET);
	fwrite(entries, numEntries * sizeof(efi_gpt_entry_t), 1, ofp);

	/* Put alternate gpt at last sector */
	fseek(ofp, total_disk_size - sizeof(gpt), SEEK_SET);
	fwrite(&agpt, sizeof(gpt), 1, ofp);
	fflush(ofp);
	fclose(ofp);

	/* Validate */
	efi_is_gpt_header_valid(&gpt, 1ULL, entries, total_disk_size);
	efi_is_gpt_header_valid(&agpt, EFI_SECTORADDR(total_disk_size)-1, entries, total_disk_size);
	efi_compare_gpts(&gpt, &agpt, EFI_SECTORADDR(total_disk_size)-1);

	efi_dump_tables(&gpt, &agpt, entries);
	/* 
	 * If both headers valid, fine.
	 *
	 * If one header valid and one header invalid, use the valid header and
	 * correct the invalid header. 
	 *
	 * If both headers invalid, fail to boot.
	 */
}
#endif


