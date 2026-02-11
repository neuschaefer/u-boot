#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <malloc.h>

/*******************************************************************/
/* Set the uImage size as a env variable:                          *
 *                                                                 *
 * This is used later for booting the kernel image from the RAM    */
/*******************************************************************/

int do_set_img_size(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc = 0;
	unsigned int addr = 0;
	unsigned int size;
	char s[4];

	if (argc < 2) {
		cmd_usage(cmdtp);
		return rc;
	}

	addr = simple_strtoul(argv[2], NULL, 10);
	printf("Reading the header for uImage from 0x%x\n",addr);
	/*FIXME*/
	/*The size is as of now rounded off to an extra block to avoid CRC check error. Will need to
	 * replace the values with MACROS*/
	size = (image_get_image_size((const image_header_t*) addr) + 511)/512;
	sprintf(s, "%x", size);
	setenv(argv[1], s);

	return 0;
}

U_BOOT_CMD(
        set_img_size, 3, 1, do_set_img_size,
        "set the image size env variable to be used for boot purpose",
        "[name] [addr] \n - name of environment variable to be set and address where the image is loaded"
        );


