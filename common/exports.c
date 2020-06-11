#include <common.h>
#include <exports.h>

static void dummy(void)
{
}

unsigned long get_version(void)
{
	return XF_VERSION;
}

void jumptable_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	int i;

	gd->jt = (void **) malloc (XF_MAX * sizeof (void *));
	for (i = 0; i < XF_MAX; i++)
		gd->jt[i] = (void *) dummy;

	gd->jt[XF_get_version] = (void *) get_version;
	gd->jt[XF_malloc] = (void *) malloc;
	gd->jt[XF_free] = (void *) free;
	gd->jt[XF_get_timer] = (void *)get_timer;
	gd->jt[XF_udelay] = (void *)udelay;
#if defined(CONFIG_I386) || defined(CONFIG_PPC)
	gd->jt[XF_install_hdlr] = (void *) irq_install_handler;
	gd->jt[XF_free_hdlr] = (void *) irq_free_handler;
#endif	/* I386 || PPC */
#if (CONFIG_COMMANDS & CFG_CMD_I2C)
	gd->jt[XF_i2c_write] = (void *) i2c_write;
	gd->jt[XF_i2c_read] = (void *) i2c_read;
#endif	/* CFG_CMD_I2C */
#ifdef CONFIG_EXPORT_ETH_FNS
	gd->jt[XF_eth_init] = (void *) eth_init;
	gd->jt[XF_eth_halt] = (void *) eth_halt;
	gd->jt[XF_eth_send] = (void *) eth_send;
	gd->jt[XF_eth_rx] = (void *) eth_rx;
	gd->jt[XF_NetSetHandler] = (void *) NetSetHandler;
#endif /* EXPORT_ETH_FNS */
}
