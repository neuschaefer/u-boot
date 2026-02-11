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
*  @file    cmd_vcbootfs.c
*
*  @brief   Implements the vc bootfs sub-command.
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <common.h>
#include <command.h>
#include <errno.h>

#ifdef CONFIG_OF_LIBFDT_CMD
#include <fdt.h>
#include <libfdt.h>
#endif

#include "cmd_vc_bootfs.h"
#include "bootfs-defs.h"
#include "vc_debug_sym.h"

/* ---- Private Constants and Types -------------------------------------- */

typedef struct
{
   VC_MEM_ACCESS_HANDLE_T  vc_hndl;
   VC_MEM_ADDR_T           dir_vc_addr;
   BOOTFS_DIR_T            dir;

} BOOTFS_HANDLE_T;

/* ---- Private Variables ------------------------------------------------ */
/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
*
*  Verifies that a BootFS image is in fact present, and if so gathers
*  information about the image.
*
***************************************************************************/

static int bootfs_open( BOOTFS_HANDLE_T *handle )
{
   int                     rc;
   size_t                  vc_mem_size;
   const char *            bootfs_sym_name = "__BOOTFS_INITIAL_LOCATION";

   memset( handle, 0, sizeof( *handle ));

   if (( rc = OpenVideoCoreMemory( &handle->vc_hndl )) != 0 ) {
      printf( "%s: OpenVideoCoreMemory failed: %d\n", __func__, rc );
      return rc;
   }

   if ( LookupVideoCoreSymbol( handle->vc_hndl, bootfs_sym_name,
                               &handle->dir_vc_addr, &vc_mem_size ) < 0 ) {
      fprintf( stderr, "Symbol '%s' not found\n", bootfs_sym_name );
      rc = -ENOENT;
      goto err;
   }

   if ( vc_mem_size != sizeof( handle->dir )) {
      fprintf( stderr, "%s has a size of %d bytes, expecting %d\n",
               bootfs_sym_name, vc_mem_size, sizeof( handle->dir ));
      rc = -EIO;
      goto err;
   }

   if ( !ReadVideoCoreMemory( handle->vc_hndl, &handle->dir, handle->dir_vc_addr, vc_mem_size ))
   {
      fprintf( stderr, "Error reading %d bytes from vc addr 0x%08x (for %s)\n",
               vc_mem_size, handle->dir_vc_addr, bootfs_sym_name );
      rc = -EIO;
      goto err;
   }

   if ( strncmp( BOOTFS_MAGIC,
                 handle->dir.magic,
                 sizeof( handle->dir.magic )) != 0 ) {
      fprintf( stderr, "No Boot filesystem detected (magic failed)\n" );
      fprintf( stderr, "The videocore firmware has to be loaded but not\n" );
      fprintf( stderr, "started for bootfs commands to work properly.\n");
      rc = -EIO;
      goto err;
   }

   return 0;

err:

   CloseVideoCoreMemory( handle->vc_hndl );
   return rc;
}

/****************************************************************************
*
*  Reads a directory entry from the BootFS filesystem
*
***************************************************************************/

static int bootfs_read_dir_entry( BOOTFS_HANDLE_T *handle, uint32_t dir_idx, BOOTFS_DIR_ENTRY_T *dir_entry )
{
   VC_MEM_ADDR_T  dir_entry_vc_addr;

   if ( dir_idx >= handle->dir.num_entries ) {
      fprintf( stderr, "Directory index of %u exceeds number of entries: %u\n",
               dir_idx, handle->dir.num_entries );
      return -ENOENT;
   }

   dir_entry_vc_addr = handle->dir_vc_addr + handle->dir.offset;
   dir_entry_vc_addr += (dir_idx * sizeof( *dir_entry ));

   if ( !ReadVideoCoreMemory( handle->vc_hndl, dir_entry, dir_entry_vc_addr, sizeof( *dir_entry )))
   {
      fprintf( stderr, "Error reading %d bytes from vc addr 0x%08x fort dir_idx = %u\n",
               sizeof( *dir_entry ), dir_entry_vc_addr, dir_idx );
      return -EIO;
   }

   return 0;
}

