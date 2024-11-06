#include <linux/sizes.h>
#include <asm/global_data.h>
#include <log.h>
#include <fdt_support.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void) {
	gd->ram_size = SZ_128M;
	return 0;
}

#if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
	struct udevice *dev;

	/*
	 * Initialize clocks early, so that the CSB clock rate is known soon
	 * enough.
	 */
	uclass_get_device(UCLASS_CLK, 0, &dev);

	return 0;
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	ft_cpu_setup(blob, bd);
	return 0;
}
#endif
