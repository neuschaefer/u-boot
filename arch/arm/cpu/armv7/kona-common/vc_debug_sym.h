/*****************************************************************************
* Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
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
*  @file    vc_debug_sym.c
*
*  @brief   Adaptation of
*           vc4-dev/host_applications/linux/libs/debug_sym/debug_sym.h
*           and includes the VC_DEBUG_SYMBOL_T struct from
*           vc4-dev/host_support/include/vc_debug_sym.h
*
****************************************************************************/

#if !defined( VC_DEBUG_SYM_H )
#define VC_DEBUG_SYM_H

/* ---- Include Files ----------------------------------------------------- */

#include <asm/kona-common/chal_defs.h>

/* ---- Constants and Types ---------------------------------------------- */

typedef struct
{
    const char *label;
    uint32_t    addr;
    size_t      size;

} VC_DEBUG_SYMBOL_T;

typedef struct
{
    uint32_t    symbolTableOffset;
    uint32_t    magic;
    uint32_t    paramSize;
} VC_DEBUG_HEADER_T;

typedef struct
{
    uint32_t    vcMemBase;
    uint32_t    vcMemSize;
    uint32_t    vcEntryPoint;
    uint32_t    symbolTableLength;
} VC_DEBUG_PARAMS_T;

typedef struct opaque_vc_mem_access_handle_t *VC_MEM_ACCESS_HANDLE_T;

typedef uint32_t    VC_MEM_ADDR_T;

#define TO_VC_MEM_ADDR(ptr)    ((VC_MEM_ADDR_T)(unsigned long)(ptr))

#define VC_DEBUG_HEADER_MAGIC  (('V' << 0) + ('C' << 8) + ('D' << 16) + ('H' << 24))

// Offset within the videocore memory map to get the address of the debug header.
#define VC_DEBUG_HEADER_OFFSET 0x2800

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

/*
 * The following were taken from vcinclude/hardware_vc4_bigisland.h
 */

#define ALIAS_NORMAL(x)             ((void*)(((unsigned long)(x)&~0xc0000000uL)|0x00000000uL)) // normal cached data (uses main 128K L2 cache)
#define IS_ALIAS_PERIPHERAL(x)      (((unsigned long)(x)>>29)==0x3uL)

/*
 * Get access to the videocore memory space. Returns zero if the memory was
 * opened successfully, or a negative value (-errno) if the access could not
 * be obtained.
 */
int OpenVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T *handle );

/*
 * Get access to the videocore space from a file. The file might be /dev/mem, or
 * it might be saved image on disk.
 */
int OpenVideoCoreMemoryAt( uint32_t vc_base_addr, VC_MEM_ACCESS_HANDLE_T *vcHandlePtr );

/*
 * Returns the number of symbols which were detected.
 */
unsigned NumVideoCoreSymbols( VC_MEM_ACCESS_HANDLE_T handle );

/*
 * Returns the name, address and size of the i'th symbol.
 */
int GetVideoCoreSymbol( VC_MEM_ACCESS_HANDLE_T handle,
                        unsigned idx,
                        char *nameBuf,
                        size_t nameBufSize,
                        VC_MEM_ADDR_T *vcMemAddr,
                        size_t *vcMemSize );

/*
 * Looks up the named, symbol. If the symbol is found, it's value and size
 * are returned.
 *
 * Returns  true if the lookup was successful.
 */
int LookupVideoCoreSymbol( VC_MEM_ACCESS_HANDLE_T handle,
                           const char *symbol,
                           VC_MEM_ADDR_T *vcMemAddr,
                           size_t *vcMemSize );

/*
 * Looks up the named, symbol. If the symbol is found, and it's size is equal
 * to the sizeof a uint32_t, then true is returned.
 */
int LookupVideoCoreUInt32Symbol( VC_MEM_ACCESS_HANDLE_T handle,
                                 const char *symbol,
                                 VC_MEM_ADDR_T *vcMemAddr );

/*
 * Reads 'numBytes' from the videocore memory starting at 'vcMemAddr'. The
 * results are stored in 'buf'.
 *
 * Returns true if the read was successful.
 */
int ReadVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T handle,
                         void *buf,
                         VC_MEM_ADDR_T vcMemAddr,
                         size_t numBytes );

/*
 * Reads an unsigned 32-bit value from videocore memory.
 */
static inline int ReadVideoCoreUInt32( VC_MEM_ACCESS_HANDLE_T handle,
                                       uint32_t *val,
                                       VC_MEM_ADDR_T vcMemAddr )
{
    return ReadVideoCoreMemory( handle, val, vcMemAddr, sizeof( val ));
}

/*
 * Reads a block of memory using the address associated with a symbol.
 */
int ReadVideoCoreMemoryBySymbol( VC_MEM_ACCESS_HANDLE_T vcHandle,
                                 const char            *symbol,
                                 void                  *buf,
                                 size_t                 numBytes );
/*
 * Reads an unsigned 32-bit value from videocore memory.
 */
static inline int ReadVideoCoreUInt32BySymbol( VC_MEM_ACCESS_HANDLE_T handle,
                                               const char *symbol,
                                               uint32_t *val )
{
    return ReadVideoCoreMemoryBySymbol( handle, symbol, val, sizeof( val ));
}

/*
 * Looksup a string symbol by name, and reads the contents into a user
 * supplied buffer.
 */
int ReadVideoCoreStringBySymbol( VC_MEM_ACCESS_HANDLE_T handle,
                                 const char *symbol,
                                 char *buf,
                                 size_t bufSize );

/*
 * Writes 'numBytes' into the videocore memory starting at 'vcMemAddr'. The
 * data is taken from 'buf'.
 *
 * Returns true if the write was successful.
 */
int WriteVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T handle,
                          void *buf,
                          VC_MEM_ADDR_T vcMemAddr,
                          size_t numBytes );

/*
 * Writes an unsigned 32-bit value into videocore memory.
 */
static inline int WriteVideoCoreUInt32( VC_MEM_ACCESS_HANDLE_T handle,
                                        uint32_t val,
                                        VC_MEM_ADDR_T vcMemAddr )
{
    return WriteVideoCoreMemory( handle, &val, vcMemAddr, sizeof( val ));
}

/*
 * Closes the memory space opened previously via OpenVideoCoreMemory.
 */
void CloseVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T handle );

/*
 * Returns the size of the videocore memory space used by the image.
 */
size_t GetVideoCoreMemorySize( VC_MEM_ACCESS_HANDLE_T handle );

/*
 * Returns the address of the videocore memory space.
 */
uint32_t GetVideoCoreMemoryAddr( VC_MEM_ACCESS_HANDLE_T vcHandle );

/*
 * Returns the offset to the videocore memory space used by images.
 */
uint32_t GetVideoCoreMemoryBase( void );

/*
 * Sets the offset to the videocore memory space used by images.
 */
void SetVideoCoreMemoryBase( uint32_t addr );

/*
 * Returns the entry point of the videocore image.
 */
uint32_t GetVideoCoreEntryPoint( VC_MEM_ACCESS_HANDLE_T vcHandle );

/*
 * Queries videocore to get the current size of the memory.
 */
uint32_t vc_mem_get_current_size( void );

#endif /* VC_DEBUG_SYM_H */