/****************************************************************************
*
*  Writes a directory entry into the BootFS filesystem
*
***************************************************************************/

static int bootfs_write_dir_entry( BOOTFS_HANDLE_T *handle, uint32_t dir_idx, BOOTFS_DIR_ENTRY_T *dir_entry )
{
   VC_MEM_ADDR_T  dir_entry_vc_addr;

   if ( dir_idx >= handle->dir.num_entries ) {
      fprintf( stderr, "Directory index of %u exceeds number of entries: %u\n",
               dir_idx, handle->dir.num_entries );
      return -ENOENT;
   }

   dir_entry_vc_addr = handle->dir_vc_addr + handle->dir.offset;
   dir_entry_vc_addr += (dir_idx * sizeof( *dir_entry ));

   if ( !WriteVideoCoreMemory( handle->vc_hndl, dir_entry, dir_entry_vc_addr, sizeof( *dir_entry )))
   {
      fprintf( stderr, "Error writing %d bytes from vc addr 0x%08x fort dir_idx = %u\n",
               sizeof( *dir_entry ), dir_entry_vc_addr, dir_idx );
      return -EIO;
   }

   return 0;
}

/****************************************************************************
*
*  Calculates the crc for a file.
*
***************************************************************************/

static uint32_t bootfs_calc_crc( BOOTFS_HANDLE_T *handle, BOOTFS_DIR_ENTRY_T *dir_entry )
{
   uint8_t        buf[ 1024 ];
   size_t         bytes_remaining;
   size_t         bytes_this_time;
   uint32_t       crc = ~0;
   VC_MEM_ADDR_T  addr;

   addr = handle->dir_vc_addr + dir_entry->offset;

   bytes_remaining = dir_entry->file_size;

   while ( bytes_remaining > 0 ) {

      bytes_this_time = bytes_remaining;
      if ( bytes_this_time > sizeof( buf )) {
         bytes_this_time = sizeof( buf );
      }

      if ( !ReadVideoCoreMemory( handle->vc_hndl, buf, addr, bytes_this_time )) {
         fprintf( stderr, "%s: read of %d bytes from vc address 0x%08x failed\n",
                  __func__, bytes_this_time, addr );
         return -EIO;
      }
      crc = crc32_no_comp( crc, buf, bytes_this_time );

      addr += bytes_this_time;
      bytes_remaining -= bytes_this_time;
   }

   return ~crc;
}

/****************************************************************************
*
*  moves memory around. This code deals with overlaps. Right now it copies
*  uses a temp buffer. This could be optimized by adding a memory mover
*  function in vc_debug_sym.c
*
***************************************************************************/

static int bootfs_memmove( BOOTFS_HANDLE_T *handle, VC_MEM_ADDR_T src, VC_MEM_ADDR_T dst, size_t num_bytes )
{
   uint8_t  buf[ 1024 ];
   int      move_forward = 1;

   if ( src < dst ) {
      /* We need to start at the end and work backwards */

      src += num_bytes;
      dst += num_bytes;

      move_forward = 0;
   }

   while ( num_bytes > 0 ) {

      size_t   bytes_this_time;

      bytes_this_time = num_bytes;
      if ( bytes_this_time > sizeof( buf )) {
         bytes_this_time = sizeof( buf );
      }

      if ( !move_forward ) {
         src -= bytes_this_time;
         dst -= bytes_this_time;
      }

      if ( !ReadVideoCoreMemory( handle->vc_hndl, buf, src, bytes_this_time )) {
         fprintf( stderr, "%s: read of %d bytes from vc address 0x%08x failed\n",
                  __func__, bytes_this_time, src );
         return -EIO;
      }

      if ( !WriteVideoCoreMemory( handle->vc_hndl, buf, dst, bytes_this_time )) {
         fprintf( stderr, "%s: write of %d bytes from vc address 0x%08x failed\n",
                  __func__, bytes_this_time, dst );
         return -EIO;
      }

      if ( move_forward ) {
         src += bytes_this_time;
         dst += bytes_this_time;
      }
      num_bytes -= bytes_this_time;
   }

   return 0;
}

