#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/brcm_rdb_kona_gptimer.h>
#include <asm/arch/brcm_rdb_sysmap.h>

/*
 * The timer is a decrementer, we'll left it free running at 32kHz.
 * We have 0.032 ticks per microsecond and an overflow in almost 36hrs
 */
#define TIMER_CLOCK		(32000)
#define COUNT_TO_USEC(x)	(x * 32)
#define USEC_TO_COUNT(x)	(x / 32)	/* overflows at 71min */
#define TICKS_PER_HZ		(TIMER_CLOCK / CONFIG_SYS_HZ)
#define TICKS_TO_HZ(x)		((x) / TICKS_PER_HZ)


/* macro to read the decrementing 32 bit timer as an increasing count */
#define READ_TIMER() (readl(TIMER_BASE_ADDR + KONA_GPTIMER_STCLO_OFFSET))


int timer_init(void)
{
    return 0 ;
}

/* Return how many HZ passed since "base" */
ulong get_timer(ulong base)
{
    return  TICKS_TO_HZ(READ_TIMER()) - base;
}

void __udelay(unsigned long usec)
{
    ulong ini, end;
    ini = READ_TIMER();
    end = ini + USEC_TO_COUNT(usec);
	while (((signed)(end - READ_TIMER())) > 0)
		;
}
