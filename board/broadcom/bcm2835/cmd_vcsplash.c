#include <common.h>
#include <command.h>
#include <asm/arch/vcio.h>
#include <asm/arch/ipc.h>
#include <asm/io.h>
#include "mdec.h"

/*******************************************************************/
/* bootvc - boot vc image from image in memory */
/*******************************************************************/
int do_vcsplash (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong	addr, size;
	int     rc = 0;

	// Check VC Media Dec IPC.
    if(mdec_init()) {
		return -1;
	}

	if (argc < 2) {
		cmd_usage(cmdtp);
		return rc;
	} else if (argc == 2) {
		if (0 == strncmp(argv[1], "stop", 4)) {
			media_dec_set_state(0, 0);
		} else if (0 == strncmp(argv[1], "start",5)) {
			media_dec_set_state(1, 0);
		} else if (0 == strncmp(argv[1], "exit",4)) {
			media_dec_tear_down(1);
		} else {
			cmd_usage(cmdtp);
		}
		return rc;
	} else if (argc == 3) {
		// Loading from memory
		addr = simple_strtoul(argv[1], NULL, 16);
		size = simple_strtoul(argv[2], NULL, 16);
        printf("setup playback from %x with size %x\n", addr, size);
		if (size == 0) {
			printf("Error, size is 0\n");
			return rc;
		}
	} else if (argc == 5) {
		//int part, loadaddr;
		//char *filename, *dev;

		// Loading from file not implemented yet
		// fopen fstat memcpy(loadaddr, ...)
		printf("Not implemented\n");
		return rc;
	} else {
		cmd_usage(cmdtp);
		return rc;
	}

	// Setup the registers for playback
	media_dec_setup_data_playback(addr, size, 1);

	return rc;
}



U_BOOT_CMD(
		vcsplash, CONFIG_SYS_MAXARGS, 0, do_vcsplash,
		"show splash video on videocore with video at address 'addr'",
		"[addr size] | [dev partition addr filename]\n        - boot videocore with application image at address 'addr'\n"
		"          arguments in hexadecimal"
		);

