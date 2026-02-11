#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <malloc.h>

extern void hw_watchdog_init(void);
extern void hw_watchdog_disable(void);

/*******************************************************************/
/* This command can be used to turn on/off watchdog timer.         */
/*******************************************************************/

int do_set_watchdog(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2) {
		cmd_usage(cmdtp);
		return 0;
	}

	if (strcmp(argv[1], "off") == 0) {
		printf("Turn off watchdog timer\n");
		hw_watchdog_disable();
	}
	else if (strcmp(argv[1], "on") == 0){
		printf("Turn on watchdog timer\n");
		hw_watchdog_init();
	}
	else {
		cmd_usage(cmdtp);
	}

	return 0;
}

U_BOOT_CMD(
        set_watchdog, 2, 1, do_set_watchdog,
        "Turn on/off watch dog timer",
        "set_watchdog on  --- enable the timer \n"
		"set_watchdog off --- disable the timer \n"
        );


