#include <common.h>
#include <command.h>
#include <asm/arch/vcio.h>
#include <asm/io.h>
#include <asm/arch/ipc.h>


/*******************************************************************/
/* bootvc - boot vc image from image in memory */
/*******************************************************************/

// Don't allow multiple boots.
static int vc_init = 0;

int do_bootvc (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong	addr, ipc_base;
	int     rc = 0;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return rc;
	}

    if (vc_init == 0) {
        addr = simple_strtoul(argv[1], NULL, 16);
        printf("Booting videcore at 0x%08lx.\n", addr);

        bcm_mailbox_write(0, addr);

        rc = ipc_init();
        vc_init++;
    } else {
        printf("VC init already called, skipping...\n");
    }

	return rc;
}



U_BOOT_CMD(
		bootvc, CONFIG_SYS_MAXARGS, 1, do_bootvc,
		"boot videocore with appplication image at address 'addr'",
		"[addr] \n        - boot videocore with application image at address 'addr'\n"
		"          arguments in hexadecimal"
		);


