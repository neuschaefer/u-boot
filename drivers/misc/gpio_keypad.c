/*****************************************************************************
* Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
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
#include <asm/gpio.h>

/*
 * Keypad mapping
 */
struct KEYMAP {
	unsigned int gpio;
	unsigned int active_low;
	char *name;
};

static struct KEYMAP board_keypad_keymap[] = { GPIO_KEYMAP };

#define MAX_KEYPAD_GPIOS (sizeof(board_keypad_keymap)/sizeof(struct KEYMAP))

static int initialized = 0;

/*
 * Top level key command
 */
static int do_key(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int gpio;
	if (argc < 2)
		return cmd_usage(cmdtp);
	else if (!initialized) {
		for (gpio = 0; gpio < MAX_KEYPAD_GPIOS; gpio++) {
			gpio_direction_input(gpio);
		}
		initialized = 1;
	}

	if (strcmp(argv[1], "list") == 0) {
		for (gpio = 0; gpio < MAX_KEYPAD_GPIOS; gpio++) {
			printf("%s is %s.\n",
			       board_keypad_keymap[gpio].name,
			       (gpio_get_value(board_keypad_keymap[gpio].gpio) ^
				board_keypad_keymap[gpio].
				active_low) ? "pressed" : "not pressed");
		}
	} else {
		for (gpio = 0; gpio < MAX_KEYPAD_GPIOS; gpio++) {
			if (strcmp(argv[1], board_keypad_keymap[gpio].name) ==
			    0) {
				return (gpio_get_value
					(board_keypad_keymap[gpio].
					 gpio) ^ board_keypad_keymap[gpio].
					active_low) ? 0 : 1;
			}
		}
	}
	return 1;
}

U_BOOT_CMD(key, 2, 0, do_key, "Check if a key is pressed.\n", "list|<name>\n");