/****************************************************************************
*
*  Closes a previously opened bootfs handle.
*
***************************************************************************/

static void bootfs_close( BOOTFS_HANDLE_T *handle )
{
   CloseVideoCoreMemory( handle->vc_hndl );
   memset( handle, 0, sizeof( *handle ));
}

/****************************************************************************
*
*  Adds a file to the bootfs filesystem.
*
***************************************************************************/

static int bootfs_add_file( BOOTFS_HANDLE_T *handle, const char *filename,
                            void *file_data, size_t file_size, const char *descr )
{
   int                  rc;
   BOOTFS_TRAILER_T     trailer;
   size_t               padded_file_size;
   BOOTFS_DIR_ENTRY_T   new_dir_entry;
   size_t               new_dir_entry_offset;
   VC_MEM_ADDR_T        new_dir_entry_addr;
   uint32_t             i;

   /*
    * The BootFS filesystem is arranged as follows:
    *
    * 1 x BOOTFS_DIR_T
    * n x BOOTFS_DIR_ENTRY_T
    * n x file data
    * 1 x BOOTFS_TRAILER_T
    *
    * Each file is padded to be a multiple of BOOTFS_PAD_SIZE bytes.
    */

   new_dir_entry_offset = handle->dir.offset
                        + ( handle->dir.num_entries * sizeof( new_dir_entry ));

   new_dir_entry_addr = handle->dir_vc_addr + new_dir_entry_offset;

   /*
    * Construct the new directory entry.
    */

   memset( &new_dir_entry, 0, sizeof( new_dir_entry ));
   new_dir_entry.entry_size = sizeof( new_dir_entry );
   strncpy( new_dir_entry.name, filename, sizeof( new_dir_entry.name ) - 1 );
   new_dir_entry.offset = handle->dir.stored_size + sizeof( new_dir_entry );
   new_dir_entry.file_size = file_size;
   strncpy( new_dir_entry.description, descr, sizeof( new_dir_entry.description ) - 1 );
   new_dir_entry.crc = crc32( 0, file_data, file_size );

   padded_file_size = ( file_size + ( BOOTFS_PAD_SIZE - 1 )) & ~( BOOTFS_PAD_SIZE - 1 );

   /*
    * Write out a new trailer.
    */

   memset( &trailer, 0, sizeof( trailer ));
   strcpy( trailer.magic, BOOTFS_TRAILER_MAGIC );
   trailer.stored_size = new_dir_entry.offset + padded_file_size;

   if ( !WriteVideoCoreMemory( handle->vc_hndl, &trailer,
                               handle->dir_vc_addr + trailer.stored_size,
                               sizeof( trailer ))) {
      fprintf( stderr, "%s: failed to write trailed to vc addr 0x%08x for %d bytes\n",
               __func__, handle->dir_vc_addr + trailer.stored_size,
               sizeof( trailer ));
      return -EIO;
   }

   /*
    * Write out the new file data.
    */

   if ( !WriteVideoCoreMemory( handle->vc_hndl, file_data,
                               handle->dir_vc_addr + new_dir_entry.offset,
                               file_size )) {
      fprintf( stderr, "%s: failed to new file data to vc addr 0x%08x for %d bytes\n",
               __func__, handle->dir_vc_addr + new_dir_entry.offset,
               file_size );
      return -EIO;
   }

   /*
    * Move the file data for all of the existing files down by a directory entry.
    */

   if (( rc = bootfs_memmove( handle,
                              new_dir_entry_addr,
                              new_dir_entry_addr + sizeof( new_dir_entry ),
                              handle->dir.stored_size - new_dir_entry_offset ))) {
      return rc;
   }

   /*
    * Update the offsets of the existing files.
    */

   for ( i = 0; i < handle->dir.num_entries; i++ ) {

      BOOTFS_DIR_ENTRY_T   dir_entry;

      if (( rc = bootfs_read_dir_entry( handle, i, &dir_entry )) != 0 ) {
         return rc;
      }

      dir_entry.offset += sizeof( dir_entry );

      if (( rc = bootfs_write_dir_entry( handle, i, &dir_entry )) != 0 ) {
         return rc;
      }
   }

   /*
    * Write out the new directory entry (update dir first so it will pass sanity tests)
    */

   handle->dir.num_entries++;
   handle->dir.alloc_entries++;
   handle->dir.stored_size = trailer.stored_size;

   if (( rc = bootfs_write_dir_entry( handle, handle->dir.num_entries - 1, &new_dir_entry )) != 0 ) {
      return rc;
   }

   /*
    * Update the main directory
    */

   if ( !WriteVideoCoreMemory( handle->vc_hndl, &handle->dir,
                               handle->dir_vc_addr, sizeof( handle->dir ))) {
      fprintf( stderr, "%s: failed to write directory header to vc addr 0x%08x for %d bytes\n",
               __func__, handle->dir_vc_addr, sizeof( handle->dir ));
      return -EIO;
   }

   return 0;
}

