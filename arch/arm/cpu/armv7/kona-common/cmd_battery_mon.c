#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/kona-common/battery.h>


/*******************************************************************/
/* Monitor battery status:                                         *
 *     1. start charging if needed                                 *
 *     2. unless battery voltage is at argv[1] nicrovolts, do not  *
 *        boot to uImage                                           *
 *                                                                 *
 * This is to prevent the system from going into a power-cycle     *
 * loop.                                                           */
/*******************************************************************/

int do_battery_mon(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int     rc = 0;
	unsigned int  microvolts = 0;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return rc;
	}

    microvolts = simple_strtoul(argv[1], NULL, 10);
    printf("Threshold voltage set to %u \n", microvolts);

    battery_status_check(microvolts) ;

    return 0 ;	
}

U_BOOT_CMD(
		battery_mon, CONFIG_SYS_MAXARGS, 1, do_battery_mon,
		"battery voltage monitoring, and start charging when voltage is than threshhold",
		"[microvolts] \n - threshold voltage is microvolts"
		);


