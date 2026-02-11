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
#include <command.h>
#include <environment.h>
#include <mmc.h>
#include <gpt.h>
#include <efi.h>
#include <malloc.h>		/* for free() prototype */

#ifndef ARRAY_LEN
#define ARRAY_LEN(x) sizeof(x)/sizeof((x)[0])
#endif

/* Verbosity levels */
#define V_QUIET  0
#define V_NORMAL 1
#define V_DEBUG  2
#define V_NOISY  3

#define PRINT(fmt, args... )                                      fprintf(stderr, fmt, ##args)
#define PNORMAL(fmt, args... )  if (gVerbose > V_QUIET)         { fprintf(stderr, fmt, ##args); }
#define PDEBUG(fmt, args... )   if (gVerbose > V_NORMAL)        { fprintf(stderr, fmt, ##args); }
#define PNOISY(fmt, args... )   if (gVerbose > V_DEBUG)         { fprintf(stderr, fmt, ##args); }

typedef struct {
	char name[35];		/* name of image (not unicode) */
	uint64_t start;		/* starting address of image in bytes */
	uint64_t size;		/* ending address of image in bytes */
	uint64_t attributes;	/* image attributes (flags) */
} info_t;

typedef struct {
	efi_legacy_mbr_t pmbr;
	efi_gpt_header_t gpt;
	efi_gpt_header_t agpt;
	efi_gpt_entry_t entries[128];	/* Primary entries */
	efi_gpt_entry_t alt_entries[128];	/* Alternate entries */
	int primaryGood;
	int alternateGood;
} gpt_state_t;

static char zero_pad[512];	/* For zero stuffing between entries and header, corruption */

static int gVerbose = V_NORMAL;

/* Unexported? */
unsigned long long simple_strtoull(const char *cp, char **endp,
				   unsigned int base);

static int readdata(void *dst, uint64_t offset, int bytecnt, struct mmc *mmc,
		    char *desc)
{
	int count = (bytecnt + 511) / 512;
	int block = offset / 512;

	if ((offset + bytecnt) > mmc->capacity) {
		PRINT("Attempted to read beyond end of MMC\n");
		return -1;
	}

	if (bytecnt == 0) {
		PDEBUG("readdata with bytecnt=0\n");
		return 0;
	}
	if (count != mmc->block_dev.block_read(0, block, count, dst)) {
		PRINT(desc, offset, bytecnt, "failed");
		return -1;
	}
	PDEBUG(desc, offset, bytecnt, "ok");
	return 0;
}

static int writedata(void *dst, uint64_t offset, int bytecnt, struct mmc *mmc,
		     char *desc)
{
	int count = (bytecnt + 511) / 512;
	int block = offset / 512;

	if ((offset + bytecnt) > mmc->capacity) {
		PRINT("Attempted to write beyond end of MMC\n");
		return -1;
	}

	if (bytecnt == 0) {
		PDEBUG("writedata with bytecnt=0\n");
		return 0;
	}
	if (count != mmc->block_dev.block_write(0, block, count, dst)) {
		PRINT(desc, offset, bytecnt, "failed");
		return -1;
	}
	PDEBUG(desc, offset, bytecnt, "ok");
	return 0;
}

