#include <common.h>
#include <config.h>
#include <asm/processor.h>

extern void soc_init(void);

int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/*Should match with linux mach type for this board */
	gd->bd->bi_arch_number = 900;

	/* Should match with linux Makefile.boot entry for params-phys-y */
	gd->bd->bi_boot_params = CFG_SDRAM_BASE+0x0100;

	/* Initialize SOC related */
	soc_init(); 
	return 0;
}


int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	bd_t *bd = gd->bd;

	/* Only one Bank*/
	bd->bi_dram[0].start = CFG_SDRAM_BASE;
	bd->bi_dram[0].size = CFG_SDRAM_LEN;
	return (0);
}

short 
ReadJumperConfig(void)
{
	/* Return Path ID */
	return -1;
}
