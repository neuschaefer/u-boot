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
*           vc4-dev/host_applications/linux/libs/debug_sym/debug_sym.c
*           fo u-boot.
*
****************************************************************************/

// ---- Include Files -------------------------------------------------------

#include <common.h>
#include <config.h>
#include <asm/kona-common/ipc.h>
#include <errno.h>
#include <malloc.h>

#include "vc_debug_sym.h"

// ---- Public Variables ----------------------------------------------------
// ---- Private Constants and Types -----------------------------------------

#ifndef PAGE_SIZE
#define PAGE_SIZE   4096
#define PAGE_MASK   (~(PAGE_SIZE - 1))
#endif

struct opaque_vc_mem_access_handle_t
{
    uint8_t            *armVcBaseAddr; /* The ARM address of VideoCore physical address 0 */
    VC_MEM_ADDR_T       vcMemBase;     /* The VideoCore address of the start of the loaded image */
    VC_MEM_ADDR_T       vcMemEnd;      /* The VideoCore address of the end of the loaded image */
    VC_MEM_ADDR_T       vcMemSize;     /* The amount of memory used by the loaded image */
    VC_MEM_ADDR_T       vcEntryPoint;  /* The entry point for the loaded image */

    VC_MEM_ADDR_T       vcSymbolTableOffset;
    unsigned            numSymbols;
    VC_DEBUG_SYMBOL_T  *symbol;
};

#if 0
#define DBG( fmt, ... )    printf( "%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__ )
#else
#define DBG( fmt, ... )
#endif

#define INFO( fmt, ... )   printf( "%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__ )
#define ERR( fmt, ... )    printf( "%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__ )

typedef enum
{
    READ_MEM,
    WRITE_MEM,
} MEM_OP_T;

// ---- Private Variables ---------------------------------------------------

static VC_MEM_ADDR_T vcMemBase = CONFIG_VCMEM_ADDR;
static uint32_t      vcMemSize;

// ---- Private Function Prototypes -----------------------------------------

// ---- Functions -----------------------------------------------------------

/****************************************************************************
*
*   Queries the videocore to get the current size of the memory.
*
***************************************************************************/

size_t vc_mem_get_current_size( void )
{
    uint32_t        wakeup_register;

    // When we run in Open Mode, we don't want to do an ipc_read_mailbox
    // so we'll assume that if vcMemSize is non-zero then it was read from
    // the VC Debug Header.

    if ( vcMemSize == 0 )
    {
        wakeup_register = ipc_query_wakeup_vc();
        if (( wakeup_register & ~1 ) == 0 )
        {
           DBG( "%s: videocore not yet loaded, skipping...", __func__ );
        }
        else
        {
           vcMemSize = ipc_read_mailbox( 0 );
        }
    }

    return vcMemSize;
}

/****************************************************************************
*
*   Get access to the videocore memory space. Returns zero if the memory was
*   opened successfully.
*
***************************************************************************/

int OpenVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T *vcHandlePtr  )
{
    return OpenVideoCoreMemoryAt( 0, vcHandlePtr );
}

/****************************************************************************
*
*   Get access to the videocore memory space. Returns zero if the memory was
*   opened successfully.
*
***************************************************************************/