static int rebuild_primary_gpt(const char *details, struct mmc *mmc)
{
	efi_gpt_entry_t entries[128];	/* Primary entries */
	efi_partition_info_t partinfo[64];
	efi_legacy_mbr_t pmbr;
	efi_gpt_header_t gpt;
	efi_gpt_header_t agpt;
	int imgcount = 0;
	uint64_t base_address = 0;

	memset(&pmbr, 0, sizeof(pmbr));
	memset(&gpt, 0, sizeof(gpt));
	memset(&agpt, 0, sizeof(agpt));

	// Parse partitionName,startaddr,size,attr...
	char *partitionNameStr = NULL;
	char *startStr = NULL;
	char *sizeStr = NULL;
	char *attrStr = NULL;
	uint64_t start;
	uint64_t size;
	uint64_t attr;

	char *toFree = partitionNameStr = strdup(details);
	char *nextp = NULL;

        // /* Add a gpt partition. */
        // strcpy(partinfo[imgcount].name, "gpt");
        // partinfo[imgcount].start = 0;
        // partinfo[imgcount].end = start + 0x20000 - EFI_SECTORSIZE;
        // partinfo[imgcount].attributes = 0;
        // imgcount++;

 	uint64_t next_start = 0x20000;

        while (*partitionNameStr != 0) {
		/* Skip any leading blanks. */
		while (*partitionNameStr == ' ')
			partitionNameStr++;
		if ((startStr = strchr(partitionNameStr, ',')) != NULL) {
			*startStr++ = '\0';
			if (startStr[0] ==  '-')
                            /* - means auto calculate. */
                            start = next_start;
                        else
                            start = simple_strtoull(startStr, NULL, 16);
		} else {
			PRINT
			    ("Error: Bad line format 1 at partitionName,startaddr,size,attr ...\n");
			return -1;
		}
		if ((sizeStr = strchr(startStr, ',')) != NULL) {
                        *sizeStr++ = '\0';
			if (sizeStr[0] == '-')
                            /* - size means all the rest. */
                            /* Not forgetting the alternate GPT. */
                            size = mmc->capacity - start - 0x20000;
               		else
                            size = simple_strtoull(sizeStr, NULL, 16);
   		} else {
			PRINT
			    ("Error: Bad line format 2 at partitionName,startaddr,size,attr ...\n");
			return -1;
		}
                next_start = start + size;
		if ((attrStr = strchr(sizeStr, ',')) != NULL) {
			*attrStr++ = '\0';
			attr = simple_strtoull(attrStr, &nextp, 16);
		} else {
			PRINT
			    ("Error: Bad line format 3 at partitionName,startaddr,size,attr ...\n");
			return -1;
		}
		strncpy(partinfo[imgcount].name, partitionNameStr,
			sizeof(partinfo[imgcount].name));
		partinfo[imgcount].start = start;
		if (size != 0) {
			partinfo[imgcount].end = start + size - EFI_SECTORSIZE;
		} else {
			partinfo[imgcount].end = start;
		}
		partinfo[imgcount].attributes = attr;

		PDEBUG
		    ("partition \'%s\' start=0x%llx, end=0x%llx, attr=0x%llx size=0x%llx\n",
		     partinfo[imgcount].name, partinfo[imgcount].start,
		     partinfo[imgcount].end, partinfo[imgcount].attributes,
		     size);
		partitionNameStr = nextp;
		imgcount++;
		/* Skip any trailing blanks. */
		while (*partitionNameStr == ' ')
			partitionNameStr++;
	}

        // /* Add an agpt partition. */
        // strcpy(partinfo[imgcount].name, "agpt");
        // partinfo[imgcount].start = mmc->capacity - 0x20000;
        // partinfo[imgcount].end   = mmc->capacity - EFI_SECTORSIZE;
        // partinfo[imgcount].attributes = 0;
        // imgcount++;

	efi_populate_tables(&gpt, &agpt, imgcount, &pmbr, entries, partinfo,
			    mmc->capacity, EFI_SECTORADDR(base_address) + 1);

	int blockallocsize = sizeof(pmbr) + sizeof(gpt) + imgcount * sizeof(efi_gpt_entry_t);
	char *block = malloc(blockallocsize);

	/* Write main PMBR, GPT HDR, and Partition Entries */
	memcpy(block, &pmbr, sizeof(pmbr));
	memcpy(block + sizeof(pmbr), &gpt, sizeof(gpt));
	memcpy(block + sizeof(pmbr) + sizeof(gpt), entries,
	       imgcount * sizeof(efi_gpt_entry_t));

	/* 
	 * Allocate a multiple of EFI_SECTORSIZE since the readdata
	 * reads in blocks and we don't want to trample
	 */
	int checksize = ((blockallocsize + EFI_SECTORSIZE - 1)/EFI_SECTORSIZE) * EFI_SECTORSIZE;
	char *check = malloc(checksize);

	readdata(check, 0, blockallocsize, mmc, "GPT read 0x%llx 0x%x (%s)\n");

	if (memcmp(check, block, sizeof(pmbr) + sizeof(gpt) + imgcount * sizeof(efi_gpt_entry_t)) != 0) {
		writedata(block, 0, blockallocsize, mmc, "GPT write 0x%llx 0x%x (%s)\n");
		PNORMAL("Wrote primary GPT.\n");
	} else {
		PNORMAL("No need to update primary GPT.\n");
	}
	free(block);
	free(check);
	free(toFree);

	return 0;
}

