#include <common.h>
#include <command.h>
#include <asm/arch/vcio.h>
#include <asm/arch/ipc.h>
#include <asm/io.h>
#include "tv_interface.h"

/*******************************************************************/
/* tvcontrol -- switch tv outputs */
/*******************************************************************/
int do_vctvcontrol (cmd_tbl_t *cmdtp, int flag, int argc, char * argv[])
{
	int     rc = 0;

	// Check TV Interface IPC.
	if(tvif_init()) {
		return -1;
	}

	if (argc < 2) {
		cmd_usage(cmdtp);
		return rc;
	} else if (argc == 2) {
		if (0 == strncmp(argv[1], "off", 3)) {
			tvif_set_output(TV_INTF_CTRL_OFF);
		} else if (0 == strncmp(argv[1], "hdmi",4)) {
			tvif_set_output(TV_INTF_CTRL_HDMI);
		} else if (0 == strncmp(argv[1], "tv",2)) {
			tvif_set_output(TV_INTF_CTRL_SDTV);
		} else if (0 == strncmp(argv[1], "auto",4)) {
			tvif_set_output(TV_INTF_CTRL_AUTO);
		} else {
			cmd_usage(cmdtp);
		}
		return rc;
	} else {
		cmd_usage(cmdtp);
		return rc;
	}
	return rc;
}

U_BOOT_CMD(
		vctvcontrol, CONFIG_SYS_MAXARGS, 0, do_vctvcontrol,
		"control tv service on videocore [off | hdmi | tv | auto]",
		""
		);

