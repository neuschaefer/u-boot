/*****************************************************************************
* Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
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
/**
*
*  @file    cmd_vc_display.c
*
*  @brief   Implements the vc display sub-command.
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <common.h>
#include <command.h>
#include <errno.h>

#include "cmd_vc_display.h"
#include "vc_debug_sym.h"

#include "slim_ipc_display.h"

#include <asm/kona-common/ipc.h>

/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */
// Shadow copy of the data in videocore.
static DISPLAY_IPC_T          display_local;

static VC_MEM_ACCESS_HANDLE_T display_vc_handle;
static VC_MEM_ADDR_T          display_vc_addr;

static int                    display_initialised = 0;

struct
{
   uint32_t   width;
   uint32_t   height;
   uint32_t   pitch;
   void     * image_data[2];
} display_fb_state;


/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
 *  Initialise the interface across to videocore
 ****************************************************************************/

static int vc_display_init (void)
{
   int rc = 0;
   const char * display_sym_name = "display_ipc";
   size_t mem_size;

   if (display_initialised)
   {
      return 0;
   }

   if (( rc = OpenVideoCoreMemory( &display_vc_handle )) != 0 ) {
      printf( "%s: OpenVideoCoreMemory failed: %d\n", __func__, rc );
      return rc;
   }

   if ( LookupVideoCoreSymbol( display_vc_handle, display_sym_name, &display_vc_addr, &mem_size ) < 0 ) {
      fprintf( stderr, "Symbol '%s' not found\n", display_sym_name );
      CloseVideoCoreMemory( display_vc_handle );
      return -ENOENT;
   }

   if ( mem_size != sizeof( display_local )) {
      fprintf( stderr, "%s has a size of %d bytes, expecting %d\n",
               display_sym_name, mem_size, sizeof( display_local ));
      CloseVideoCoreMemory( display_vc_handle );
      return -EIO;
   }

   display_initialised = 1;

   return rc;
}


/****************************************************************************
 *  Send a command across to videocore
 ****************************************************************************/

static int vc_display_send (int wait)
{
   int rc = 0;

   static uint32_t request_counter = 1;

   if ((ipc_query_wakeup_vc() & ~1) == 0) {
      fprintf( stderr, "VideoCore is not running\n" );
      return -1;
   }

   vc_display_init ();

   display_local.req = ++request_counter;
   display_local.ack = ~display_local.req;

   if ( !WriteVideoCoreMemory( display_vc_handle, &display_local, display_vc_addr, sizeof(display_local))) {
      fprintf( stderr, "Error writing %d bytes to vc addr 0x%08x\n", sizeof(display_local), display_vc_addr );
      return -EIO;
   }

   ipc_notify_vc_event (0xd);

   if (wait) {
      uint32_t ack_value = ~request_counter;
      VC_MEM_ADDR_T vc_addr_ack = display_vc_addr + offsetof (DISPLAY_IPC_T, ack);

      while (ack_value != request_counter) {
         if ( !ReadVideoCoreMemory( display_vc_handle, &ack_value, vc_addr_ack, sizeof(uint32_t)) != 0) {
            fprintf( stderr, "Error reading %d bytes from vc addr 0x%08x\n", sizeof(uint32_t), vc_addr_ack );
            return -EIO;
         }
      }

      /* Now read the whole thing back in. */
      if ( !ReadVideoCoreMemory( display_vc_handle, &display_local, display_vc_addr, sizeof(display_local)) != 0) {
         fprintf( stderr, "Error reading %d bytes from vc addr 0x%08x\n", sizeof(display_local), display_vc_addr );
         return -EIO;
      }
   }

   return rc;
}


/****************************************************************************
 *  Control the display power mode within videocore
 *
 *  vc display power (on|off)
 ****************************************************************************/