int OpenVideoCoreMemoryAt( uint32_t vc_base_addr, VC_MEM_ACCESS_HANDLE_T *vcHandlePtr )
{
    int                     rc = 0;
    VC_MEM_ACCESS_HANDLE_T  newHandle;
    VC_DEBUG_HEADER_T       debugHeader;
    VC_DEBUG_PARAMS_T       debugParams;
    VC_DEBUG_SYMBOL_T       debug_sym;
    VC_MEM_ADDR_T           symAddr;
    size_t                  symTableSize;
    unsigned                symIdx;
    uint32_t                addr;

    if (( newHandle = calloc( 1, sizeof( *newHandle ))) == NULL )
    {
        return -ENOMEM;
    }

    if (vc_base_addr != 0)
    {
        SetVideoCoreMemoryBase(vc_base_addr);
    }
    else
    {
        vc_base_addr = GetVideoCoreMemoryBase();
    }

    newHandle->armVcBaseAddr = (uint8_t *)vc_base_addr;
    vc_base_addr = (uint32_t)ALIAS_NORMAL(vc_base_addr);
    newHandle->armVcBaseAddr -= vc_base_addr;

    // See if we can detect the symbol table
    addr = vc_base_addr + VC_DEBUG_HEADER_OFFSET;

    // Provide some initial memory limits to permit the debug header to be read
    newHandle->vcMemBase = TO_VC_MEM_ADDR( addr );
    newHandle->vcMemEnd  = newHandle->vcMemBase + sizeof(debugHeader) + sizeof(debugParams) - 1;

    if ( !ReadVideoCoreMemory( newHandle,
                               &debugHeader,
                               addr,
                               sizeof( debugHeader )))
    {
        ERR( "ReadVideoCoreMemory @VC_DEBUG_HEADER_OFFSET (0x%08x) failed\n", addr );
        rc = -EIO;
        goto err_exit;
    }

    memset(&debugParams, 0, sizeof(debugParams));
    debugParams.vcMemBase = vc_base_addr;
    debugParams.vcMemSize = vc_mem_get_current_size();
    debugParams.vcEntryPoint = (vc_base_addr | 0xc0000000) + 0x200;

    if ( debugHeader.magic == VC_DEBUG_HEADER_MAGIC )
    {
        int size = debugHeader.paramSize;

        if ( size < sizeof(debugParams) )
        {
            ERR( "Warning: VideoCore Debug Header is newer or corrupt (length %d)\n", size );
            size = sizeof(debugParams);
        }

        addr += sizeof(debugHeader);

        if ( !ReadVideoCoreMemory( newHandle,
                                   &debugParams,
                                   addr,
                                   size ) )
        {
           ERR( "ReadVideoCoreMemory @0x%08x failed\n", addr );
           rc = -EIO;
           goto err_exit;
        }
        vcMemSize = debugParams.vcMemSize;
    }

    newHandle->vcMemBase = debugParams.vcMemBase;
    newHandle->vcMemSize = debugParams.vcMemSize;
    newHandle->vcEntryPoint = debugParams.vcEntryPoint;
    newHandle->vcMemEnd  = debugParams.vcMemBase + debugParams.vcMemSize - 1;

    if ( newHandle->vcMemSize == 0 )
    {
        ERR( "Videocore doesn't appear to be running - vcMemSize is zero\n" );
        rc = -EIO;
        goto err_exit;
    }

    // Perform the comparison in the VC normal alias

    if ( newHandle->vcMemBase != vc_base_addr )
    {
        // But show the results in ARM space

        ERR( "Image loaded to the wrong base (0x%08x - image expects 0x%08x)\n",
            vcMemBase, (uint32_t)newHandle->armVcBaseAddr + newHandle->vcMemBase );
        rc = -EIO;
        goto err_exit;
    }

    newHandle->vcSymbolTableOffset = debugHeader.symbolTableOffset;
    DBG( "vcSymbolTableOffset = 0x%08x", newHandle->vcSymbolTableOffset );

    DBG( "Opened VC @ 0x%p for %d Mb", newHandle->armVcBaseAddr, newHandle->vcMemSize >> 20 );

    // Make sure that the pointer points into the first few megabytes of
    // the memory space.

    if ( (newHandle->vcSymbolTableOffset - newHandle->vcMemBase) > ( 4 * 1024 * 1024 ))
    {
        ERR( "newHandle->vcSymbolTableOffset (%d) > 4Mb\n", newHandle->vcSymbolTableOffset );
        rc = -EIO;
        goto err_exit;
    }

    // Make a pass to count how many symbols there are.

    symAddr = newHandle->vcSymbolTableOffset;
    newHandle->numSymbols = 0;
    do
    {
        if ( !ReadVideoCoreMemory( newHandle,
                                   &debug_sym,
                                   symAddr,
                                   sizeof( debug_sym )))
        {
            ERR( "ReadVideoCoreMemory @ symAddr(0x%08x) failed\n", symAddr );
            rc = -EIO;
            goto err_exit;
        }

        newHandle->numSymbols++;

        DBG( "Symbol %d: label: 0x%p addr: 0x%08x size: %zu",
             newHandle->numSymbols,
             debug_sym.label,
             debug_sym.addr,
             debug_sym.size );

        if ( newHandle->numSymbols > 1024 )
        {
            // Something isn't sane.

            ERR( "numSymbols (%d) > 1024 - looks wrong\n", newHandle->numSymbols );
            rc = -EIO;
            goto err_exit;
        }
        symAddr += sizeof( debug_sym );

    } while ( debug_sym.label != 0 );
    newHandle->numSymbols--;

    DBG( "Detected %d symbols", newHandle->numSymbols );

    // Allocate some memory to hold the symbols, and read them in.

    symTableSize = newHandle->numSymbols * sizeof( debug_sym );
    if (( newHandle->symbol = malloc( symTableSize )) == NULL )
    {
        rc = -ENOMEM;
        goto err_exit;
    }
    if ( !ReadVideoCoreMemory( newHandle,
                               newHandle->symbol,
                               newHandle->vcSymbolTableOffset,
                               symTableSize ))
    {
        ERR( "ReadVideoCoreMemory @ newHandle->vcSymbolTableOffset(0x%08x) failed\n", newHandle->vcSymbolTableOffset );
        rc = -EIO;
        goto err_exit;
    }

    // The names of the symbols are pointers in videocore space. We want
    // to have them available locally, so we make copies and fixup
    // the pointer.

    for ( symIdx = 0; symIdx < newHandle->numSymbols; symIdx++ )
    {
        VC_DEBUG_SYMBOL_T   *sym;
        char                 symName[ 256 ];

        sym = &newHandle->symbol[ symIdx ];

        DBG( "Symbol %d: label: 0x%p addr: 0x%08x size: %zu",
             symIdx,
             sym->label,
             sym->addr,
             sym->size );

        if ( !ReadVideoCoreMemory( newHandle,
                                   symName,
                                   TO_VC_MEM_ADDR(sym->label),
                                   sizeof( symName )))
        {
            ERR( "ReadVideoCoreMemory @ sym->label(0x%08x) failed\n", sym->addr );
            rc = -EIO;
            goto err_exit;
        }
        symName[ sizeof( symName ) - 1 ] = '\0';
        *((const char **)&sym->label) = strdup( symName );

        DBG( "Symbol %d (@0x%p): label: '%s' addr: 0x%08x size: %zu",
             symIdx,
             sym,
             sym->label,
             sym->addr,
             sym->size );
    }

    *vcHandlePtr = newHandle;
    return 0;

err_exit:
    free( newHandle );

    return rc;
}

