/*****************************************************************************
* Copyright 2009 - 2011 Broadcom Corporation.  All rights reserved.
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

#if !defined( BOOTFS_DEFS_H )
#define BOOTFS_DEFS_H

/**
 * \file bootfs-defs.h
 *
 * This file contains structure definitions which are shared
 * between the build machine utility used to create the boot
 * filesystem, and the code running on the videocore which
 * accesses the filesystem.
 *
 * \section bootfs Boot Filesystem
 *
 * BootFS implements a filesystem which is available right from
 * early boot on the videocore. The filesystem is RAM resident,
 * and deleting files will reclaim the memory occupied by the
 * file (perhaps used for a splash screen).
 *
 * IMPORTANT NOTE: There is a copy of this header file in the
 *                 videocore repository, in the filesystem
 *                 directory.
 *
 *                 Any changes to the layout of any structures
 *                 in this file need to be reflected in that
 *                 file as well.
 */

/* ---- Include Files ----------------------------------------------------- */

//#include "interface/vcos/vcos_stdint.h"
#include <common.h>

#if defined( __VIDEOCORE__ )
#include "vcfw/rtos/common/rtos_common_mem.h"
typedef MEM_HANDLE_T                BOOTFS_MEM_HANDLE_T;
#define BOOTFS_MEM_HANDLE_INVALID   MEM_HANDLE_INVALID
#else
typedef void                       *BOOTFS_MEM_HANDLE_T;
#define BOOTFS_MEM_HANDLE_INVALID   (void *)0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Constants and Types ---------------------------------------------- */

#define BOOTFS_NAME_SIZE        32
#define BOOTFS_DESCR_SIZE       76
#define BOOTFS_MAGIC            "BOOTFS"
#define BOOTFS_TRAILER_MAGIC    "BOOTFST"
#define BOOTFS_PAD_SIZE         32          // Must be a power of 2

// Somebody else seems to be using close to 256 bytes at the end of the heap.
// For the time being, I just increased the gap, until I figure out who else
// is using the memory.

#define BOOTFS_HEAP_END_GAP 512

/** Directory Entry Structure. This structure describes an individual
 *  directory entry contained within the filesystem.
 */

typedef struct
{
    uint32_t            entry_size;                     // Size of this directory entry structure (in bytes).
    char                name[BOOTFS_NAME_SIZE];         // Null-terminated name of the file (including extension)

    uint32_t            offset;                         // Offset to file data.
    BOOTFS_MEM_HANDLE_T handle;                         // Handle to file data.
    uint32_t            file_size;                      // Size of the file, in bytes
    char                description[BOOTFS_DESCR_SIZE]; // Description of the file (i.e. type of LCD or camera driver)
    uint32_t            crc;                            // 32-bit CRC of the file.

} BOOTFS_DIR_ENTRY_T;

/** Trailer. This is appended to the image by make-bootfs
*   append-to so that it can strip out a previous bootfs image.
*/

typedef struct
{
    char                magic[8];       // The string BOOTFS followed by two null characters
    uint32_t            stored_size;    // Size of the complete filesystem (doesn't include the trailer)
    uint8_t             reserved[20];   // Pad out structure to 32 bytes.

} BOOTFS_TRAILER_T;

/** Directory. Currently there are no subdirectories, but they
 *  should be easy to implement.
 */
typedef struct
{
    char                magic[8];       // The string BOOTFS followed by two null characters
    uint32_t            stored_size;    // Size of the complete filesystem when stored in its compacted form
    uint32_t            num_entries;    // Number of directory entries in this directory.
    uint32_t            alloc_entries;  // Number of directory entries allocated in this directory.
    uint32_t            offset;         // Offset to directory entry.
    BOOTFS_MEM_HANDLE_T handle;         // Handle to directory entry.
    uint8_t             reserved[4];    // Pad out structure to 32 bytes.

} BOOTFS_DIR_T;

#ifdef __cplusplus
}
#endif

#endif /* BOOTFS_DEFS_H */