int corruptAlternate(struct mmc *mmc)
{
	uint64_t offset = mmc->capacity - sizeof(efi_gpt_header_t);
	int bytecnt;

	bytecnt = sizeof(efi_gpt_header_t);
	if (writedata(zero_pad, offset, bytecnt, mmc,
		      "Write corrupted alternate GPT header at offset=0x%llx bytecnt=0x%x (%s)\n"))
	{
		return -1;
	}
	PNORMAL("Corrupted alternate GPT header\n");
	return 0;
}

int corruptPrimary(struct mmc *mmc)
{
	uint64_t offset = sizeof(efi_legacy_mbr_t);
	int bytecnt;

	bytecnt = sizeof(efi_gpt_header_t);
	if (writedata(zero_pad, offset, bytecnt, mmc,
		      "Write corrupted primary GPT header at offset=0x%llx bytecnt=0x%x (%s)\n"))
	{
		return -1;
	}
	PNORMAL("Corrupted primary GPT header\n");
	return 0;
}

int readGPTs(struct mmc *mmc, gpt_state_t * state)
{
	int base_address = 0, numPartitions;
	uint64_t bytecnt, offset;
	uint64_t total_disk_size = mmc->capacity;
	if (readdata
	    (&state->pmbr, base_address, sizeof(efi_legacy_mbr_t), mmc,
	     "Read PMBR at offset=0x%llx bytecnt=0x%x (%s)\n")) {
		return -1;
	}

	offset = base_address + sizeof(efi_legacy_mbr_t);
	if (readdata
	    (&state->gpt, offset, sizeof(efi_gpt_header_t), mmc,
	     "Read primary GPT header at offset=0x%llx bytecnt=0x%x (%s)\n")) {
		return -1;
	}

	offset += sizeof(efi_gpt_header_t);

	/* Assume max entries, trim down when hdr crc passes and exact number known */
	bytecnt = ARRAY_LEN(state->entries) * sizeof(efi_gpt_entry_t);

	if (readdata
	    (state->entries, offset, bytecnt, mmc,
	     "Read primary GPT entries at offset=0x%llx bytecnt=0x%x (%s)\n")) {
		return -1;
	}

	numPartitions = le32_to_cpu(state->gpt.num_partition_entries);
	if (numPartitions > ARRAY_LEN(state->entries)) {
		numPartitions = ARRAY_LEN(state->entries);
	}

	if (efi_is_gpt_header_valid
	    (&state->gpt, EFI_SECTORADDR(base_address) + 1, state->entries,
	     total_disk_size)) {
		PDEBUG("GPT primary header valid\n");
		state->primaryGood = 1;
	} else {
		PDEBUG("GPT primary header invalid\n");
		state->primaryGood = 0;
	}

	offset = total_disk_size - 512;

	bytecnt = sizeof(efi_gpt_header_t);
	if (readdata
	    (&state->agpt, offset, bytecnt, mmc,
	     "Read alternate GPT header at offset=0x%llx bytecnt=0x%x (%s)\n"))
	{
		return -1;
	}

	numPartitions = le32_to_cpu(state->agpt.num_partition_entries);
	if (numPartitions > ARRAY_LEN(state->alt_entries) || numPartitions == 0) {
		numPartitions = ARRAY_LEN(state->alt_entries);
	}
	offset = total_disk_size - (numPartitions * sizeof(efi_gpt_entry_t)
				    + (EFI_PART2SECT(numPartitions) -
				       numPartitions * sizeof(efi_gpt_entry_t))
				    + sizeof(efi_gpt_header_t));

	bytecnt = numPartitions * sizeof(efi_gpt_entry_t);
	if (readdata
	    (state->alt_entries, offset, bytecnt, mmc,
	     "Read alternate GPT entries at offset=0x%llx bytecnt=0x%x (%s)\n"))
	{
		return -1;
	}

	if (efi_is_gpt_header_valid
	    (&state->agpt, EFI_SECTORADDR(total_disk_size) - 1,
	     state->alt_entries, total_disk_size)) {
		int errs;
		state->alternateGood = 1;
		errs =
		    efi_compare_gpts(&state->gpt, &state->agpt,
				     EFI_SECTORADDR(total_disk_size) - 1);
		if (errs) {
			if (state->primaryGood) {
				state->alternateGood = 0;	/* looks good but disagrees with primary. Mark bad. */
			}
		}
	} else {
		state->alternateGood = 0;
	}
	if (state->alternateGood) {
		PDEBUG("GPT alternate header valid\n");
		numPartitions = le32_to_cpu(state->agpt.num_partition_entries);
	}
	return 0;

}