/****************************************************************************
*
*   Returns the number of symbols which were detected.
*
***************************************************************************/

unsigned NumVideoCoreSymbols( VC_MEM_ACCESS_HANDLE_T vcHandle )
{
    return vcHandle->numSymbols;
}

/****************************************************************************
*
*   Returns the name, address and size of the i'th symbol.
*
***************************************************************************/

int GetVideoCoreSymbol( VC_MEM_ACCESS_HANDLE_T vcHandle, unsigned idx, char *labelBuf, size_t labelBufSize, VC_MEM_ADDR_T *vcMemAddr, size_t *vcMemSize )
{
    VC_DEBUG_SYMBOL_T   *sym;

    if ( idx >= vcHandle->numSymbols )
    {
        return -EINVAL;
    }
    sym = &vcHandle->symbol[ idx ];

    strncpy( labelBuf, sym->label, labelBufSize );
    labelBuf[labelBufSize - 1] = '\0';

    if ( vcMemAddr != NULL )
    {
        *vcMemAddr = (VC_MEM_ADDR_T)sym->addr;
    }
    if ( vcMemSize != NULL )
    {
        *vcMemSize = sym->size;
    }

    return 0;
}

/****************************************************************************
*
*   Looks up the named, symbol. If the symbol is found, it's value and size
*   are returned.
*
*   Returns  true if the lookup was successful.
*
***************************************************************************/

