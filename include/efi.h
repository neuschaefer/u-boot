/************************************************************
 * EFI GUID Partition Table
 * Per Intel EFI Specification v1.02
 * http://developer.intel.com/technology/efi/efi.htm
 *
 * By Matt Domsch <Matt_Domsch@dell.com>  Fri Sep 22 22:15:56 CDT 2000  
 *   Copyright 2000,2001 Dell Inc.
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
 ************************************************************/
/* Portions copyright Broadcom 2010 */
/* based on linux/fs/partitions/efi.h */

#include <common.h>

#ifndef _EFI_H
#define _EFI_H

/*
 * Copyright portions of ... BROADCOM stuff goes here
*/
// typedef uint64_t __le64;
// typedef uint32_t __le32;
// typedef uint16_t __le16;
	
/* Linux sector addresses represent 512 byte blocks by convention. */
#define EFI_SECTORSIZE		512
#define EFI_SECTORADDR(x) ((x)/EFI_SECTORSIZE)

#define EFI_GPT_ATTR_REQ_TO_FUNCTION		0x00000001	/* Define active kernel/fs/boot1/boot2? */
#define EFI_GPT_ATTR_RESERVED_MASK			0x0000fffe
#define EFI_GPT_ATTR_GUID_SPECIFIC_MASK	0xffff0000	/* Define active kernel/fs/boot1/boot2? */

#define EFI_GPT_HEADER_SIZE					 92	/* crc'd hdr bytes of 512 byte sector */
#define EFI_GPT_ENTRY_SIZE						128	/* size of each partition entry */
#define EFI_GPT_NAMELEN_BYTES					 72	/* buffer size for name in partition record */

/* Start: This part from linux/include/efi.h */
typedef struct {
	uint8_t b[16];
} efi_guid_t;

typedef uint16_t efi_char16_t;	/* UNICODE character */

#define EFI_GUID(a,b,c,d0,d1,d2,d3,d4,d5,d6,d7) \
((efi_guid_t) \
{{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, ((a) >> 24) & 0xff, \
  (b) & 0xff, ((b) >> 8) & 0xff, \
  (c) & 0xff, ((c) >> 8) & 0xff, \
  (d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) }})
/* End: This part from linux/include/efi.h */

#define MSDOS_MBR_SIGNATURE 0xaa55
#define EFI_PMBR_OSTYPE_EFI 0xEF
#define EFI_PMBR_OSTYPE_EFI_GPT 0xEE

#define EFI_GPT_HEADER_SIGNATURE 0x5452415020494645ULL
#define EFI_GPT_HEADER_REVISION_V1 0x00010000
#define EFI_GPT_PRIMARY_PARTITION_TABLE_LBA 1

#define NULL_GUID	EFI_GUID(  0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 )

typedef struct {
	__le64 signature;
	__le32 revision;
	__le32 header_size;
	__le32 header_crc32;
	__le32 reserved1;
	__le64 my_lba;
	__le64 alternate_lba;
	__le64 first_usable_lba;
	__le64 last_usable_lba;
	efi_guid_t disk_guid;
	__le64 partition_entry_lba;
	__le32 num_partition_entries;
	__le32 sizeof_partition_entry;
	__le32 partition_entry_array_crc32;

	/* The rest of the logical block is reserved by UEFI and must be zero.
	 * EFI standard handles this by: */
	 uint8_t		reserved2[ EFI_SECTORSIZE - EFI_GPT_HEADER_SIZE ];
} 
#ifdef __GNUC__
__attribute__ ((packed)) 
#endif
efi_gpt_header_t;

typedef struct {
	efi_guid_t partition_type_guid;
	efi_guid_t unique_partition_guid;
	__le64 starting_lba;
	__le64 ending_lba;
	__le64 attributes;
	efi_char16_t partition_name[EFI_GPT_NAMELEN_BYTES / sizeof (efi_char16_t)];
} 
#ifdef __GNUC__
__attribute__ ((packed)) 
#endif
efi_gpt_entry_t;

/* struct partition from include/linux/genhd.h */
/*
 * 	genhd.h Copyright (C) 1992 Drew Eckhardt
 *	Generic hard disk header file by  
 * 		Drew Eckhardt
 *
 *		<drew@colorado.edu>
 */
typedef struct {
	unsigned char boot_ind;		/* 0x80 - active */
	unsigned char head;			/* starting head */
	unsigned char sector;		/* starting sector */
	unsigned char cyl;			/* starting cylinder */
	unsigned char sys_ind;		/* What partition type */
	unsigned char end_head;		/* end head */
	unsigned char end_sector;	/* end sector */
	unsigned char end_cyl;		/* end cylinder */
	__le32 start_sect;			/* starting sector counting from 0 */
	__le32 nr_sects;				/* nr of sectors in partition */
} 
#ifdef __GNUC__
__attribute__ ((packed)) 
#endif
efi_mbr_partition_t;

typedef struct {
	uint8_t boot_code[440];
	__le32 unique_mbr_signature;
	__le16 unknown;
	efi_mbr_partition_t partition_record[4];
	__le16 signature;
} 
#ifdef __GNUC__
__attribute__ ((packed)) 
#endif
efi_legacy_mbr_t;

/* 
 * User data that is filled in, describing the partitions. 
 */
typedef struct
{
	uint64_t start;			/* starting address of image in bytes */
	uint64_t end;				/* ending address of image in bytes */
	uint64_t attributes;		/* image attributes (flags) */
	char name[35];				/* name of image (not unicode) */
}
efi_partition_info_t;



void efi_randomize_guid(efi_guid_t *gp);
uint32_t efi_crc32(const void *buf, unsigned long len);

int efi_is_gpt_header_valid(efi_gpt_header_t *gpt, uint64_t lba, efi_gpt_entry_t *gpte, 
									 uint64_t total_disk_size);

int efi_guidcmp (efi_guid_t left, efi_guid_t right);
int efi_is_pte_valid(const efi_gpt_entry_t *pte, const uint64_t lastlba);
int efi_compare_gpts(efi_gpt_header_t *pgpt, efi_gpt_header_t *agpt, uint64_t lastlba);
void efi_populate_pmbr(efi_legacy_mbr_t *pmbr, uint64_t total_disk_size);

int efi_populate_header(efi_gpt_header_t *gpt, uint32_t numEntries, uint32_t entries_crc, 
								uint64_t total_disk_size, uint64_t gpt_lba, 
								int is_alternate_gpt_table, efi_guid_t *disk_guidp);

void	efi_populate_entry(efi_gpt_entry_t *gpte, efi_partition_info_t *infop);

void efi_populate_tables(efi_gpt_header_t *gpt, efi_gpt_header_t *agpt, 
								 uint32_t numEntries, efi_legacy_mbr_t *pmbr, 
								 efi_gpt_entry_t *entries, efi_partition_info_t *info, 
								 uint64_t total_disk_size, uint64_t gpt_lba);

void efi_dump_gpt(char *banner, efi_gpt_header_t *gpt);
void efi_dump_entry(efi_gpt_entry_t *entries);
void efi_dump_tables(efi_gpt_header_t *gpt, efi_gpt_header_t *agpt, efi_gpt_entry_t *entries);

#endif