int fixPrimary(struct mmc *mmc, gpt_state_t * state)
{
	uint64_t total_disk_size = mmc->capacity;
	uint64_t offset = 0;
	int bytecnt;
	unsigned int numPartitions;
	uint64_t base_address = 0;
	if (state->alternateGood) {
		numPartitions = le32_to_cpu(state->agpt.num_partition_entries);
		efi_populate_pmbr(&state->pmbr, total_disk_size);
		efi_populate_header(&state->gpt, numPartitions,
				    state->agpt.partition_entry_array_crc32,
				    total_disk_size,
				    EFI_SECTORADDR(base_address)
				    + 1, 0, &state->agpt.disk_guid);

		offset = base_address;
		bytecnt = sizeof(state->pmbr);
		if (writedata
		    (&state->pmbr, offset, bytecnt, mmc,
		     "Write pmbr to disk at offset=0x%llx bytecnt=0x%x (%s)\n"))
		{
			return -1;
		}
		offset += bytecnt;
		bytecnt = sizeof(state->agpt);
		if (writedata
		    (&state->gpt, offset, bytecnt, mmc,
		     "Write primary GPT header to disk at offset=0x%llx bytecnt=0x%x (%s)\n"))
		{
			return -1;
		}
		offset += bytecnt;
		bytecnt = numPartitions * sizeof(efi_gpt_entry_t);
		if (writedata
		    (state->alt_entries, offset, bytecnt, mmc,
		     "Write primary GPT entries to disk at offset=0x%llx bytecnt=0x%x(%s)\n"))
		{
			return -1;
		}

		PNORMAL("Fixed primary GPT header\n");
		state->primaryGood = 1;

	} else {
		PRINT("Can't fix primary, alternate also bad\n");
		return -1;
	}
	return 0;
}

