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
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/kona-common/chal_keypad.h>

#define MAX_COLS                8
#define MAX_ROWS                8

/*
 * Keypad mapping
 */
struct KEYMAP {
	/*
	 * Scancode contains two hex numbers. The 1st hex denotes the row and
	 * the 2nd hex denotes the column
	 *
	 * Syntax: 0x[Row][Column]
	 */
	unsigned int row;
	unsigned int col;
	char *name;
};

static struct KEYMAP board_keypad_keymap[] = { KONA_KEYPAD_KEYMAP };

#define MAX_SCANCODES (sizeof(board_keypad_keymap)/sizeof(struct KEYMAP))

/* Return 1 if key is pressed. Return 0 otherwise. */
static inline int key_is_pressed(CHAL_KEYPAD_MATRIX_t * keypad_status,
				 unsigned int row, unsigned int col)
{
	return chal_keypad_matrix_is_set(keypad_status, row, col) ? 1 : 0;
}

static CHAL_KEYPAD_HANDLE_t hKeypad;

/* Main key scan and event processing routine. Called from an ISR so make sure
 * not to do anything slow or can block. */
static int key_scan(char *name)
{
	unsigned int scancode;
	CHAL_KEYPAD_MATRIX_t keypad_status;

	/* read keypad status */
	chal_keypad_scan_get_status(&hKeypad, &keypad_status);

	for (scancode = 0; scancode < MAX_SCANCODES; scancode++) {
		if (strcmp(name, board_keypad_keymap[scancode].name) == 0) {
			unsigned int r = board_keypad_keymap[scancode].row;
			unsigned int c = board_keypad_keymap[scancode].col;
			if (key_is_pressed(&keypad_status, r, c)) {
				return 0;
			} else {
				return -1;
			}
		}
	}
	return -1;
}

static int initialized = 0;

int kona_keypad_init(void)
{
	CHAL_KEYPAD_CONFIG_t config;

	/* Initialize hardware. */
	config.rows = MAX_ROWS;
	config.columns = MAX_COLS;
	config.activeLowMode = 0;
	config.swapRowColumn = 0;
	config.interruptEdge = CHAL_KEYPAD_INTERRUPT_EDGE_MAX;
	config.debounceTime = 6;

	/* Interrupts are disabled and cleared during init */
	hKeypad.regBaseAddr = KEYPAD_BASE_ADDR;

	if (chal_keypad_init(&hKeypad, &config) == 0) {
		printf("KEY:   Kona Keypad intialized.\n");
		initialized = 1;
		return 0;
	} else {
		printf("KEY:   Kona Keypad intialized failed.\n");
		initialized = 0;
		return -1;
	}
}

int kona_keypad_remove(void)
{
	chal_keypad_term(&hKeypad);
	initialized = 0;
	return 0;
}

/*
 * Top level key command
 */
static int do_key(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int scancode, r, c;
	if (argc < 2)
		return cmd_usage(cmdtp);
	else if (!initialized) {
		printf("Keypad driver not initialized.\n");
	} else if (strcmp(argv[1], "list") == 0) {
		for (scancode = 0; scancode < MAX_SCANCODES; scancode++) {
			printf("%s is %s.\n",
			       board_keypad_keymap[scancode].name,
			       key_scan(board_keypad_keymap[scancode].name) ?
			       "not pressed" : "pressed");
		}
	} else if (strcmp(argv[1], "raw") == 0) {
		CHAL_KEYPAD_MATRIX_t keypad_status;
		chal_keypad_scan_get_status(&hKeypad, &keypad_status);
		for (c = 0; c < MAX_COLS; ++c) {
			for (r = 0; r < MAX_ROWS; ++r) {
				printf("%c",
				       key_is_pressed(&keypad_status, r,
						      c) ? '.' : '*');
			}
			printf("\n");
		}
	} else
		return key_scan(argv[1]);

	return 0;
}

U_BOOT_CMD(key, 2, 0, do_key, "Check if a key is pressed.\n", "list|<name>\n");
