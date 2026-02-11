#include <common.h>
#include <flash.h>
#include <malloc.h>
#include <asm/errno.h>
#include <nand.h>
#include "nandbch.h"
#include <asm/arch/chal_ccu_nand_inline.h>
#include <asm/arch/pinmux.h>
#include "bcmcapri_mtd.h"
#include <exports.h>
#include <asm/io.h>

#define ENOTSUPP	524	/* Operation is not supported */

#define NANDBCH_BANK_SIZE(ni)      (0x1U<<NANDBCH_BANK_SHIFT(ni))

/* Implementation. */

/* Empty function to keep nand_base.c link happy */
void reset_timer(void)
{
}

static void nandTestError(uint32_t rc)
{
	char *msg;

	switch (rc) {
	case NANDBCH_RC_SUCCESS:
		msg = "Success";
		break;
	case NANDBCH_RC_FAILURE:
		msg = "Generic failure";
		break;
	case NANDBCH_RC_NOMEM:
		msg = "UC memory not available";
		break;
	case NANDBCH_RC_TOUT:
		msg = "UC completion timeout";
		break;
	case NANDBCH_RC_BANK_ERROR:
		msg = "UC execution error";
		break;
	case NANDBCH_RC_DMA_ERROR:
		msg = "DMA error";
		break;
	case NANDBCH_RC_ECC_CFG_ERR:
		msg = "Unsupported BCH ECC configuration";
		break;
	case NANDBCH_RC_ECC_ERROR:
		msg = "Uncorrectable BCH ECC error";
		break;
	case NANDBCH_RC_ECC_TOUT:
		msg = "BCH ECC irq timeout";
		break;
	case NANDBCH_RC_BANK_CFG_ERR:
		msg = "Unsupported bank configuration";
		break;
	case NANDBCH_RC_BB_NOERASE:
		msg = "Erase bad block not alowed";
		break;
	case NANDBCH_RC_NON_ONFI:
		msg = "NAND not ONFI compliant";
		break;
	case NANDBCH_RC_PARAM_ERR:
		msg = "ONFI parameter page error";
		break;
	case NANDBCH_RC_EXTPARAM_ERR:
		msg = "ONFI extended parameter page error";
		break;
	case NANDBCH_RC_PINMUX_ERR:
		msg = "PINMUX error";
		break;
	case NANDBCH_RC_NOCONFIG:
		msg = "Configuration data not found";
		break;
	case NANDBCH_RC_IF_ERR:
		msg = "Interface configuration error";
		break;
	case NANDBCH_RC_FAIL_STATUS:
		msg = "NAND status fail";
		break;
	case NANDBCH_RC_AUX_CFG_ERR:
		msg = "Invalid AUX data configuration";
		break;
	case NANDBCH_RC_OOB_SIZE_ERR:
		msg = "Insufficient OOB bytes for current configuration";
		break;

	default:
		msg = "Unknown return code";
	};

	printf("rc=%d %s\n", rc, msg);
}

/**
 * nand_block_isbad - [MTD Interface] Check if block at offset is bad
 * @mtd:	MTD device structure
 * @offs:	offset relative to mtd start
 */
static int capri_nand_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	uint8_t is_bad;
	int ret;
	nandbch_info_t *chip = mtd->priv;;

	dbgprint("%s: offs = 0x%012llx\n", __func__, offs);

	/* Check for invalid offset */
	if (offs > mtd->size)
		return -EINVAL;

	ret = nandbch_block_isbad(chip,
				  offs / NANDBCH_BANK_SIZE(chip),
				  (offs % NANDBCH_BANK_SIZE(chip)) /
				  mtd->erasesize, &is_bad);
	if (ret != NANDBCH_RC_SUCCESS) {
		nandTestError(ret);
		return -1;
	}
	return is_bad == 1;
}

/**
 * nand_block_markbad - [MTD Interface] Mark block at the given offset as bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int capri_nand_block_markbad(struct mtd_info *mtd, loff_t offs)
{
	nandbch_info_t *chip = mtd->priv;;
	int ret;

	dbgprint("%s: offs = 0x%012llx\n", __func__, offs);
	if ((ret = capri_nand_block_isbad(mtd, offs))) {
		/* If it was bad already, return success and do nothing. */
		if (ret > 0)
			return 0;
		return ret;
	}

	ret = nandbch_block_markbad(chip,
				    offs / NANDBCH_BANK_SIZE(chip),
				    (offs % NANDBCH_BANK_SIZE(chip)) /
				    mtd->erasesize);
	if (ret != NANDBCH_RC_SUCCESS) {
		nandTestError(ret);
		return -EIO;
	}
	return 0;
}