int fixAlternate(struct mmc *mmc, gpt_state_t * state)
{
	uint64_t total_disk_size = mmc->capacity;
	uint64_t offset = 0;
	int bytecnt;
	unsigned int numPartitions;
	uint64_t base_address = 0;

	if (state->primaryGood) {
		numPartitions = le32_to_cpu(state->gpt.num_partition_entries);
		efi_populate_header(&state->agpt, numPartitions,
				    state->gpt.partition_entry_array_crc32,
				    total_disk_size,
				    EFI_SECTORADDR(base_address)
				    + 1, 1, &state->gpt.disk_guid);

		offset = total_disk_size
		    - (numPartitions * sizeof(efi_gpt_entry_t)
		       + (EFI_PART2SECT(numPartitions) -
			  numPartitions * sizeof(efi_gpt_entry_t))
		       + sizeof(state->agpt));

		bytecnt = numPartitions * sizeof(efi_gpt_entry_t);
		if (writedata
		    (state->entries, offset, bytecnt, mmc,
		     "Write alternate GPT entries to disk at offset=0x%llx bytecnt=0x%x(%s)\n"))
		{
			return -1;
		}

		offset += bytecnt;
		bytecnt =
		    EFI_PART2SECT(numPartitions) -
		    numPartitions * sizeof(efi_gpt_entry_t);
		if (bytecnt) {
			/* pad so that alt GPT is on 512 byte boundary */
			/* if (writedata(zero_pad, offset, bytecnt, mmc, "Write alternate GPT zero fill to disk at offset=0x%llx bytes=0x%x (%s)\n"))
			   {
			   return -1;
			   } */
		}
		offset += bytecnt;
		bytecnt = sizeof(state->agpt);
		if (writedata
		    (&state->agpt, offset, bytecnt, mmc,
		     "Write alternate GPT header to disk at offset=0x%llx bytecnt=0x%x (%s)\n"))
		{
			return -1;
		}
		PNORMAL("Fixed alternate GPT header\n");
		state->alternateGood = 1;

	} else {
		PRINT("Can't fix alternate, primary also bad\n");
		return -1;
	}
	return 0;
}


/* Global copy of gpt table in ram */
static efi_ptable gpt;

/*
 * Enumerate partition names into environment variable.
*/
static int gpt_enumerate(u32 numEntries, efi_gpt_entry_t * entry)
{
	int i, j;		/* loop counters */

	char sectname[36];		/* ascii name */

        char part_list[2048];
        part_list[0]=0;

	for (i = 0; i < numEntries; entry++, i++) {

		/* Set the name */
		memset(sectname, 0, sizeof(sectname));

		/* Convert unicode name to simple char * name */
		for (j = 0; j < (sizeof(sectname) - 1); j++) {
			if (entry->partition_name[j]) {
				sectname[j] = entry->partition_name[j];
			}
		}
		sectname[sizeof(sectname) - 1] = '\0';	/* force terminating null */
                strcat(part_list,sectname);
                strcat(part_list," ");
	}
        part_list[strlen(part_list)-1]=0;
        debug("setenv gpt_partition_list %s\n", part_list);
        setenv("gpt_partition_list", part_list);

	return 0;
}


/*
 * Dynamically setup environment variables for offsets and sizes discovered in GPT table 
 * after running "gpt setenv" for a partition name, gpt_partition_addr and gpt_partition_size
 * environment variables will be set.
*/
static int gpt_setenv(const char *name, u32 numEntries, efi_gpt_entry_t * entry)
{
	int i, j;		/* loop counters */
	u64 starting_lba;	/* starting sector */
	u64 ending_lba;		/* ending sector */

	char sectname[36];		/* ascii name */
	u64 length;		/* size in 512 byte sectors */
	char startbuf[32];
	char sizebuf[32];

	for (i = 0; i < numEntries; entry++, i++) {

		/* Set the name */
		memset(sectname, 0, sizeof(sectname));

		/* Convert unicode name to simple char * name */
		for (j = 0; j < (sizeof(sectname) - 1); j++) {
			if (entry->partition_name[j]) {
				sectname[j] = entry->partition_name[j];
			}
		}
		sectname[sizeof(sectname) - 1] = '\0';	/* force terminating null */

		/* Get the start/end sectors. Note the benefit of the __le64 type */
		starting_lba = le64_to_cpu(entry->starting_lba);
		ending_lba = le64_to_cpu(entry->ending_lba);

		/* Calculate the total length in sectors */
		length = ending_lba + 1 - starting_lba;

		if (!strcmp(name, sectname))
		{
			/* Match found, setup environment variables */
			/* create name_start, name_size and assign values */
			sprintf(startbuf, "%llx", starting_lba);
			debug("setenv gpt_partition_addr %s\n", startbuf);
			setenv("gpt_partition_addr", startbuf);
			sprintf(sizebuf, "%llx", length);
			debug("setenv gpt_partition_size %s\n", sizebuf);
			setenv("gpt_partition_size", sizebuf);
			sprintf(sizebuf, "%d", i+1);
			debug("setenv gpt_partition_entry %s\n", sizebuf);
			setenv("gpt_partition_entry", sizebuf);
			sprintf(sizebuf, "%s", sectname);
			debug("setenv gpt_partition_name %s\n", sizebuf);
			setenv("gpt_partition_name", sizebuf);
			return 0;
		}
	}
	return -1;
}


