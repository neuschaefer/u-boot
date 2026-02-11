#include <exports.h>
#include <nand.h>
#include <asm/arch/platform.h>
#include <asm/io.h>
#include <bcm2708_smi.h>
#include "bcm2708_nand.h"

#define SLOT			0
#define DATA_PORT		0x1C
#define COMMAND_PORT	0x1D
#define ADDRESS_PORT	0x1E

#if 0 // Currently we force WP off in bootloader
// NAND WP is GPIO 16 active when high.
static void bcm2708_nand_set_wp(void)
{
	printf("setting nand wp\n");
	//writel(1<<16, GP_CLR0);
}

static void bcm2708_nand_clear_wp(void)
{
	printf("clearing nand wp\n");
	//writel(1<<16, GP_SET0);
}
#endif

static int bcm2708_nand_wait_rb(struct mtd_info *mtd)
{
	int ret = 0;
	int timeout = 0;
	do {
		ret = readl(GP_LEV0);
		//printf("ready?: %08x %d", ret, ret & (1<<17));
		ret &= (1<<17);
		//printf("wb count ++ %d\n", timeout);
		timeout++;
		if (timeout > 1000000) {
			printf("Warning: possible lockup in nand wait\n");
		}
	} while (!ret);
	return ret;
}

/* functions exported to mtd framework */
static void bcm2708_nand_hwcontrol(struct mtd_info *mtd, int cmd,
		unsigned int ctrl)
{
	//printf("cmd_ctrl: cmd %d(0x%08x), ctrl: %d\n", cmd,cmd, ctrl);
	if (cmd == NAND_CMD_NONE)
		return;
	/* command */
	if (ctrl & NAND_CLE)
		smi_write_direct(SLOT, COMMAND_PORT, sizeof(uint8_t), 1, (const void *)&cmd); 
	else
		smi_write_direct(SLOT, ADDRESS_PORT, sizeof(uint8_t), 1, (const void *)&cmd);
}

static uint8_t bcm2708_nand_read_byte(struct mtd_info *mtd)
{
	uint8_t read_data;
	/*read the byte and return */
	smi_read_direct(SLOT, DATA_PORT, sizeof(uint8_t), 1, (const void *)&read_data);
	return read_data;
}

static void bcm2708_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	smi_read(SLOT, DATA_PORT, len, 1, buf);
}

static void bcm2708_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	smi_write(SLOT, DATA_PORT, len, 1, buf);
}

static void bcm2708_nand_init_chip(struct nand_chip *chip)
{
	/* fill in the nand specific data */
	chip->chip_delay	= 50;
	chip->options		= 0;
	chip->cmd_ctrl		= bcm2708_nand_hwcontrol;
	chip->dev_ready		= bcm2708_nand_wait_rb; 
	chip->read_byte		= bcm2708_nand_read_byte; 
	chip->read_buf		= bcm2708_nand_read_buf;
	chip->write_buf		= bcm2708_nand_write_buf;
	chip->ecc.mode		= NAND_ECC_SOFT;

}

int board_nand_init(struct nand_chip *chip)
{

	int ret = 0;
	bcm2708_nand_print("board_nand_init\n");	

	// init u-boot structures to talk to our chip
	bcm2708_nand_init_chip(chip);

	// init register base for smi
	if( (ret = bcm2708_smi_init()) != 0)
		return ret;

	bcm2708_nand_print("NAND init done\n");

	return 0;
}
