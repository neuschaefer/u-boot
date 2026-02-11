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

#define DISPLAY_IPC_DOORBELL   0xD


typedef enum
{
   DISPLAY_IPC_UNUSED,
   DISPLAY_IPC_POWER,
   DISPLAY_IPC_BRIGHTNESS,
   DISPLAY_IPC_FB_INIT,
   DISPLAY_IPC_FB_UPDATE
} DISPLAY_IPC_COMMAND_T;

typedef enum
{
   DISPLAY_IPC_FB_FORMAT_RGBA32 = 8888,
   DISPLAY_IPC_FB_FORMAT_RGB565 = 565
} DISPLAY_IPC_FB_FORMAT;

// Chosen to match DISP_POWER_STATE_T
typedef enum
{
   DISPLAY_IPC_POWER_OFF = 0,
   DISPLAY_IPC_POWER_BLANK = 1,
   DISPLAY_IPC_POWER_PARTIAL = 2,
   DISPLAY_IPC_POWER_RESERVED = 3,
   DISPLAY_IPC_POWER_FULL = 4
} DISPLAY_IPC_POWER_T;


/* All 32 bit values or pointers. */
typedef struct DISPLAY_IPC_tag
{
   uint32_t command; // From DISPLAY_IPC_COMMAND_T
   uint32_t display_number;

   union
   {
      struct
      {
         uint32_t new_state; // From DISPLAY_IPC_POWER_T
      } power;
      struct
      {
         uint32_t level;
      } brightness;
      struct
      {
         uint32_t format; // From DISPLAY_IPC_FB_FORMAT
         uint32_t double_buffer;
      } fb_init;
      struct
      {
         uint32_t use_buffer;
      } fb_update;

   } request;

   union
   {
      struct
      {
         uint32_t width;
         uint32_t height;
         uint32_t pitch;
         void * image_data[2];
      } fb_init;
   } response;

   uint32_t req;
   uint32_t ack;
} DISPLAY_IPC_T;