/*
 * Dump GPT table information
 */
static int gpt_info(u32 numEntries, efi_gpt_entry_t *entry)
{
	int i, j;		/* loop counters */
	u64 starting_lba;	/* starting sector */
	u64 ending_lba;		/* ending sector */
	u64 length;		/* size in 512 byte sectors */

	printf("Partition         Start                   End                    Size             Name\n");

	for (i = 0; i < numEntries; entry++, i++) {

		/* Get the start/end sectors. Note the benefit of the __le64 type */
		starting_lba = le64_to_cpu(entry->starting_lba);
		ending_lba = le64_to_cpu(entry->ending_lba);

		/* Calculate the total length in sectors */
		length = ending_lba + 1 - starting_lba;

		printf("    %2d     %09llx (%09llx)  %09llx (%09llx)  %09llx (%09llx)  ", 
		i+1, 
		starting_lba, EFI_SECTORSIZE * starting_lba,
		ending_lba, EFI_SECTORSIZE * ending_lba,
		length, EFI_SECTORSIZE * length);
				
		for (j=0; j<35; j++) 
		{
			if (entry->partition_name[j]) 
			{
				printf("%c", entry->partition_name[j]);
			}
		}
		printf("\n");
	}
	return 0;
}

static int gpt_layout(u32 numEntries, efi_gpt_entry_t *entry)
{
    char xbuffer[8192];
    char *buffer = &xbuffer[0];

    int i, j;		/* loop counters */
    u64 starting_lba;	/* starting sector */
    u64 ending_lba;		/* ending sector */
    u64 length;		/* size in 512 byte sectors */
    u64 attributes;		/* size in 512 byte sectors */

    for (i = 0; i < numEntries; entry++, i++) {

        starting_lba = le64_to_cpu(entry->starting_lba);
        ending_lba   = le64_to_cpu(entry->ending_lba);
        attributes   = le64_to_cpu(entry->attributes);

        /* Calculate the total length in sectors */
        length = ending_lba + 1 - starting_lba;

	for (j=0; j<35; j++)
        {
            if (entry->partition_name[j])
            {
                buffer += sprintf(buffer, "%c", entry->partition_name[j]);
            }
        }

        buffer += sprintf(buffer, ",0x%llx,0x%llx,0x%llx ",EFI_SECTORSIZE * starting_lba, EFI_SECTORSIZE * length, attributes);
    }
    setenv("gpt_layout", xbuffer);
    return 0;
}


/*
 * Top level gpt command
 */
