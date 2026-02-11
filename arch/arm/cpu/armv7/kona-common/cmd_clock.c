/*
 * (C) Copyright 2011
 * Broadcom Inc
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>

#include <asm/kona-common/clk.h>

#ifdef CONFIG_KONA

int do_clockops(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	switch (argc) {
	case 4:
		if (strcmp(argv[2], "setrate") == 0) {
			unsigned long rate = 0;
			struct clk* clk = clk_get(argv[1]);
			if (clk == NULL) {
				puts("bad clock name\n");
				return 0;
			}
			rate = simple_strtoul(argv[3], NULL, 10);
			if (!rate) {
				puts("bad clock rate\n");
				return 0;
			}
			clk_set_rate(clk, rate);
			return 0;
		}
		else {
			return cmd_usage(cmdtp);
		}

	case 3:
		if (strcmp(argv[2], "getrate") == 0) {
			unsigned int rate = 0;
			struct clk* clk = clk_get(argv[1]);
			if (clk == NULL) {
				puts("bad clock name\n");
				return 0;
			}
			rate = clk_get_rate(clk);
			printf("Current rate = %d\n", rate);
			return 0;
		}
		else if (strcmp(argv[2], "disable") == 0) {
			struct clk* clk = clk_get(argv[1]);
			if (clk == NULL) {
				puts("bad clock name\n");
				return 0;
			}
			clk_disable(clk);
			return 0;
		}
		else if (strcmp(argv[2], "enable") == 0) {
			struct clk* clk = clk_get(argv[1]);
			if (clk == NULL) {
				puts("bad clock name\n");
				return 0;
			}
			clk_enable(clk);
			return 0;
		}
		
	default:
		return cmd_usage(cmdtp);
	}
}


U_BOOT_CMD(
	clock, 4, 1, do_clockops,
	"clock framework",
	"<clock name> enable\n"
	"clock <clock name> disable\n"
	"clock <clock name> getrate\n"
	"clock <clock name> setrate <rate>\n");
#endif
