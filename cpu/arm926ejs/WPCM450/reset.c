/* WPCM450 u-boot reboot */

#include <common.h>
#include <linux/types.h>
#include "soc_hw.h"

void
reset_cpu  (ulong addr)
{
    printf("Resetting ...\n");
	// Reset the watchdog timer module, for chip version less than 4
	// NOTE: From Z2, no need to reset watch dog timer module
	if((*(volatile unsigned char*)(GCR_PDID + 0x03)) < 4 )
	{
		   *(volatile unsigned long *)TIMER_WTCR = 0x01 ; // reset watch dog timer
	}
	*(volatile unsigned long *)TIMER_WTCR = 0x482; // enable watchdog timer & enable reset
    while(1);
}