/**
 * nand_erase - [MTD Interface] erase block(s)
 * @mtd:	MTD device structure
 * @instr:	erase instruction
 *
 * Erase one ore more blocks
 */
static int capri_nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int block, ret;
	nandbch_info_t *chip = mtd->priv;
	int blocks_per_bank =
	    NANDBCH_BANK_SIZE(chip) / NANDBCH_BLOCK_SIZE(chip);
	loff_t len;

	dbgprint("%s: start = 0x%012llx, "
		 "len = %llu\n", __func__,
		 (unsigned long long)instr->addr / mtd->erasesize,
		 (unsigned long long)instr->len / mtd->erasesize);

	/* Start address must align on block boundary */
	if ((instr->addr % mtd->erasesize) != 0) {
		printf("nand_erase: Unaligned address\n");
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if ((instr->len % mtd->erasesize) != 0) {
		printf("nand_erase: Length not block aligned\n");
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if ((instr->len + instr->addr) > mtd->size) {
		printf("nand_erase: Erase past end of device\n");
		return -EINVAL;
	}

	instr->fail_addr = 0xffffffff;

	/* Shift to get first block */
	block = (int)(instr->addr / mtd->erasesize);

	/* Loop through the blocks */
	len = instr->len / mtd->erasesize;

	instr->state = MTD_ERASING;

	while (len) {

		int rc;
		rc = nandbch_block_force_erase(chip, block / blocks_per_bank,
					       block % blocks_per_bank);
		if (rc != NANDBCH_RC_SUCCESS) {
			nandTestError(rc);
			return -EIO;
		}

		/* Increment block address and decrement length */
		len--;
		block++;;
	}
	instr->state = MTD_ERASE_DONE;

	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;

	/* Do call back function */
	if (!ret)
		mtd_erase_callback(instr);

	/* Return more or less happy */
	return ret;
}

/**
 * nand_write_oob - [MTD Interface] NAND write data and/or out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 */
static int capri_nand_write_oob(struct mtd_info *mtd, loff_t to,
				struct mtd_oob_ops *ops)
{
	nandbch_info_t *chip = mtd->priv;;
	int ret = -ENOTSUPP;

	dbgprint("%s: to = 0x%012llx\n", __func__, to);
	ops->retlen = 0;

	/* Do not allow writes past end of device */
	if (ops->datbuf && (to + ops->len) > mtd->size) {
		MTDDEBUG(MTD_DEBUG_LEVEL0, "nand_write_oob: "
			 "Attempt write beyond end of device\n");
		return -EINVAL;
	}

	switch (ops->mode) {
	case MTD_OOB_PLACE:
	case MTD_OOB_AUTO:
	case MTD_OOB_RAW:
		break;

	default:
		goto out;
	}

	if (ops->datbuf) {
		int rc = nandbch_page_write(chip, to / NANDBCH_BANK_SIZE(chip),
					    (to % NANDBCH_BANK_SIZE(chip)) /
					    mtd->writesize, ops->datbuf);
		if (rc != NANDBCH_RC_SUCCESS) {
			nandTestError(rc);
			return -EIO;
		}
	}
	if (ops->oobbuf) {
		int rc = nandbch_aux_write(chip, to / NANDBCH_BANK_SIZE(chip),
					   (to % NANDBCH_BANK_SIZE(chip)) /
					   mtd->writesize,
					   ops->datbuf + mtd->writesize);
		if (rc != NANDBCH_RC_SUCCESS) {
			nandTestError(rc);
			return -EIO;
		}
	}

 out:
	return ret;
}

/**
 * nand_read_oob - [MTD Interface] NAND read data and/or out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operation description structure
 *
 * NAND read data and/or out-of-band data
 */
static int capri_nand_read_oob(struct mtd_info *mtd, loff_t from,
			       struct mtd_oob_ops *ops)
{
	nandbch_info_t *chip = mtd->priv;;
	int ret = -ENOTSUPP;
	nandbch_eccstats_t stats;
#if 0
	uint8_t auxstats;
#endif
	uint32_t writesize = mtd->writesize;

	dbgprint
	    ("%s: from=0x%012llx, ops->datbuf=%p ops->oobbuf=%p writesize=0x%x\n",
	     __func__, from, ops->datbuf, ops->oobbuf, mtd->writesize);

	ops->retlen = 0;

	/* Do not allow reads past end of device */
	if (ops->datbuf && (from + ops->len) > mtd->size) {
		MTDDEBUG(MTD_DEBUG_LEVEL0, "nand_read_oob: "
			 "Attempt read beyond end of device\n");
		return -EINVAL;
	}

	switch (ops->mode) {
	case MTD_OOB_PLACE:
	case MTD_OOB_AUTO:
	case MTD_OOB_RAW:
		break;

	default:
		goto out;
	}

	ret = 0;

	if (ops->datbuf) {
		int rc = nandbch_page_read(chip, from / NANDBCH_BANK_SIZE(chip),
					   (from % NANDBCH_BANK_SIZE(chip)) /
					   writesize, ops->datbuf, &stats);
		if (rc != NANDBCH_RC_SUCCESS) {
			nandTestError(rc);
			return -EIO;
		}
	}
	if (ops->oobbuf) {
/* Linux use aux_read for speed. u-boot uses oob read for accessing oob bytes for debug */
#if 0
		int rc = nandbch_aux_read(chip, from / NANDBCH_BANK_SIZE(chip),
					  (from % NANDBCH_BANK_SIZE(chip)) /
					  writesize, ops->datbuf + writesize,
					  &auxstats);
#else
		int rc = nandbch_oob_read(chip, from / NANDBCH_BANK_SIZE(chip),
					  (from % NANDBCH_BANK_SIZE(chip)) /
					  writesize, ops->datbuf + writesize);
#endif
		if (rc != NANDBCH_RC_SUCCESS) {
			nandTestError(rc);
			return -EIO;
		}
	}
 out:
	return ret;
}

/**
 * nand_write - [MTD Interface] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @len:	number of bytes to write
 * @retlen:	pointer to variable to store the number of written bytes
 * @buf:	the data to write
 *
 * NAND write with ECC
 */
static int capri_nand_write(struct mtd_info *mtd, loff_t to, size_t len,
			    size_t * retlen, const uint8_t * buf)
{
	nandbch_info_t *chip = mtd->priv;;

	dbgprint("%s to=0x%llx len=0x%x buf=%p\n", __func__, to, len, buf);

	/* Do not allow writes past end of device */
	if ((to + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	*retlen = len;

	int i = len;
	while (i > 0) {
		int rc = nandbch_page_write(chip, to / NANDBCH_BANK_SIZE(chip),
					    (to % NANDBCH_BANK_SIZE(chip)) /
					    mtd->writesize, (uint8_t *) buf);
		if (rc != NANDBCH_RC_SUCCESS) {
			nandTestError(rc);
			return -EIO;
		}
		i -= mtd->writesize;
		to += mtd->writesize;
		buf += mtd->writesize;
	}
	return 0;
}

/**
 * nand_read - [MTD Interface] MTD compability function for nand_do_read_ecc
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @len:	number of bytes to read
 * @retlen:	pointer to variable to store the number of read bytes
run  * @buf:	the databuffer to put data
 *
 * Get hold of the chip and call nand_do_read
 */
static int capri_nand_read(struct mtd_info *mtd, loff_t from, size_t len,
			   size_t * retlen, uint8_t * buf)
{
	nandbch_info_t *chip = mtd->priv;;
	nandbch_eccstats_t stats;
	uint8_t dataerrs = 0;
	int col;

	dbgprint("%s from=0x%llx len=0x%x buf=%p\n", __func__, from, len, buf);

	col = (int)(from & (mtd->writesize - 1));

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	*retlen = len;

	int i = len;
	while (i > 0) {
		int j;
		unsigned bytes = min(mtd->writesize - col, i);
		int rc = nandbch_page_read(chip, from / NANDBCH_BANK_SIZE(chip),
					   (from % NANDBCH_BANK_SIZE(chip)) /
					   mtd->writesize, chip->databuf, &stats);
		if (rc != NANDBCH_RC_SUCCESS) {
			nandTestError(rc);
			return -EIO;
		}
		for (j = 0; j < stats.len; j++) {
			dataerrs += stats.errs[j];
		}
		if (dataerrs) {
			dbgprint("%s: %u corrected errors on read at 0x%llx\n",
			       __func__, dataerrs, from);
		}
		i -= bytes;
		memcpy(buf, chip->databuf + col, bytes);
		from += bytes;
		buf += bytes;
		col = 0;
	}
	return 0;
}

static nandbch_info_t ni;

static void dumpNandInfo(nandbch_info_t * pni)
{
	printf("\nNAND %d bit, ECC %s\n"
	       "ID %02X%02X%02X%02X%02X%02X%02X%02X\n"
	       "banks %d\n"
	       "bank size %d MB\n"
	       "page size %d KB\n"
	       "block size %d KB\n"
	       "aux data size %d bytes\n"
	       "used oob bytes %d",
	       NANDBCH_BUS_WIDTH(pni),
	       (NANDBCH_FLAGS(pni) & NANDBCH_FLAG_ECC) ? "enabled" : "disabled",
	       (NANDBCH_ID(pni))[0],
	       (NANDBCH_ID(pni))[1],
	       (NANDBCH_ID(pni))[2],
	       (NANDBCH_ID(pni))[3],
	       (NANDBCH_ID(pni))[4],
	       (NANDBCH_ID(pni))[5],
	       (NANDBCH_ID(pni))[6],
	       (NANDBCH_ID(pni))[7],
	       NANDBCH_BANKS(pni),
	       0x1 << (NANDBCH_BANK_SHIFT(pni) - 20),
	       NANDBCH_PAGE_SIZE(pni) >> 10,
	       NANDBCH_BLOCK_SIZE(pni) >> 10,
	       NANDBCH_AUX_SIZE(pni),
	       (NANDBCH_FLAGS(pni) & NANDBCH_FLAG_ECC) ? (NANDBCH_AUX_SIZE(pni)
							  +
							  NANDBCH_AUX_ECC_SIZE
							  (pni) +
							  NANDBCH_SECTORS(pni) *
							  NANDBCH_ECC_SIZE(pni))
	       : NANDBCH_AUX_SIZE(pni));

	if (NANDBCH_OOB_SIZE(pni)) {
		printf("/%d", NANDBCH_OOB_SIZE(pni));
	}

	printf("\n");

	if (NANDBCH_FLAGS(pni) & NANDBCH_FLAG_ECC) {
		printf("main data ecc %d/%d\n"
		       "aux data ecc %d/%d\n",
		       NANDBCH_ECC_T(pni), NANDBCH_SECTOR_SIZE(pni),
		       NANDBCH_AUX_ECC_T(pni), NANDBCH_AUX_SIZE(pni));
	}
	if (NANDBCH_FLAGS(pni) & NANDBCH_FLAG_TIMING) {
		printk("timing_select %d\n"
		       "timing mode %d\n",
		       NANDBCH_TIMING_SELECT(pni), 
		       NANDBCH_TIMING_MODE(pni));
	}
}

#ifndef CONFIG_NAND_USE_ONFI

#define NUM_GEOMETRIES 2	/* Put geometry table at the start of the first 2 blocks */

static int ReadConfigValid(nandbch_info_t * pni)
{
	uint32_t rc;
	uint32_t page;
	uint32_t address;
	uint32_t bank;

	/* Read config data */
	bank = 0;
	address = 0;
	page = address >> CONFIG_NAND_PAGE_SHIFT;
	dbgprint("%s bank%u cycles %u page %u\n", __func__, bank,
		 CONFIG_NAND_ADDR_CYCLES, page);
	if ((rc = nandbch_config(pni,
				 bank,
				 CONFIG_NAND_ADDR_CYCLES,
				 &page,
				 1 << (CONFIG_NAND_BLOCK_SHIFT -
				       CONFIG_NAND_PAGE_SHIFT), NUM_GEOMETRIES))
	    != NANDBCH_RC_SUCCESS) {
		nandTestError(rc);
		printf("%s: Fails to read configuration: bank %d addr 0x%08x\n",
		       __func__, bank, address);
		return 0;
	}
	if ((NANDBCH_BUS_WIDTH(pni) != 8) ||
	    (NANDBCH_ADDR_CYCLES(pni) != CONFIG_NAND_ADDR_CYCLES) ||
	    (NANDBCH_PAGE_SHIFT(pni) != CONFIG_NAND_PAGE_SHIFT) ||
	    (NANDBCH_BLOCK_SHIFT(pni) != CONFIG_NAND_BLOCK_SHIFT) ||
	    (NANDBCH_BANK_SHIFT(pni) != CONFIG_NAND_BANK_SHIFT) ||
	    (NANDBCH_SECTOR_SIZE(pni) != CONFIG_NAND_SECTOR_SIZE) ||
	    (NANDBCH_ECC_T(pni) != CONFIG_NAND_ECC_T)) {
		printf("%s: reads configuration: but invalid data\n", __func__);
		return 0;
	}
	/* Valid configuration */
	printf("%s: valid configuration\n", __func__);
	return 1;
}

/* Write 2 config structures 64 pages apart starting at address 0 */
static int WriteConfigValid(nandbch_info_t * pni)
{
	uint32_t rc;
	uint32_t address;
	uint32_t bank = 0;
	int configs_written = 0;
	uint32_t page;
	uint32_t i;

	/* 
	 * We need to preserve the pages of the block after the page 0 
	 * geometry table as this may contain other phases of bootstraps. 
	 * For simplicity, just save all other pages 1 - N-1 in a temporary
	 * buffer, erase the block, write the geometry, and then write
	 * the saved data.
	 */
	uint8_t *blockp = malloc(NANDBCH_BLOCK_SIZE(pni));
	if (!blockp) {
		printf("Cannot allocate 0x%x bytes for block buffer\n", NANDBCH_BLOCK_SIZE(pni));
		return 0;
	}

	address = 0; /* block aligned to geometry is at start of block */
	while (configs_written < NUM_GEOMETRIES) {
		page = (address >> NANDBCH_PAGE_SHIFT(pni));
		page++; /* skip geometry page */

		/* read the block data - page 1 goes into start of save buffer */
		memset(blockp, 0xff, NANDBCH_BLOCK_SIZE(pni));
		for (i=0; i < NANDBCH_PAGES(pni)-1; i++, page++) {
			rc = nandbch_page_read(pni, bank, page, &blockp[i*NANDBCH_PAGE_SIZE(pni)], NULL);
			if (rc != NANDBCH_RC_SUCCESS)
			{
				printf("Cannot read page 0x%x\n", page);
				nandTestError(rc);
				free(blockp);
				return 0;
			}
		}

		/* erase the block */
		while ((rc = nandbch_block_erase(pni, bank,
			address >> NANDBCH_BLOCK_SHIFT(pni))) != NANDBCH_RC_SUCCESS) {
			nandTestError(rc);
			printf("%s: Fail to erase block: bank %d addr 0x%08x",
			       __func__, bank, address);
			address += NANDBCH_BLOCK_SIZE(pni);
			continue;
		}

		/* Write config data to config page */
		page = (address >> NANDBCH_PAGE_SHIFT(pni));
		if ((rc = nandbch_config_write(pni, bank, page)) != NANDBCH_RC_SUCCESS) {
			nandTestError(rc);
			printf
			    ("%s: Fails to write configuration: bank %d addr 0x%08x\n",
			     __func__, bank, address);
			free(blockp);
			return 0; 
		} else {
			configs_written++;
		}

		/* Now write the saved page data to the block */
		page++; /* skip geometry page */
		for (i=0; i < NANDBCH_PAGES(pni)-1; i++, page++) {
			rc = nandbch_page_write(pni, bank, page, &blockp[i*NANDBCH_PAGE_SIZE(pni)]);
			if (rc != NANDBCH_RC_SUCCESS)
			{
				printf("Cannot write page 0x%x\n", page);
				nandTestError(rc);
				free(blockp);
				return 0;
			}
		}

		address += NANDBCH_BLOCK_SIZE(pni);
		if (address >= NANDBCH_BANK_SIZE(pni))
		{
			printf("Exhausted flash looking for valid blocks to erase/write geometry table\n");
			free(blockp);
			return 0;
		}
	}
	free(blockp);
	return 1;
}
#endif

extern void nandbch_remap_badblocks(nandbch_info_t * ni);
extern void nandbch_remap_swupbbt(struct mtd_info *mtd);

int board_nand_init(struct nand_chip *nand)
{
	uint32_t rc;
	uint8_t flags;
	nandbch_info_t test_ni;

	ccu_nand_init(CCU_NAND_POLICY_SET_POLICY_ALL,
		      CCU_NAND_PLL_SELECT_VAR_208M_CLK, 0);

	/* Initialize nand info */
	memset((void *)&ni, 0x0, sizeof(ni));

	/* Basic init with information we know apriori */
	flags = NANDBCH_FLAG_ECC;

#ifndef CONFIG_NAND_USE_ONFI
	/* This information must be known apriori in order to program the geometry page and skip bad blocks */
	flags |= NANDBCH_FLAG_GEOMETRY;
	NANDBCH_BUS_WIDTH(&ni) = 8;
	NANDBCH_ADDR_CYCLES(&ni) = CONFIG_NAND_ADDR_CYCLES;
	NANDBCH_PAGE_SHIFT(&ni) = CONFIG_NAND_PAGE_SHIFT;
	NANDBCH_BLOCK_SHIFT(&ni) = CONFIG_NAND_BLOCK_SHIFT;
	NANDBCH_BANK_SHIFT(&ni) = CONFIG_NAND_BANK_SHIFT;
	NANDBCH_OOB_SIZE(&ni) = CONFIG_NAND_OOB_SIZE;

	flags |= NANDBCH_FLAG_BCH_CONFIG;
	NANDBCH_SECTOR_SIZE(&ni) = CONFIG_NAND_SECTOR_SIZE;
	NANDBCH_ECC_T(&ni) = CONFIG_NAND_ECC_T;

	flags |= NANDBCH_FLAG_TIMING;
	NANDBCH_TIMING_SELECT(&ni) = CONFIG_NAND_TIMING_SELECT;
	NANDBCH_TIMING_MODE(&ni) = CONFIG_NAND_TIMING_MODE;

	test_ni = ni;		/* structure copy */

	rc = ReadConfigValid(&test_ni);
	/* We reinit to setup the correct internal ECC registers */
	if (nandbch_init(&ni, flags) != NANDBCH_RC_SUCCESS) {
		nandTestError(rc);
		printf("%s: Fails to initialize NAND\n", __func__);
		return -1;
	}
	dumpNandInfo(&ni);
	if (rc == 0) {
		printf("NAND failed initial read of geometry page\n");
		/* correct config in flash */
		test_ni = ni;
		if (!WriteConfigValid(&test_ni)) {
			printf("NAND failed to write geometry page\n");
			return -1;	/* failed to write valid config to flash */
		}
		printf("NAND write geometry page succeeded\n");
	} else {
		printf("NAND read of geometry page succeeded\n");
	}
	/* config is valid at this point */
#else
	/* ONFI init - geometry comes from ONFI */
	if (nandbch_init(&ni, flags) != NANDBCH_RC_SUCCESS) {
		nandTestError(rc);
		printf("%s: Fails to initialize NAND\n", __func__);
		return -1;
	}
	dumpNandInfo(&ni);
#endif

	/* Initialize uboot NAND commands. */

	nand_curr_device = 0;

	nand_info[0].size =
	    NANDBCH_BANKS(&ni) * 0x1 << (NANDBCH_BANK_SHIFT(&ni));
	nand_info[0].erasesize = NANDBCH_BLOCK_SIZE(&ni);
	nand_info[0].writesize = NANDBCH_PAGE_SIZE(&ni);
	nand_info[0].oobsize =
	    NANDBCH_AUX_SIZE(&ni) + NANDBCH_AUX_ECC_SIZE(&ni) +
	    (NANDBCH_SECTORS(&ni) * NANDBCH_ECC_SIZE(&ni));
	nand_info[0].oobavail = NANDBCH_AUX_SIZE(&ni);
	nand_info[0].name = "nand0";
	nand_info[0].type = 2;
	nand_info[0].numeraseregions = 0;
	nand_info[0].eraseregions = NULL;
	nand_info[0].erase = capri_nand_erase;
	nand_info[0].read = capri_nand_read;
	nand_info[0].write = capri_nand_write;
	nand_info[0].read_oob = capri_nand_read_oob;
	nand_info[0].write_oob = capri_nand_write_oob;
	nand_info[0].block_isbad = capri_nand_block_isbad;
	nand_info[0].block_markbad = capri_nand_block_markbad;
	nand_info[0].priv = &ni;

	add_mtd_device(&nand_info[0]);
	nandbch_remap_badblocks(&ni);
	nandbch_remap_swupbbt(&nand_info[0]);	
	return 0;
}
