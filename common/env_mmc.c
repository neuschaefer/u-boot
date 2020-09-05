/*
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <mmc.h>

/* references to names in env_common.c */
extern uchar default_environment[];

char *env_name_spec = "MMC";

#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr;
#endif /* ENV_IS_EMBEDDED */

static int mmc_env_devno;

/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

uchar env_get_char_spec(int index)
{
	return *((uchar *)(gd->env_addr + index));
}

int env_init(void)
{
	/* use default */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

#ifdef CONFIG_DYNAMIC_MMC_DEVNO
	mmc_env_devno = get_mmc_env_devno();
#else
	mmc_env_devno = CONFIG_SYS_MMC_ENV_DEV;
#endif

	return 0;
}

int init_mmc_for_env(struct mmc *mmc)
{
	if (!mmc) {
		puts("No MMC card found\n");
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return  -1;
	}

	return 0;
}

#ifdef CONFIG_CMD_SAVEENV

inline int write_env(struct mmc *mmc, unsigned long size,
			unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, mmc->write_bl_len) / mmc->write_bl_len;
	blk_cnt   = ALIGN(size, mmc->write_bl_len) / mmc->write_bl_len;

	n = mmc->block_dev.block_write(mmc_env_devno, blk_start,
					blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

int saveenv(void)
{
	struct mmc *mmc = find_mmc_device(mmc_env_devno);

	if (init_mmc_for_env(mmc))
		return 1;

	printf("Writing to MMC(%d)... ", mmc_env_devno);
	if (write_env(mmc, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, env_ptr)) {
		puts("failed\n");
		return 1;
	}

	puts("done\n");
	return 0;
}
#endif /* CONFIG_CMD_SAVEENV */

inline int read_env(struct mmc *mmc, unsigned long size,
			unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt   = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

	n = mmc->block_dev.block_read(mmc_env_devno, blk_start,
					blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

#if defined(CONFIG_MX50_ARM2) && defined(CONFIG_FSL_ENV_IN_MMC) /* E_BOOK */ 
#define	RAWTABLE_ID		0x44524244
#define	RAWTABLE_SIG	0x72617774
#define	RAWTABLE_RAW	0

typedef struct tagRAWTABLEHEADER {
	unsigned long	id;
	unsigned long	count;
	unsigned char	reserved1[24];
	unsigned char	reserved2[28];
	unsigned long	sig;
}	RAWTABLEHEADER;

typedef struct tagRAWDATAENTRY {
	unsigned long	type; // 0 : RAW / 1 : Partition
	unsigned long	start;
	unsigned long	size;
	unsigned char	reserved[20];
    char			label[32];
}	RAWDATAENTRY;

static	char*	rawtable = NULL;

void read_raw_table(void)
{
	struct mmc*		mmc = find_mmc_device(mmc_env_devno);
	uint			blk_start, blk_cnt, n;
	RAWTABLEHEADER*	header;

	if (init_mmc_for_env(mmc))
		return;

	rawtable = (char*)malloc(CONFIG_RAWTABLE_SIZE);
	if(!rawtable) {
		puts("rawtable : out of memory\n");
		return;
	}

	blk_start = ALIGN(CONFIG_RAWTABLE_OFFSET, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt   = ALIGN(CONFIG_RAWTABLE_SIZE, mmc->read_bl_len) / mmc->read_bl_len;

	n = mmc->block_dev.block_read(mmc_env_devno, blk_start, blk_cnt, (uchar *)rawtable);
	if(n != blk_cnt) {
		puts("rawtable : cannot read table\n");
		free(rawtable);
		rawtable = NULL;
		return;
	}

	header = (RAWTABLEHEADER*)rawtable;
	if(header->id != RAWTABLE_ID ||
	   header->count > ((CONFIG_RAWTABLE_SIZE - sizeof(RAWTABLEHEADER)) / sizeof(RAWDATAENTRY)) ||
	   header->sig != RAWTABLE_SIG) {
		puts("rawtable : table error\n");
		free(rawtable);
		rawtable = NULL;
		return;
	}
}

RAWDATAENTRY* rawtable_find(const char* name)
{
	RAWTABLEHEADER*	header = (RAWTABLEHEADER*)rawtable;
	RAWDATAENTRY*	entry = NULL;
	unsigned long	i;
	int				len = strlen(name);

	if(!rawtable ||
	   len >= 32)
		return entry;

	for(i = 0, entry = (RAWDATAENTRY*)&rawtable[sizeof(RAWTABLEHEADER)]; i < header->count; i++, entry++) {
		if(entry->type == RAWTABLE_RAW &&
		   strncmp(name, entry->label, len) == 0)
			return entry;
	}

	return NULL;
}

void env_relocate_spec(int boot_select)
{
	struct mmc*		mmc = find_mmc_device(mmc_env_devno);
	RAWDATAENTRY*	recovery = NULL;

	if (init_mmc_for_env(mmc))
		return;

	if(read_env(mmc, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, env_ptr) ||
	   crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc) {
		boot_select = 1;
	}

	read_raw_table();
	recovery = rawtable_find("Recovery Boot Env");
	if(!recovery || recovery->size != CONFIG_ENV_SIZE) {
		puts("not exists 'Recovery Boot Env' entry\n");
		recovery = NULL;
	}

	if(boot_select && recovery) {
		puts("Read recovery boot environment\n");

		if(init_mmc_for_env(mmc) ||
		   read_env(mmc, recovery->size, recovery->start, env_ptr) ||
		   crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc) {
			puts("read error\n");
			return use_default();
		}
	}

	gd->env_valid = 1;
}
#else /* E_BOOK */
void env_relocate_spec(void)
{
#if !defined(ENV_IS_EMBEDDED)
	struct mmc *mmc = find_mmc_device(mmc_env_devno);

	if (init_mmc_for_env(mmc))
		return;

	if (read_env(mmc, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, env_ptr))
		return use_default();

	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();

	gd->env_valid = 1;
#endif
}
#endif /* E_BOOK */

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
	puts ("*** Warning - bad CRC or MMC, using default environment\n\n");
	set_default_env();
}
#endif