/****************************************************************************
*
*  Add a file to the videocore firmware BootFS
*
*  vc bootfs add filename addr len [descr]
*
***************************************************************************/

static int do_vc_bootfs_add(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   BOOTFS_HANDLE_T   bootfs;
   const char       *filename;
   void             *file_data;
   size_t            file_size;
   const char       *descr;
   int               rc = 0;

   if (( argc < 4 ) || ( argc > 5 )) {
      fprintf( stderr, "Expecting 3 or 4 arguments to the add command\n" );
      return 1;
   }
   filename = argv[1];
   file_data = (void *)simple_strtoul(argv[2], NULL, 16);
   file_size = simple_strtoul(argv[3], NULL, 16);;
   if (argc == 5 ) {
      descr = argv[4];
   }
   else
   {
      descr = "";
   }

   if ( bootfs_open( &bootfs ) != 0 ) {
      return 1;
   }

   if ( bootfs_add_file( &bootfs, filename, file_data, file_size, descr ) != 0 )
   {
      rc = 1;
   }

   bootfs_close( &bootfs );

   return rc;
}

/****************************************************************************
*
*  Add the dt-blob to the videocore firmware BootFS
*
*  vc bootfs add_dtblob
*
***************************************************************************/

static int do_vc_bootfs_add_dtblob(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_OF_LIBFDT_CMD
   int               rc;
   size_t            fdt_size;
   BOOTFS_HANDLE_T   bootfs;

   /*
    * First check to see if there is a working fdt (flattened device tree).
    */

   if ( working_fdt == NULL ) {
      fprintf( stderr, "No working fdt\n" );
      return 1;
   }

   if ( fdt_check_header( working_fdt ) != 0 ) {
      fprintf( stderr, "Working fdt @0x%08lx not valid (i.e. no dt-blob)\n", (unsigned long)working_fdt );
      return 1;
   }

   fdt_size = fdt_totalsize( working_fdt );

   if ( bootfs_open( &bootfs ) != 0 ) {
      return 1;
   }

   if ( bootfs_add_file( &bootfs, "dt-blob.bin", working_fdt, fdt_size, "flattened device tree" ) != 0 )
   {
      rc = 1;
   }

   bootfs_close( &bootfs );

   return rc;
#else
   fprintf( stderr, "Need CONFIG_OF_LIBFDT_CMD enabled to add dt-blob\n" );
   return 1;
#endif
}

/****************************************************************************
*
*  Dump header information about the videocore firmware BootFS
*
*  vc bootfs dump
*
***************************************************************************/

