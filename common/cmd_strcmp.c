/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#include <common.h>
#include <command.h>

static int do_strcmp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 3)
		return cmd_usage(cmdtp);

	if (strlen(argv[1]) != strlen(argv[2]))
		return -1;

	/* Compare the temporary string to the given parameter */
	if (!strncmp(argv[1], argv[2], strlen(argv[2])))
		return 0;

	return -1;
}

U_BOOT_CMD(
	strcmp, 3, 0, do_strcmp,
	"String to String Comparison\n",
	"[str] [str]\n"
	"\t- Compare two strings \n"
	"\t- return True if they are the same otherwise return False \n" );