int LookupVideoCoreSymbol( VC_MEM_ACCESS_HANDLE_T vcHandle, const char *symbol, VC_MEM_ADDR_T *vcMemAddr, size_t *vcMemSize )
{
    unsigned        idx;
    char            symName[ 64 ];
    VC_MEM_ADDR_T   symAddr;
    size_t          symSize;

    for ( idx = 0; idx < vcHandle->numSymbols; idx++ )
    {
        GetVideoCoreSymbol( vcHandle, idx, symName, sizeof( symName ), &symAddr, &symSize );
        if ( strcmp( symbol, symName ) == 0 )
        {
            if ( vcMemAddr != NULL )
            {
                *vcMemAddr = symAddr;
            }
            if ( vcMemSize != 0 )
            {
                *vcMemSize = symSize;
            }

            DBG( "%s found, addr = 0x%08x size = %zu", symbol, symAddr, symSize );
            return 1;
        }
    }

    if ( vcMemAddr != NULL )
    {
        *vcMemAddr = 0;
    }
    if ( vcMemSize != 0 )
    {
        *vcMemSize = 0;
    }
    DBG( "%s not found", symbol );
    return 0;
}

/****************************************************************************
*
*   Looks up the named, symbol. If the symbol is found, and it's size is equal
*   to the sizeof a uint32_t, then true is returned.
*
***************************************************************************/

int LookupVideoCoreUInt32Symbol( VC_MEM_ACCESS_HANDLE_T vcHandle,
                                 const char *symbol,
                                 VC_MEM_ADDR_T *vcMemAddr )
{
    size_t  vcMemSize;

    if ( !LookupVideoCoreSymbol( vcHandle, symbol, vcMemAddr, &vcMemSize ))
    {
        return 0;
    }

    if ( vcMemSize != sizeof( uint32_t ))
    {
        ERR( "Symbol: '%s' has a size of %zu, expecting %zu", symbol, vcMemSize, sizeof( uint32_t ));
        return 0;
    }
    return 1;
}

/****************************************************************************
*
*   Does Reads or Writes on the videocore memory.
*
***************************************************************************/

static int AccessVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T vcHandle,
                                  MEM_OP_T               mem_op,
                                  void                  *buf,
                                  VC_MEM_ADDR_T          vcMemAddr,
                                  size_t                 numBytes )
{
    DBG( "%s %zu bytes @ 0x%08x", mem_op == WRITE_MEM ? "Write" : "Read", numBytes, vcMemAddr );

    /*
     * Since we'll be passed videocore pointers, we need to deal with the high bits.
     *
     * We need to strip off the high 2 bits to convert to a physical address, except
     * for when the high 3 bits are equal to 011, which means that it corresponds to
     * a peripheral and isn't accessible.
     */

    if ( IS_ALIAS_PERIPHERAL( vcMemAddr ))
    {
        // This is a peripheral address.

        ERR( "Can't access peripheral address 0x%08x", vcMemAddr );
        return 0;
    }
    vcMemAddr = TO_VC_MEM_ADDR(ALIAS_NORMAL( vcMemAddr ));

    if ( (vcMemAddr < vcHandle->vcMemBase) ||
         (vcMemAddr > vcHandle->vcMemEnd) )
    {
        ERR( "Memory address 0x%08x is outside range 0x%08x-0x%08x", vcMemAddr,
             vcHandle->vcMemBase, vcHandle->vcMemEnd );
        return 0;
    }
    if (( vcMemAddr + numBytes - 1) > vcHandle->vcMemEnd )
    {
        ERR( "Memory address 0x%08x + numBytes 0x%08zx is > memory end 0x%08x",
             vcMemAddr, numBytes, vcHandle->vcMemEnd );
        return 0;
    }

    if ( mem_op == WRITE_MEM )
    {
       memcpy( vcHandle->armVcBaseAddr + vcMemAddr, buf, numBytes );
    }
    else
    {
       memcpy( buf, vcHandle->armVcBaseAddr + vcMemAddr, numBytes );
    }

    return 1;
}


/****************************************************************************
*
*   Reads 'numBytes' from the videocore memory starting at 'vcMemAddr'. The
*   results are stored in 'buf'.
*
*   Returns true if the read was successful.
*
***************************************************************************/

int ReadVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T vcHandle, void *buf, VC_MEM_ADDR_T vcMemAddr, size_t numBytes )
{
    return AccessVideoCoreMemory( vcHandle, READ_MEM, buf, vcMemAddr, numBytes );
}