static int do_vc_bootfs_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   BOOTFS_HANDLE_T   bootfs;
   uint32_t          i;

   if ( bootfs_open( &bootfs ) != 0 ) {
      return 1;
   }

   printf( "bootfs_dir_vc_addr = 0x%08x\n", bootfs.dir_vc_addr );
   printf( "magic              = '%s'\n", bootfs.dir.magic );
   printf( "stored_size        = 0x%08x\n", bootfs.dir.stored_size );
   printf( "num_entries        = %d (0x%x)\n", bootfs.dir.num_entries, bootfs.dir.num_entries );
   printf( "alloc_entries      = %d (0x%x)\n", bootfs.dir.alloc_entries, bootfs.dir.alloc_entries );
   printf( "offset             = 0x%08x\n", bootfs.dir.offset );
   printf( "\n" );
   printf( "ESz Name                              Offset       Size  CRC      Description\n" );
   printf( "--- ------------------------------- ---------- --------- -------- -------------------------------------\n" );

   for ( i = 0; i < bootfs.dir.num_entries; i++ )
   {
      BOOTFS_DIR_ENTRY_T   dir_entry;
      uint32_t             crc;

      if ( bootfs_read_dir_entry( &bootfs, i, &dir_entry ) != 0 ) {
         return 1;
      }

      crc = bootfs_calc_crc( &bootfs, &dir_entry );
      if ( crc != dir_entry.crc ) {
         sprintf( dir_entry.description,
                   "Found crc: 0x%08x, expecting 0x%08x", crc, dir_entry.crc );
      }

      printf( "%3d %-31s 0x%08x %9u %08x %s\n",
              dir_entry.entry_size,
              dir_entry.name,
              dir_entry.offset,
              dir_entry.file_size,
              dir_entry.crc,
              dir_entry.description );
   }

   bootfs_close( &bootfs );

   return 0;
}

/****************************************************************************
*
*  List files contained in the videocore firmware BootFS
*
*  vc bootfs list
*
***************************************************************************/

static int do_vc_bootfs_list(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   BOOTFS_HANDLE_T      bootfs;
   uint32_t             i;

   if ( bootfs_open( &bootfs ) != 0 ) {
      return 1;
   }

   printf( "Name                                Size       CRC Description\n" );
   printf( "------------------------------- --------- -------- -------------------------------------\n" );

   for ( i = 0; i < bootfs.dir.num_entries; i++ )
   {
      BOOTFS_DIR_ENTRY_T   dir_entry;
      uint32_t             crc;

      if ( bootfs_read_dir_entry( &bootfs, i, &dir_entry ) != 0 ) {
         return 1;
      }

      crc = bootfs_calc_crc( &bootfs, &dir_entry );
      if ( crc != dir_entry.crc ) {
         sprintf( dir_entry.description,
                   "Found crc: 0x%08x, expecting 0x%08x", crc, dir_entry.crc );
      }

      printf( "%-31s %9u %08x %s\n",
              dir_entry.name,
              dir_entry.file_size,
              dir_entry.crc,
              dir_entry.description );
   }

   bootfs_close( &bootfs );

   return 0;
}

/****************************************************************************
*
*  vc bootfs command table
*
***************************************************************************/

static cmd_tbl_t cmd_vc_bootfs_sub[] = {
   U_BOOT_CMD_MKENT(add,         5, 0, do_vc_bootfs_add,          "", "" ),
   U_BOOT_CMD_MKENT(add_dtblob,  1, 0, do_vc_bootfs_add_dtblob,   "", "" ),
   U_BOOT_CMD_MKENT(dump,        1, 0, do_vc_bootfs_dump,         "", "" ),
   U_BOOT_CMD_MKENT(list,        1, 0, do_vc_bootfs_list,         "", "" ),
};


/****************************************************************************
*
*  do_vc_bootfs
*
*  Syntax:
*     vc bootfs list
*     vc bootfs add filename addr len [descr]
*     vc bootfs add-dt-blob
*     vc bootfs del filename
*
***************************************************************************/

int do_vc_bootfs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   cmd_tbl_t *cp;

   if ( argc < 2 ) {
      printf( "Expecting bootfs sub-command\n" );
      return cmd_usage( cmdtp );
   }

   /* drop initial bootfs arg */

   argc--;
   argv++;

   cp = find_cmd_tbl(argv[0], cmd_vc_bootfs_sub, ARRAY_SIZE(cmd_vc_bootfs_sub));
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