static int do_vc_display_power(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int  rc = 0;

   if ((argc != 2) || ((strcmp (argv[1], "on") != 0) && (strcmp (argv[1], "off") != 0))) {
      printf( "Expecting display power mode\n" );
      rc = 1;
   }
   else {
      int turn_on = argv[1][1] == 'n';

      display_local.command = DISPLAY_IPC_POWER;
      display_local.request.power.new_state = turn_on ? DISPLAY_IPC_POWER_FULL : DISPLAY_IPC_POWER_OFF;
      rc = vc_display_send (0); // Don't wait.
   }

   return rc;
}

/****************************************************************************
 *  Initialise the framebuffer interface
 *
 *  vc display fb init
 ****************************************************************************/
static int do_vc_display_fb_init (uint32_t display_number)
{
   int rc = 0;

   display_local.command = DISPLAY_IPC_FB_INIT;
   display_local.display_number = display_number;

   display_local.request.fb_init.format = DISPLAY_IPC_FB_FORMAT_RGBA32;
   display_local.request.fb_init.double_buffer = 1;

   /* Request and wait for result. */
   rc = vc_display_send (1);

   if (rc == 0)
   {
      display_fb_state.width  = display_local.response.fb_init.width;
      display_fb_state.height = display_local.response.fb_init.height;
      display_fb_state.pitch  = display_local.response.fb_init.pitch;

      // TODO: convert pointers/create local buffer.
      display_fb_state.image_data[0]  = display_local.response.fb_init.image_data[0];
      display_fb_state.image_data[1]  = display_local.response.fb_init.image_data[1];
   }

   return rc;
}

/****************************************************************************
 *  Update the framebuffer
 *
 *  vc display fb update (0|1)
 ****************************************************************************/
static int do_vc_display_fb_update (uint32_t buffer)
{
   int rc = 0;

   display_local.command = DISPLAY_IPC_FB_UPDATE;
   display_local.display_number = 0; // Not used.

   display_local.request.fb_update.use_buffer = buffer;

   /* Request and wait for completion. */
   rc = vc_display_send (1);

   return rc;
}


/****************************************************************************
*
*  Control the frame buffer within videocore
*
*  vc display fb
*
***************************************************************************/

static int do_vc_display_fb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int  rc = 0;

   if ((argc == 2) && !strcmp (argv[1], "init")) {
      rc = do_vc_display_fb_init (0);
   }
   else if ((argc == 3) && !strcmp (argv[1], "update") && 
            (argv[2][1] == 0) && ((argv[2][0] == '0') || (argv[2][0] == '1'))) {
      rc = do_vc_display_fb_update (argv[2][0] - '0');
   }
   else {
      rc = 1;
   }

   return rc;
}

/****************************************************************************
*
*  vc display command table
*
***************************************************************************/

static cmd_tbl_t cmd_vc_display_sub[] = {
   U_BOOT_CMD_MKENT(power    , 2, 0, do_vc_display_power,        "", "" ),
   U_BOOT_CMD_MKENT(fb       , 2, 0, do_vc_display_fb   ,        "", "" ),
};


/****************************************************************************
*
*  do_vc_display
*
*  Syntax:
*     vc bootfs power (on|off)
*
***************************************************************************/

int do_vc_display(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   cmd_tbl_t *cp;

   if ( argc < 2 ) {
      printf( "Expecting display sub-command\n" );
      return cmd_usage( cmdtp );
   }

   /* drop initial display arg */

   argc--;
   argv++;

   cp = find_cmd_tbl(argv[0], cmd_vc_display_sub, ARRAY_SIZE(cmd_vc_display_sub));
   if (cp)
   {
      if (( flag & CMD_FLAG_REPEAT ) != 0 ) {
         /*
          * Repeat - (i.e. user just pressed RETURN). Only do it
          *          for sub commands which support it.
          */

         if ( cp->repeatable )
            return cp->cmd(cmdtp, flag, argc, argv);

         /* Otherwise behave like a no-op */
         return 0;
      }
      return cp->cmd(cmdtp, flag, argc, argv);
   }
   return cmd_usage(cmdtp);
}