/****************************************************************************
*
*   Reads 'numBytes' from the videocore memory starting at 'vcMemAddr'. The
*   results are stored in 'buf'.
*
*   Returns true if the read was successful.
*
***************************************************************************/

int ReadVideoCoreMemoryBySymbol( VC_MEM_ACCESS_HANDLE_T vcHandle, const char *symbol, void *buf, size_t bufSize )
{
    VC_MEM_ADDR_T   vcMemAddr;
    size_t          vcMemSize;

    if ( !LookupVideoCoreSymbol( vcHandle, symbol, &vcMemAddr, &vcMemSize ))
    {
        ERR( "Symbol not found: '%s'", symbol );
        return 0;
    }

    if ( vcMemSize > bufSize )
    {
        vcMemSize = bufSize;
    }

    if ( !ReadVideoCoreMemory( vcHandle, buf, vcMemAddr, vcMemSize ))
    {
        ERR( "Unable to read %zu bytes @ 0x%08x", vcMemSize, vcMemAddr );
        return 0;
    }
    return 1;
}

/****************************************************************************
*
*   Looks up a symbol and reads the contents into a user supplied buffer.
*
*   Returns true if the read was successful.
*
***************************************************************************/

int ReadVideoCoreStringBySymbol( VC_MEM_ACCESS_HANDLE_T vcHandle,
                                 const char *symbol,
                                 char *buf,
                                 size_t bufSize )
{
    VC_MEM_ADDR_T   vcMemAddr;
    size_t          vcMemSize;

    if ( !LookupVideoCoreSymbol( vcHandle, symbol, &vcMemAddr, &vcMemSize ))
    {
        ERR( "Symbol not found: '%s'", symbol );
        return 0;
    }

    if ( vcMemSize > bufSize )
    {
        vcMemSize = bufSize;
    }

    if ( !ReadVideoCoreMemory( vcHandle, buf, vcMemAddr, vcMemSize ))
    {
        ERR( "Unable to read %zu bytes @ 0x%08x", vcMemSize, vcMemAddr );
        return 0;
    }

    // Make sure that the result is null-terminated

    buf[vcMemSize-1] = '\0';
    return 1;
}

/****************************************************************************
*
*   Writes 'numBytes' into the videocore memory starting at 'vcMemAddr'. The
*   data is taken from 'buf'.
*
*   Returns true if the write was successful.
*
***************************************************************************/

int WriteVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T vcHandle,
                          void *buf,
                          VC_MEM_ADDR_T vcMemAddr,
                          size_t numBytes )
{
    return AccessVideoCoreMemory( vcHandle, WRITE_MEM, buf, vcMemAddr, numBytes );
}

/****************************************************************************
*
*   Closes the memory space opened previously via OpenVideoCoreMemory.
*
***************************************************************************/

void CloseVideoCoreMemory( VC_MEM_ACCESS_HANDLE_T vcHandle )
{
    if ( vcHandle->symbol != NULL )
    {
        free( vcHandle->symbol );
    }

    free( vcHandle );
}

/****************************************************************************
*
*   Returns the size of the videocore memory space.
*
***************************************************************************/

size_t GetVideoCoreMemorySize( VC_MEM_ACCESS_HANDLE_T vcHandle )
{
    return vcHandle->vcMemSize;
}

/****************************************************************************
*
*   Returns the base address of the videocore space.
*
***************************************************************************/

uint32_t GetVideoCoreMemoryAddr( VC_MEM_ACCESS_HANDLE_T vcHandle )
{
    return (uint32_t)vcHandle->armVcBaseAddr;
}

/****************************************************************************
*
*  Returns the offset to the videocore memory space used by images.
*
***************************************************************************/
uint32_t GetVideoCoreMemoryBase( void )
{
   return vcMemBase;
}

/****************************************************************************
*
*  Sets the offset to the videocore memory space used by images.
*
***************************************************************************/
void SetVideoCoreMemoryBase( uint32_t addr )
{
   vcMemBase = addr;
}

/****************************************************************************
*
*   Returns the entry point of the videocore image.
*
***************************************************************************/

uint32_t GetVideoCoreEntryPoint( VC_MEM_ACCESS_HANDLE_T vcHandle )
{
    return (uint32_t)vcHandle->vcEntryPoint;
}
