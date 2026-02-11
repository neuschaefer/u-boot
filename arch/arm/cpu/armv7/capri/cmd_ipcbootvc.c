#include <common.h>
#include <command.h>
#include <image.h>
#include <mmc.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/kona-common/ipc.h>
#include <asm/arch/chal_ipc.h>

#include <asm/kona-common/secure.h>

#if defined( CONFIG_VC_LCD_GPIOMUX ) || defined( CONFIG_VC_JTAG_GPIOMUX )
#include <asm/arch/gpiomux.h>
#endif

/*******************************************************************/
/* bootvc - boot vc image from image in memory */
/*******************************************************************/

// Don't allow multiple boots.
static int vc_init = 0;


typedef struct
{
  const int number;
  const int input;
  const int initial_level;
  const char *description;
} vc4_pin_config_t;

static vc4_pin_config_t vc4_pin_config[] = {VC4_OWNED_PINS};

int capri_start_vc(ulong entry)
{
  int rc = 0;
  CHAL_IPC_BOOTMODE bootmode;
  CHAL_IPC_HANDLE ipcHandle;
  uint32_t		regVal;

  /* Enable VC4 JTAG */
  printf("Enabling VC4 JTAG\n");
  /* VC JTAG pinmux */
  writel(0x00000103, 0x35004B58);
  writel(0x00000103, 0x35004B5C);
  writel(0x00000103, 0x35004B60);
  writel(0x00000103, 0x35004B64);
  writel(0x00000103, 0x35004B6C);

  /* top_level_control - JTAG MUX select */
  writel(0x08808000, 0x350040FC);

  /* Enable Non-secure JTAG access to VC */
  writel(0x00048000, 0x35000718);

#ifdef CONFIG_BCM11140_ETH
  /* Power stuff from Gordon H */
  writel(0x000003ff, 0x35014024);
#endif
  writel(0x06318c0c, 0x35011000);
  writel(0x00000001, 0x35010000);

  /* enable power to IPC block */
  //writel(
  //(1 << ESUB_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT) |
  //(0xA5A5 << ESUB_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT),
  //ESUB_CLK_BASE_ADDR + ESUB_CLK_MGR_REG_WR_ACCESS_OFFSET);
  writel(0xa5a501, 0x34000000);
  writel(0x303, 0x3400041c);

#if defined( CONFIG_VC_LCD_GPIOMUX )
  {
    gpiomux_rc_e    gpiomux_rc;

    //Configure the LCD data pins as coming from the videocore.

    gpiomux_rc = gpiomux_requestGroup( CONFIG_VC_LCD_GPIOMUX, CONFIG_VC_LCD_GPIOMUX_ID, "vc-lcd" );
    if ( gpiomux_rc != gpiomux_rc_SUCCESS )
      {
	printf( "Request to mux gpio pins for vc dpi (LCD) failed: %d\n", gpiomux_rc );
	return 1;
      }
    printf("Configuring VC LCD mux\n");
  }
#endif

#if defined( CONFIG_VC_JTAG_GPIOMUX )
  {
    gpiomux_rc_e    gpiomux_rc;

    //Configure the Videocore JTAG pins (so we can use the debugger)

    gpiomux_rc = gpiomux_requestGroup( CONFIG_VC_JTAG_GPIOMUX, CONFIG_VC_JTAG_GPIOMUX_ID, "vc-jtag" );
    if ( gpiomux_rc != gpiomux_rc_SUCCESS )
      {
	printf( "Request to mux gpio pins for vc jtag failed: %d\n", gpiomux_rc );
	return 1;
      }
    printf("Configuring VC JTAG mux\n");
  }
#endif

  /* Assign I/O state to pins owned by Videocore */

  int len = sizeof(vc4_pin_config) / sizeof(vc4_pin_config_t);
  int i = 0;
  for (i = 0; i < len; i++)
    {
      gpio_request( vc4_pin_config[i].number, 0 );
      if (!vc4_pin_config[i].input)
	gpio_direction_output( vc4_pin_config[i].number, vc4_pin_config[i].initial_level);
      else
	gpio_direction_input( vc4_pin_config[i].number);
      // Debug.
      printf("Assigning GPIO %s (%d) to VideoCore as an %s.\n",vc4_pin_config[i].description, 
	     vc4_pin_config[i].number,
	     vc4_pin_config[i].input? "INPUT":"OUTPUT");
    };


  /* Boot the videocore */

  if (vc_init == 0) {
    printf("Booting VideoCore at 0x%08lx.\n", entry);

    if (running_in_non_secure_mode())
    {
        /* Set IPC secure register to non-secure so that VC can access the mailbox */
#ifndef CONFIG_BCM11130_ROKU_AUSTIN
        printf("Setting IPC Secure Registers for Non-secure mode\n");
        regVal = readl(TZCFG_BASE_ADDR + KONATZCFG_KONA_HUB_APB5_TZPROT_OFFSET);
        regVal += KONATZCFG_KONA_HUB_APB5_TZPROT_IPC_SECURE_TZPROT_MASK;
        writel(regVal, TZCFG_BASE_ADDR + KONATZCFG_KONA_HUB_APB5_TZPROT_OFFSET);
#endif
    }

#ifdef CONFIG_LI_MODE
    // replace with little island config
    bootmode = IPC_BOOTMODE_SPLITREMAP;
    if (running_in_non_secure_mode())
    {
        writel(0xffffffff, MPU_BASE_ADDR+MPU_SRAM_MPU_CTRL0_OFFSET);
        writel(0xffffffff, MPU_BASE_ADDR+MPU_SRAM_MPU_CTRL1_OFFSET);
        writel(0x0000ffff, MPU_BASE_ADDR+MPU_SRAM_MPU_CTRL2_OFFSET);
    }
#else
    // big island configuration
    bootmode = IPC_BOOTMODE_DIRECT;
#endif

    ipcHandle = chal_ipc_config( NULL );

    rc = chal_ipc_boot_vc( ipcHandle, entry, bootmode );

    if( rc == 0 )
      {
	printf("Booted VC ok!\n");
	vc_init++;
      }
    else
      printf("Failed to boot VC - resetting...\n");
  } else {
    printf("VC init already called, skipping...\n");
  }
  return rc;
}

/* capri_bootvc is now an alias for do_vc_run */
extern int do_vc_run(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

U_BOOT_CMD(
	   capri_bootvc, CONFIG_SYS_MAXARGS, 1, do_vc_run,
	   "Load and boot VideoCore with application image at address 'addr'",
	   "[addr] \n        - boot VideoCore with application image at address 'addr'\n"
	   "          arguments in hexadecimal"
	   );