static int do_gpt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	efi_ptable *ptbl = &gpt;	/* gpt table image in ram */
	u32 numEntries;
	efi_gpt_entry_t *entry;

	if (argc < 2)
		return cmd_usage(cmdtp);


	ret = gpt_get_table(ptbl, &numEntries, &entry);

	if (strcmp(argv[1], "setenv") == 0) {
		if (argc != 3) {
			return cmd_usage(cmdtp);
		} else {
			return gpt_setenv(argv[2], numEntries, entry);
		}
	} else if (strcmp(argv[1], "enumerate") == 0) {
		if (argc != 2) {
			return cmd_usage(cmdtp);
		} else {
                        gpt_enumerate(numEntries, entry);
			return 0;
		}
        }
        else if (strcmp(argv[1], "info") == 0) {
		if (argc != 2) {
			return cmd_usage(cmdtp);
		} else {
			gpt_info(numEntries, entry);
			return 0;
		}
        }
        else if (strcmp(argv[1], "layout") == 0) {
		if (argc != 2) {
			return cmd_usage(cmdtp);
		} else {
			gpt_layout(numEntries, entry);
			return 0;
		}
        }
        else
        {
            struct mmc *mmc = NULL;

            gpt_state_t gpt_state;

            memset(&gpt_state, 0, sizeof(gpt_state));
            memset(&zero_pad, 0, sizeof(zero_pad));

            PDEBUG("Using device 0\n");

            mmc = find_mmc_device(0);
            if (!mmc) {
                PRINT("Cannot open MMC device 0\n");
                return -1;
            }

            if (strcmp(argv[1], "corrupt_alternate") == 0) {
                return corruptAlternate(mmc);
            } else if (strcmp(argv[1], "corrupt_primary") == 0) {
                return corruptPrimary(mmc);
            } else if (strcmp(argv[1], "rebuild") == 0) {
                return rebuild_primary_gpt(getenv("gpt_layout"), mmc);
            } else if (strcmp(argv[1], "summary") == 0) {
                readGPTs(mmc, &gpt_state);
                PRINT("Primary   GPT %s\n",
                      gpt_state.primaryGood ? "good" : "bad");
                PRINT("Alternate GPT %s\n",
                      gpt_state.alternateGood ? "good" : "bad");
		return (gpt_state.primaryGood || gpt_state.alternateGood) ? 0 : -1;
             } else if (strcmp(argv[1], "dump_tables") == 0) {
                readGPTs(mmc, &gpt_state);
                efi_dump_tables(&gpt_state.gpt, &gpt_state.agpt,
                                gpt_state.entries);
                return 0;
            } else if (strcmp(argv[1], "fix_alternate") == 0) {
                readGPTs(mmc, &gpt_state);
                return fixAlternate(mmc, &gpt_state);
            } else if (strcmp(argv[1], "fix_primary") == 0) {
                readGPTs(mmc, &gpt_state);
                return fixPrimary(mmc, &gpt_state);
            } else {
                return cmd_usage(cmdtp);
            }
        }
        return -1;
}

U_BOOT_CMD(gpt, 3, 0, do_gpt,
	   "GPT Table Utilities\n",
	   "setenv [name] \n"
           "    - setup env variables for one gpt section base, size, name, index\n"
	   "gpt info\n"
           "    - print GPT table section bases and sizes\n"
           "gpt enumerate\n"
           "    - store list of partitions to gpt_partition_list environment variable\n"
	   "gpt summary\n"
	   "    - Print summary of partition table validity\n"
	   "gpt dump_tables\n"
	   "    - Dump parition tables\n"
	   "gpt fix_alternate\n"
	   "    - Attempt to fix alternate partition table.\n"
	   "gpt corrupt_alternate\n"
	   "    - Corrupt alternate partition table.\n"
	   "gpt fix_primary\n"
	   "    - Attempt to fix primary partition table.\n"
	   "gpt corrupt_primary\n"
	   "    - Corrupt primary partition table.\n"
	   "gpt rebuild\n"
	   "    - Rebuild primiary GPT from gpt_layout environment variable.\n"
	   "gpt layout\n"
	   "    - Reset gpt_layout environment variable based on current GPT.\n");

