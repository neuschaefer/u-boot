#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/brcm_rdb_sysmap.h>

#ifdef CONFIG_CAPRI
#include <asm/arch/brcm_rdb_secwatchdog_adrmod.h>
#endif

static int hw_wd_enabled = 0;

/* reset AON sec watch dog to expire after 90 sec = 62.5ms * 1440 */
void hw_watchdog_reset(void)
{
	if (hw_wd_enabled) // only reset wd if enabled
	{
		writel(	(1<<SECWATCHDOG_SDOGCR_EN_SHIFT)|
			(1<<SECWATCHDOG_SDOGCR_SRSTEN_SHIFT)|
			(4<<SECWATCHDOG_SDOGCR_CLKS_SHIFT) |	// 62.5ms
			(0x5A0<<SECWATCHDOG_SDOGCR_LD_SHIFT),	// load 1440
			(SECWD_BASE_ADDR + SECWATCHDOG_SDOGCR_OFFSET));
	}
}

/* initialize the AON sec watch dog */
void hw_watchdog_init(void)
{
	writel(	(0<<SECWATCHDOG_SDOGCR_EN_SHIFT)|
		(0<<SECWATCHDOG_SDOGCR_SRSTEN_SHIFT)|
		(4<<SECWATCHDOG_SDOGCR_CLKS_SHIFT) |
		(0x5A0<<SECWATCHDOG_SDOGCR_LD_SHIFT),
		(SECWD_BASE_ADDR + SECWATCHDOG_SDOGCR_OFFSET));

	/* Delay till value is loaded */
	while (readl(SECWD_BASE_ADDR + SECWATCHDOG_SDOGCR_OFFSET) & SECWATCHDOG_SDOGCR_WD_LOAD_FLAG_MASK) {;}

	writel(	(1<<SECWATCHDOG_SDOGCR_EN_SHIFT)|
		(1<<SECWATCHDOG_SDOGCR_SRSTEN_SHIFT)|
		(4<<SECWATCHDOG_SDOGCR_CLKS_SHIFT) |
		(0x5A0<<SECWATCHDOG_SDOGCR_LD_SHIFT),
		(SECWD_BASE_ADDR + SECWATCHDOG_SDOGCR_OFFSET));

	hw_wd_enabled = 1;
}

/* disable the AON sec watch dog */
void hw_watchdog_disable(void)
{
	writel(	(0<<SECWATCHDOG_SDOGCR_EN_SHIFT)|
		(0<<SECWATCHDOG_SDOGCR_SRSTEN_SHIFT)|
		(4<<SECWATCHDOG_SDOGCR_CLKS_SHIFT) |
		(0x5A0<<SECWATCHDOG_SDOGCR_LD_SHIFT),
		(SECWD_BASE_ADDR + SECWATCHDOG_SDOGCR_OFFSET));

#ifdef CONFIG_CAPRI
	writel(	(0<<SECWATCHDOG_ADRMOD_SDOGCR_EN_SHIFT)|
		(0<<SECWATCHDOG_ADRMOD_SDOGCR_SRSTEN_SHIFT)|
		(4<<SECWATCHDOG_ADRMOD_SDOGCR_CLKS_SHIFT) |
		(0x5A0<<SECWATCHDOG_ADRMOD_SDOGCR_LD_SHIFT),
		(SECWD2_BASE_ADDR + SECWATCHDOG_ADRMOD_SDOGCR_OFFSET));
#endif

	hw_wd_enabled =	0;
}
