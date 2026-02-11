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
#include <environment.h>
#include <malloc.h>

static unsigned long int get_value(const char *val)
{
	char *env = getenv((char *)val);
	if (env)
		return simple_strtoul(env, NULL, 16);
	else
		return simple_strtoul(val, NULL, 16);
}

static int do_malloc(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	char numberbuf[32];

	if (argc < 3)
		return cmd_usage(cmdtp);
	void *mem = malloc(get_value(argv[2]));
	if (mem) {
            sprintf(numberbuf, "%08x", (unsigned int)mem);
		setenv(argv[1], numberbuf);
		return 0;
	} else {
		return -1;
	}
}

static int do_free(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 2)
		return cmd_usage(cmdtp);

	free((void *)get_value(argv[1]));
	setenv(argv[1], "");
	return 0;
}

U_BOOT_CMD(malloc, 3, 0, do_malloc,
	   "Allocate memory from u-boot heap and store pointer in environemnt variable.",
	   "target size\n");

U_BOOT_CMD(free, 2, 0, do_free,
	   "Release memory from u-boot heap at target.", "target\n");
