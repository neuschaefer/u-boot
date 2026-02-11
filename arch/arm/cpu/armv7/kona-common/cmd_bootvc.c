#include <common.h>
#include <command.h>
#include <image.h>
#include <mmc.h>
#include <gpt.h>
#include <malloc.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/kona-common/ipc.h>
#include <config.h>
#include <errno.h>
#include <linux/ctype.h>

#include "vc_debug_sym.h"
#include "cmd_vc_bootfs.h"
#include "cmd_vc_display.h"

#if defined( CONFIG_VC_LCD_GPIOMUX ) || defined( CONFIG_VC_JTAG_GPIOMUX )
#include <asm/arch/gpiomux.h>
#endif

#if 0
#define DBG( fmt, ... )    printf( "%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__ )
#else
#define DBG( fmt, ... )
#endif

/*
 * Some structures copied from vc4-dev-vcfw/logging/logging.h
 */

typedef enum {
   LOGGING_FIFO_LOG = 1,
   LOGGING_ASSERTION_LOG,
   LOGGING_TASK_LOG,
} LOG_FORMAT_T;

typedef struct {
   LOG_FORMAT_T type;
   void *log;
} LOG_DESCRIPTOR_T;

// This header prefixes the logged data on every fifo log entry.
typedef struct {
   unsigned long time;
   unsigned short seq_num;    // if two entries have the same timestamp then this seq num differentiates them.
   unsigned short size;       // = size of entire log entry = header + data
} logging_fifo_log_msg_header_t;

// This describes the state of a fifo type log.
typedef struct {
   char name[4];
   unsigned char *start;      // Start and End are defined when the log is created.
   unsigned char *end;
   unsigned char *ptr;        // Always points to where the next contents will be written.
   unsigned char *next_msg;   // Points to the first entry in the fifo. Is updated when the fifo gets full and overwrites the oldest entry.
   logging_fifo_log_msg_header_t msg_header;    // used to keep track of forming a log entry between calls to _start and _end.
} logging_fifo_log_t;

// This describes the state of an array type log.
typedef struct {
   char name[4];
   unsigned short nitems, item_size;
   unsigned long available;
   unsigned char *data;
} logging_array_log_t;

typedef unsigned long LOGGING_LOG_TYPE_T;

typedef struct {
   unsigned long sync;
   LOGGING_LOG_TYPE_T type;
   unsigned long version;
   void *self;
} logging_common_header_t;

// This is the top level data structure for the Videocore Logs. It houses several
// logs from different sources as well as extra state info.
typedef struct {
   logging_common_header_t common;
   unsigned long stc;
   int task_switch_log_size;
   unsigned char *task_switch_log;
   unsigned long n_logs;
   LOG_DESCRIPTOR_T assertion_log;
   LOG_DESCRIPTOR_T message_log;
   LOG_DESCRIPTOR_T task_log;

} logging_header_t;

/*******************************************************************/
/* bootvc - boot vc image from image in memory */
/*******************************************************************/

#if defined( CONFIG_CAPRI )
extern int capri_start_vc (ulong entry);
#else
static int island_start_vc(ulong entry)   // island only
{
   static int vc_init = 0; // Don't allow multiple boots.

   int rc = 0;

#if defined( CONFIG_VC_LCD_GPIOMUX )
    {
        gpiomux_rc_e    gpiomux_rc;

        // Configure the LCD data pins as coming from the videocore.

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

        // Configure the Videocore JTAG pins (so we can use the debugger)

        gpiomux_rc = gpiomux_requestGroup( CONFIG_VC_JTAG_GPIOMUX, CONFIG_VC_JTAG_GPIOMUX_ID, "vc-jtag" );
        if ( gpiomux_rc != gpiomux_rc_SUCCESS )
        {
            printf( "Request to mux gpio pins for vc jtag failed: %d\n", gpiomux_rc );
            return 1;
        }
        printf("Configuring VC JTAG mux\n");
    }
#endif

    // Make sure that display is off. We enable the power here to give it a chance
    // to come up before we take the LCD out of reset.

#if defined( CONFIG_VC_LCD_BL_PWM )
    gpio_request( CONFIG_VC_LCD_BL_PWM, 0 );
    gpio_direction_output( CONFIG_VC_LCD_BL_PWM, 0);
#endif
#if defined( CONFIG_VC_LCD_POWER_ENABLE )
    gpio_request( CONFIG_VC_LCD_POWER_ENABLE, 0 );
    gpio_direction_output( CONFIG_VC_LCD_POWER_ENABLE, 1);
#endif
#if defined( CONFIG_VC_LCD_RESET )
    gpio_request( CONFIG_VC_LCD_RESET, 0 );
    gpio_direction_output( CONFIG_VC_LCD_RESET, 0);
#endif
#if defined( CONFIG_VC_LCD_BL_POWER_ENABLE )
    gpio_request( CONFIG_VC_LCD_BL_POWER_ENABLE, 0 );
    gpio_direction_output( CONFIG_VC_LCD_BL_POWER_ENABLE, 1);
#endif
#if defined( CONFIG_VC_LCD_BL_ENABLE )
    gpio_request( CONFIG_VC_LCD_BL_ENABLE, 0 );
    gpio_direction_output( CONFIG_VC_LCD_BL_ENABLE, 0);
#endif

    // Boot the videocore

    if (vc_init == 0) {
        printf("Booting videocore at 0x%08lx.\n", entry);

        ipc_wakeup_vc(0, entry);

        //rc = ipc_init();  //disable bc we are using a VCHIQ based firmware

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

    // Turn on the LCD and the backlight

#if defined( CONFIG_VC_LCD_RESET )
    gpio_set_value( CONFIG_VC_LCD_RESET,           1 );
#endif
#if defined( CONFIG_VC_LCD_BL_ENABLE )
    gpio_set_value( CONFIG_VC_LCD_BL_ENABLE,       1 );
#endif
#if defined( CONFIG_VC_LCD_BL_PWM )
    gpio_set_value( CONFIG_VC_LCD_BL_PWM,          1 );
#endif

	return rc;
}

#endif /* CONFIG_CAPRI */

static int start_vc( ulong addr )
{
   image_header_t   *img_hdr;
   VC_MEM_ACCESS_HANDLE_T vcHandle;
   int rc;

   /*
    * Do a check to see if the image is a u-boot wrapped image. This happens
    * if the image is loaded via nfs or other similar method which doesn't
    * use the vc load command.
    *
    * If the image was loaded via the vc load command, then this check will
    * fail (expected) since the raw image is already present.
    */

   img_hdr = (image_header_t *)addr;
   if (image_check_magic( img_hdr )) {
      /*
       * This means that a uimage wrapped image was loaded and hasn't yet
       * been unpacked, so we'll do a memmove to deal with that.
       */

      printf( "Relocating VC4 uimage ...\n" );
      memmove( (void *)addr,
               (void *)image_get_data( img_hdr ),
               image_get_data_size( img_hdr ));
   }
   else
   {
      printf( "VC4 raw image detected\n");
   }

   rc = OpenVideoCoreMemoryAt(addr, &vcHandle);
   if (rc == 0)
   {
      uint32_t entry = GetVideoCoreEntryPoint(vcHandle);
      CloseVideoCoreMemory(vcHandle);

#if defined( CONFIG_CAPRI )
      rc = capri_start_vc(entry);
#else
      rc = island_start_vc(entry);
#endif
   }

   return rc;
}

#if !defined(CONFIG_MTD_DEVICE)
/****************************************************************************
*
*  Retrieves information about the named partition.
*
*  The address and size are in units of blocks.
*  1 block = EFI_SECTORSIZE bytes.
*
*  The block address and size parameter types were chosen to match the
*  arguments passed into the block_read function.
*
***************************************************************************/

static int lookup_gpt_entry( const char *gpt_name, unsigned long *gpt_block_addr, lbaint_t *gpt_block_count )
{
   efi_ptable       *gpt;
   u32               i;
   u32               num_entries;
   efi_gpt_entry_t  *gpt_entry;
   int               rc = -1;

   if (( gpt = malloc( sizeof( *gpt ))) == NULL )
   {
      printf( "%s: Unable to allocate %d bytes to hold gpt table\n", __func__, sizeof( *gpt ));
      goto out;
   }
   if (( rc = gpt_get_table( gpt, &num_entries, &gpt_entry )) != 0 )
   {
      printf( "%s: Failed to retrieve gpt table\n", __func__ );
      goto out;
   }

   for ( i = 0; i < num_entries; i++, gpt_entry++ ) {

      const char *cmp_name = gpt_name;
      efi_char16_t  *part_name = gpt_entry->partition_name;

      /* The GPT table is stored in unicode, so we compare byte-by-byte */

      while (( *cmp_name == *part_name ) && *cmp_name && *part_name ) {
         cmp_name++;
         part_name++;
      }
      if ( *cmp_name == *part_name ) {

         u64   starting_lba;
         u64   ending_lba;

         /* We have a match */

         starting_lba = le64_to_cpu( gpt_entry->starting_lba );
         ending_lba   = le64_to_cpu( gpt_entry->ending_lba );

         *gpt_block_addr = starting_lba;
         *gpt_block_count = ending_lba - starting_lba + 1;

         rc = 0;
         goto out;
      }
   }

   /*
    * No partition found
    */

   rc = -1;

out:

   if ( gpt ) {
      free( gpt );
   }

   return rc;
}

/****************************************************************************
*
*  vc_load_from_mmc
*
*  Reads the videocore firmware from MMC into videocore memory.
*
***************************************************************************/

static int vc_load_from_mmc( ulong vc_mem_addr, unsigned long mmc_block_addr, lbaint_t mmc_block_count )
{
   struct mmc       *mmc;
   unsigned long     blocks_read;
   lbaint_t          image_bytes;
   lbaint_t          image_blocks;
   uint8_t           sector_buf[ EFI_SECTORSIZE ];
   image_header_t   *img_hdr;
   uint32_t          img_hdr_size;
   ulong             load_addr;

   if (( mmc = find_mmc_device( CONFIG_SYS_MMC_ENV_DEV )) == NULL ) {
      printf( "%s: find_mmc_device( %d ) failed\n", __func__, CONFIG_SYS_MMC_ENV_DEV );
      return 1;
   }

   /*
    * Read the first block into memory.
    */

   if (( blocks_read = mmc->block_dev.block_read( CONFIG_SYS_MMC_ENV_DEV, mmc_block_addr, 1, sector_buf )) != 1 ) {
      printf( "%s: Failed to read block from MMC block address 0x%lx\n", __func__, mmc_block_addr );
      return 1;
   }
   flush_cache( vc_mem_addr, EFI_SECTORSIZE );

   /*
    * Check to see if a uImage is present.
    */

   img_hdr = (image_header_t *)sector_buf;
   img_hdr_size = image_get_header_size();

   load_addr = vc_mem_addr;

   if (image_check_magic( img_hdr )) {

      image_bytes = image_get_image_size( img_hdr ); /* Includes header */

      image_blocks = ( image_bytes + EFI_SECTORSIZE - 1 ) / EFI_SECTORSIZE;
      if ( image_blocks > mmc_block_count ) {
         printf( "VC image size of 0x%lx blocks is bigger than partition size of 0x%lx blocks\n",
                 image_blocks, mmc_block_count );
         return 1;
      }
      if ( image_bytes == 0 ) {
         printf( "VC image size is 0 (bad)\n" );
         return 1;
      }

      printf( "Loading VC image of %d bytes to 0x%lx ...\n",
              image_get_data_size( img_hdr ),
              load_addr );

      /*
       * Copy the portion after the header to the videocore memory.
       */

      memcpy( (void *)load_addr,
              (void *)image_get_data( img_hdr ),
              EFI_SECTORSIZE - img_hdr_size );

      load_addr += ( EFI_SECTORSIZE - img_hdr_size );
      image_blocks--;
   }
   else {
      /*
       * No u-boot header, read rest of partition.
       */

      printf( "VC image: no u-boot header detected - assuming raw image\n" );
      printf( "Loading VC image of %ld bytes to 0x%lx ...\n",
              mmc_block_count * EFI_SECTORSIZE,
              load_addr );

      load_addr += EFI_SECTORSIZE;
      image_blocks = mmc_block_count - 1;
   }

   if (( blocks_read = mmc->block_dev.block_read(CONFIG_SYS_MMC_ENV_DEV,
                                                 mmc_block_addr + 1,
                                                 image_blocks,
                                                 (void *)load_addr )) != image_blocks ) {
      printf( "Failed to read entire VC image. Expecting 0x%lx blocks, actually read 0x%lx\n",
              image_blocks, blocks_read );
      return 1;
   }
   flush_cache( load_addr, image_blocks * EFI_SECTORSIZE );

   return 0;
}
#endif // !defined(CONFIG_MTD_DEVICE)

/****************************************************************************
*
*  do_vc_load
*
*  Loads the videocore firmware from MMC. The name of the gpt  partition is
*  passed as an argument.
*
***************************************************************************/

static int do_vc_load(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   const char       *partition_name;
   unsigned long     partition_addr = 0;
   lbaint_t          partition_block_count = 0;

#if defined(CONFIG_MTD_DEVICE)
   printf("Error: vc load command not supported for NAND\n");
   return -1;
#endif

   if ( argc != 2 ) {
      printf( "Expecting partition name\n" );
      return cmd_usage(cmdtp);
   }
   partition_name = argv[1];

   // Retrieve the partition table

   if ( lookup_gpt_entry( partition_name, &partition_addr, &partition_block_count ) != 0 ) {
      printf( "%s: Unable to location a partition named '%s'\n", __func__, partition_name );
      return -1;
   }

   return vc_load_from_mmc(GetVideoCoreMemoryBase(), partition_addr, partition_block_count );
}

/****************************************************************************
*
*  do_vc_run
*
*  Starts the videocore running the firmware which is currently loaded.
*
***************************************************************************/

int do_vc_run(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   ulong vc_mem_addr;

   if ( argc == 1 ) {
      vc_mem_addr = GetVideoCoreMemoryBase();
   }
   else if ( argc == 2 ) {
      vc_mem_addr = simple_strtoul(argv[1], NULL, 16);
      SetVideoCoreMemoryBase(vc_mem_addr);
   }
   else {
      printf( "command takes at most one argument\n" );
      return cmd_usage(cmdtp);
   }

   return start_vc(vc_mem_addr);
}

/****************************************************************************
*
*  do_vc_status
*
*  Prints information about the videocore.
*
***************************************************************************/

static int do_vc_status(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   uint32_t wakeup;
   size_t   vc_mem_size;
   uint32_t vc_mem_base;
   VC_MEM_ACCESS_HANDLE_T vcHandle;
   int rc;

   if ( argc != 1 ) {
      printf( "status command doesn't take any arguments\n" );
      return cmd_usage(cmdtp);
   }

   printf( "CONFIG_VCMEM_ADDR = 0x%08x\n", CONFIG_VCMEM_ADDR );

   /*
    * Check to see if the videocore has been started or not.
    */

   wakeup = ipc_query_wakeup_vc();
   if (( wakeup & ~1 ) == 0 ) {
      printf( "Videocore does NOT appear to be running IPCAWAKE = 0x%08x\n", wakeup );
   }
   else
   {
      printf( "Videocore has been started. IPCAWAKE = 0x%08x\n", wakeup );
   }

   vc_mem_base = GetVideoCoreMemoryBase();

   rc = OpenVideoCoreMemory(&vcHandle);

   /* Read the size now, in case it has been read from the VCDH */

   vc_mem_size = vc_mem_get_current_size();

   printf( "Videocore memory size = 0x%08x (%d Mb)\n", vc_mem_size, vc_mem_size >> 20 );
   printf( "Videocore memory base = 0x%08x (%d Mb)\n", vc_mem_base, vc_mem_base >> 20 );

   if (rc == 0)
   {
      uint32_t vc_entry_point;

      printf( "Valid VCDH header found\n" );

      vc_entry_point = GetVideoCoreEntryPoint(vcHandle);

      CloseVideoCoreMemory(vcHandle);

      printf( "Videocore entry point = 0x%08x\n", vc_entry_point );
   }

   return 0;
}

/****************************************************************************
*
*  do_vc_boot
*
*  Does a load followed by a run.
*
***************************************************************************/

static int do_vc_boot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int               rc = -1;

   if (( rc = do_vc_load(cmdtp, flag, argc, argv)) == 0 )
   {
      rc = do_vc_run( cmdtp, flag, 1, argv );
   }

   return rc;
}

/****************************************************************************
*
*  do_vc_bootfs
*
*  Manipulates the BootFS image attached to videocore firmware
*
***************************************************************************/

/****************************************************************************
*
*  check_ptr_in_range
*
***************************************************************************/

static inline int check_ptr_in_range( const void *ptr, VC_MEM_ADDR_T start_ptr, VC_MEM_ADDR_T end_ptr )
{
    return (( ALIAS_NORMAL( ptr ) >= ALIAS_NORMAL( start_ptr ))
        &&  ( ALIAS_NORMAL( ptr ) <= ALIAS_NORMAL( end_ptr )));
}

/****************************************************************************
*
*  log_ptr - Function taken from vcfw/logging/logging.c for managing
*            pointer wrapping.
*
***************************************************************************/

static unsigned char *log_ptr( const logging_fifo_log_t *log, unsigned char *ptr )
{
   if ( ptr >= log->end )
       return log->start + (ptr - log->end);

   if (ptr < log->start)
       return log->end - (log->start - ptr);

   return ptr;
}

/****************************************************************************
*
*  do_vc_dump
*
*  Syntax:
*	   vc dump{.b, .w, .l} {addr} {len}
*
***************************************************************************/

#define DISP_LINE_LEN	16
static int do_vc_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   static   uint32_t       last_addr;
   static   size_t         last_length;
   static   int            last_size;

   VC_MEM_ACCESS_HANDLE_T  vc_hndl;
   uint32_t                arm_addr;
   size_t                  max_bytes;
   int                     rc;
   uint32_t                addr     = last_addr;
   size_t                  length   = last_length;
   int                     size     = last_size;

   if ( argc < 2 ) {
      return cmd_usage(cmdtp);
   }

   if (( flag & CMD_FLAG_REPEAT ) == 0 ) {
      /* New command specified.  Check for a size specification.
       * Defaults to long if no or incorrect specification.
       */

      if ((size = cmd_get_data_size(argv[0], 4)) < 0)
         return 1;

      /* Address is specified since argc > 1
      */
      addr = simple_strtoul(argv[1], NULL, 16);

      /* If another parameter, it is the length to display.
       * Length is the number of objects, not number of bytes.
       */
      if (argc > 2)
         length = simple_strtoul(argv[2], NULL, 16);
   }

   if (( rc = OpenVideoCoreMemory( &vc_hndl )) != 0 ) {
      printf( "%s: OpenVideoCoreMemory failed: %d\n", __func__, rc );
      return rc;
   }

   arm_addr = GetVideoCoreMemoryAddr( vc_hndl );
   max_bytes = GetVideoCoreMemorySize( vc_hndl );

   if ( !check_ptr_in_range( (void *)(unsigned long)addr, 0, max_bytes ))
   {
       printf( "Address 0x%08x references memory outside %zu Mb memory space\n",
               addr, max_bytes / ( 1024 * 1024 ));
       CloseVideoCoreMemory( vc_hndl );
       return 1;
   }

   if ( !check_ptr_in_range( (void *)( addr + (size * length)), 0, max_bytes ))
   {
       printf( "Address 0x%08x + len %zu references memory outside %zu Mb memory space\n",
               addr, (size * length), max_bytes / ( 1024 * 1024 ));
       CloseVideoCoreMemory( vc_hndl );
       return 1;
   }

   /*
    * Convert address to the physical memory equivalent. This means that
    *
    *  vc dump 00000800 10
    *  vc dump 40000800 10
    *  vc dump 80000800 10
    *  vc dump c0000800 10
    *
    * will now all produce identical output
    *
    */

   addr = (uint32_t)(unsigned long)ALIAS_NORMAL( addr );

   print_buffer( addr, (void *)(arm_addr + addr), size, length, DISP_LINE_LEN / size );
   addr += size*length;

   CloseVideoCoreMemory( vc_hndl );

   last_addr = addr;
   last_length = length;
   last_size = size;

   return 0;
}

/****************************************************************************
*
*  read_logging_header
*
***************************************************************************/

static int read_logging_header( VC_MEM_ACCESS_HANDLE_T  vc_hndl,
                                logging_header_t       *logging_header,
                                VC_MEM_ADDR_T          *log_start,
                                VC_MEM_ADDR_T          *log_end )
{
    VC_MEM_ADDR_T       log_start_ptr;
    VC_MEM_ADDR_T       log_end_ptr;

    if ( !LookupVideoCoreSymbol( vc_hndl, "__LOG_START", &log_start_ptr, NULL ))
    {
        printf( "Unable to determine the value of __LOG_START\n" );
        return 0;
    }
    if ( !LookupVideoCoreSymbol( vc_hndl, "__LOG_END", &log_end_ptr, NULL ))
    {
        printf( "Unable to determine the value of __LOG_END\n" );
        return 0;
    }

    if ( !ReadVideoCoreUInt32( vc_hndl, log_start, log_start_ptr ))
    {
        printf( "__LOG_START pointer (0x%08x) doesn't seem to be sane\n", log_start_ptr );
        return 0;
    }

    if ( !ReadVideoCoreUInt32( vc_hndl, log_end, log_end_ptr ))
    {
        printf( "__LOG_END pointer (0x%08x) doesn't seem to be sane\n", log_end_ptr );
        return 0;
    }

    DBG( "log_start_ptr = 0x%08x\n", log_start_ptr );
    DBG( "log_end_ptr   = 0x%08x\n", log_end_ptr );
    DBG( "log_start     = 0x%08x\n", log_start );
    DBG( "log_end       = 0x%08x\n", log_end );

    return ReadVideoCoreMemory( vc_hndl, logging_header, *log_start, sizeof( *logging_header ));
}

/****************************************************************************
*
*  read_logging_fifo
*
***************************************************************************/

static int read_logging_fifo( VC_MEM_ACCESS_HANDLE_T  vc_hndl,
                              logging_fifo_log_t     *logging_fifo,
                              void                   *vc_mem_addr )
{
    return ReadVideoCoreMemory( vc_hndl, logging_fifo, TO_VC_MEM_ADDR( vc_mem_addr ), sizeof( *logging_fifo ));
}

/****************************************************************************
*
*  read_logging_msg_hdr
*
***************************************************************************/

static int read_logging_msg_hdr( VC_MEM_ACCESS_HANDLE_T          vc_hndl,
                                 logging_fifo_log_msg_header_t  *msg_hdr,
                                 void                           *vc_mem_addr )
{
    return ReadVideoCoreMemory( vc_hndl, msg_hdr, TO_VC_MEM_ADDR( vc_mem_addr ), sizeof( *msg_hdr ));
}

/****************************************************************************
*
*  read_logging_msg
*
***************************************************************************/

static int read_logging_msg( VC_MEM_ACCESS_HANDLE_T     vc_hndl,
                             const logging_fifo_log_t  *log,
                             void                      *msg_buf,
                             void                      *vc_mem_addr,
                             size_t                     msg_size )
{
    size_t  avail = TO_VC_MEM_ADDR( log->end ) - TO_VC_MEM_ADDR( vc_mem_addr );

    if ( avail >= msg_size )
    {
        if ( !ReadVideoCoreMemory( vc_hndl, msg_buf, TO_VC_MEM_ADDR( vc_mem_addr ), msg_size ))
        {
            printf( "Unable to read message from %p (len %zu)\n", vc_mem_addr, msg_size );
            return 0;
        }
    }
    else
    {
        if ( !ReadVideoCoreMemory( vc_hndl, msg_buf, TO_VC_MEM_ADDR( vc_mem_addr ), avail ))
        {
            printf( "Unable to read message from %p (len %zu)\n", vc_mem_addr, avail );
            return 0;
        }
        if ( !ReadVideoCoreMemory( vc_hndl, &((char *)msg_buf)[avail], TO_VC_MEM_ADDR( log->start ), msg_size - avail ))
        {
            printf( "Unable to read message from %p (len %zu)\n", log->start, msg_size - avail );
            return 0;
        }
    }
    return 1;
}

/****************************************************************************
*
*  do_vc_log
*
*  Syntax:
*     vc log assert
*     vc log msg
*
***************************************************************************/

static int do_vc_log(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   VC_MEM_ADDR_T           log_start;
   VC_MEM_ADDR_T           log_end;
   logging_header_t        logging_header;
   logging_fifo_log_t      fifo;
   void                   *fifo_addr;
   int                     is_assert;
   unsigned char          *mark;
   VC_MEM_ACCESS_HANDLE_T  vc_hndl;
   int                     rc;

   if ( argc != 2 ) {
      printf( "Expecting log type (msg or assert)\n" );
      return cmd_usage(cmdtp);
   }
   if ( strcmp( argv[1], "assert" ) == 0 ) {
      is_assert = 1;
   }
   else
   if ( strcmp( argv[1], "msg" ) == 0 ) {
      is_assert = 0;
   }
   else
   {
      printf( "Expecting log type (msg or assert)\n" );
      return cmd_usage(cmdtp);
   }

   if (( rc = OpenVideoCoreMemory( &vc_hndl )) != 0 ) {
      printf( "%s: OpenVideoCoreMemory failed: %d\n", __func__, rc );
      return rc;
   }

   if ( !read_logging_header( vc_hndl,
               &logging_header,
               &log_start,
               &log_end ))
   {
       printf( "Unable to read logging_header from %08x\n", log_start );
       CloseVideoCoreMemory( vc_hndl );
       return -1;
   }

   fifo_addr = is_assert ? logging_header.assertion_log.log
                         : logging_header.message_log.log;

   if ( !read_logging_fifo( vc_hndl, &fifo, fifo_addr ))
   {
       printf( "Unable to read logging_fifo from %p\n", fifo_addr );
       CloseVideoCoreMemory( vc_hndl );
       return -1;
   }

   mark = fifo.next_msg;

   if ( mark == fifo.ptr ) {
      printf( "No messages available\n" );
      CloseVideoCoreMemory( vc_hndl );
      return 0;
   }

   while (mark != fifo.ptr)
   {
       char                            msg_buf[ 1024 ];
       unsigned short                  msg_size;
       logging_fifo_log_msg_header_t   msg_hdr;
       char                           *msg_str;

       if ( !read_logging_msg_hdr( vc_hndl, &msg_hdr, mark ))
       {
           printf( "Unable to read log message header from %p\n", mark );
           CloseVideoCoreMemory( vc_hndl );
           return -1;
       }

       msg_size = msg_hdr.size;

       /* This shouldn't happen but does */
       if (msg_size == 0)
           break;

       if ( msg_size > sizeof( msg_buf ))
       {
           msg_size = sizeof( msg_buf );
       }

       if ( !read_logging_msg( vc_hndl, &fifo, msg_buf, mark, msg_size ))
       {
           printf( "Unable to read message from %p (len %d)\n", mark, msg_size );
           CloseVideoCoreMemory( vc_hndl );
           return -1;
       }

       // Make sure that we null terminate the buffer. This is especially important if we
       // didn't read the entire message for some reason.

       if ( msg_size < sizeof( msg_buf ))
       {
           msg_buf[msg_size] = '\0';
       }
       else
       {
           msg_buf[ msg_size - 1] = '\0';
       }

       msg_str = &msg_buf[ sizeof( msg_hdr )];

       if ( is_assert )
       {
           char       *file_name;
           uint32_t    line_number;
           char        *cond_str;

           // Format of data is
           //   null terminated filename
           //   32-bit line number
           //   null terminated assertion condition

           file_name = msg_str;
           msg_str += ( strlen( file_name ) + 1 );
           memcpy( &line_number, msg_str, sizeof( line_number ));
           msg_str += sizeof( line_number );
           cond_str = msg_str;

           printf( "%06lu.%03lu: assert( %s ) failed; %s line %d\n",
                   msg_hdr.time / 1000, msg_hdr.time % 1000,
                   cond_str, file_name, line_number );
       }
       else
       {
           // Format of data is
           //   32-bit logging level
           //   null terminated message

           msg_str += sizeof( uint32_t );

           printf( "%06lu.%03lu: %s\n",
                   msg_hdr.time / 1000, msg_hdr.time % 1000,
                   msg_str );
       }

       mark = log_ptr( &fifo, mark + msg_hdr.size );

       if (ctrlc())
       {
          CloseVideoCoreMemory( vc_hndl );
          return -1;
       }
   }
   CloseVideoCoreMemory( vc_hndl );
   return 0;
}

/****************************************************************************
*
*  do_vc_base
*
*  Syntax:
*     vc base [<hex>]
*     vc log msg
*
***************************************************************************/

static int do_vc_base(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   ulong vc_mem_base;

   if ( argc == 1 ) {
      vc_mem_base = GetVideoCoreMemoryBase();
   }
   else if ( argc == 2 ) {
      vc_mem_base = simple_strtoul(argv[1], NULL, 16);
      SetVideoCoreMemoryBase(vc_mem_base);
   }
   else {
      printf( "%s command takes at most one argument\n", cmdtp->name );
      return cmd_usage(cmdtp);
   }

   printf( "VideoCore memory base = %08lx\n", vc_mem_base);

   return 0;
}

/****************************************************************************
*
*  do_vc_sym
*
*  Syntax:
*	   vc sym sym-name
*
***************************************************************************/

static int do_vc_sym(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int                     rc;
   VC_MEM_ACCESS_HANDLE_T  vc_hndl;
   const char             *sym_name;
   VC_MEM_ADDR_T           vc_mem_addr;
   size_t                  vc_mem_size;

   if ( argc != 2 ) {
      printf( "Expecting symbol name\n" );
      return cmd_usage(cmdtp);
   }
   sym_name = argv[1];

   if (( rc = OpenVideoCoreMemory( &vc_hndl )) != 0 ) {
      printf( "%s: OpenVideoCoreMemory failed: %d\n", __func__, rc );
      return rc;
   }

   if ( LookupVideoCoreSymbol( vc_hndl, sym_name, &vc_mem_addr, &vc_mem_size ) < 0 ) {
      CloseVideoCoreMemory( vc_hndl );
      printf( "Symbol '%s' not found\n", sym_name );
      return -ENOENT;
   }

   if ( vc_mem_size == 4 ) {
      uint32_t value;

      if ( ReadVideoCoreUInt32( vc_hndl, &value, vc_mem_addr ))
         printf( "Address: %08x Size: %zx Value: %08x\n", vc_mem_addr, vc_mem_size, value );
      else
         printf( "Address: %08x Size: %zx Value: *bad-ptr*\n", vc_mem_addr, vc_mem_size );
   }
   else
      printf( "Address: %08x Size: %zx\n", vc_mem_addr, vc_mem_size );

   CloseVideoCoreMemory( vc_hndl );

   return 0;
}

/****************************************************************************
*
*  do_vc_syms
*
*  Syntax:
*	   vc syms
*
***************************************************************************/

static int do_vc_syms(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int                     rc;
   unsigned                num_syms;
   unsigned                sym_idx;
   VC_MEM_ACCESS_HANDLE_T  vc_hndl;

   if (( rc = OpenVideoCoreMemory( &vc_hndl )) != 0 )
   {
      printf( "%s: OpenVideoCoreMemory failed: %d\n", __func__, rc );
      return rc;
   }

   num_syms = NumVideoCoreSymbols( vc_hndl );

   if ( num_syms == 0 )
   {
       printf( "No symbols detected\n" );
       CloseVideoCoreMemory( vc_hndl );
       return 0;
   }

   printf( "Address      Size Value    Symbol\n" );
   printf( "-------- -------- -------- ----------------------------------\n" );
   for ( sym_idx = 0; sym_idx < num_syms; sym_idx++ )
   {
       char            name_buf[ 256 ];
       VC_MEM_ADDR_T   vc_mem_addr;
       size_t          vc_mem_size;

       GetVideoCoreSymbol( vc_hndl,
                           sym_idx,
                           name_buf,
                           sizeof( name_buf ),
                           &vc_mem_addr,
                           &vc_mem_size );

       if ( vc_mem_size == 4 )
       {
           uint32_t        value;

           if ( ReadVideoCoreUInt32( vc_hndl, &value, vc_mem_addr ))
           {
               printf( "%08x %8zx %08x %s\n", vc_mem_addr, vc_mem_size, value, name_buf );
           }
           else
           {
               printf( "%08x %8zx *bad-ptr* %s\n", vc_mem_addr, vc_mem_size, name_buf );
           }
       }
       else
       {
           printf( "%08x %8zx          %s\n", vc_mem_addr, vc_mem_size, name_buf );
       }
   }
   CloseVideoCoreMemory( vc_hndl );

   return 0;
}

/****************************************************************************
*
*  do_vc_args
*
*  Syntax:
*	   vc args name[=value] ...
*
***************************************************************************/

static int do_vc_args(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int                     rc;
   int                     argn;
   VC_MEM_ACCESS_HANDLE_T  vc_hndl;

   if ( argc < 2 ) {
      printf( "Expecting symbol name\n" );
      return cmd_usage(cmdtp);
   }

   if (( rc = OpenVideoCoreMemory( &vc_hndl )) != 0 ) {
      printf( "%s: OpenVideoCoreMemory failed: %d\n", __func__, rc );
      return rc;
   }

   for (argn = 1; argn < argc; argn++)
   {
      VC_MEM_ADDR_T vc_mem_addr;
      size_t        vc_mem_size;
      char         *sym_name;
      char         *p;
      char          sep;
      uint32_t      value = 1;

      sym_name = argv[argn];
      p = sym_name;
      while (isalnum(*p) || (*p == '_'))
         p++;

      sep = *p;
      *(p++) = '\0';

      if ( LookupVideoCoreSymbol( vc_hndl, sym_name, &vc_mem_addr, &vc_mem_size ) < 0 ) {
         printf( "'%s': not found\n", sym_name );
         rc = -ENOENT;
         break;
      }

      if ( sep == '=' )
      {
         value = simple_strtoul( p, NULL, 16 );
      }
      else if ( sep == ':' )
      {
         int len = strlen(p) + 1;
         if ( len > vc_mem_size )
         {
            printf( "'%s': value too long - max %d\n", sym_name, vc_mem_size );
            rc = -EINVAL;
            break;
         }
         if ( !WriteVideoCoreMemory( vc_hndl, p, vc_mem_addr, len ) )
         {
            printf( "'%s': address %08x, size %zx - write failed\n", sym_name, vc_mem_addr, vc_mem_size );
            rc = -EINVAL;
            break;
         }
         continue;
      }
      else if (sep)
      {
         printf( "'%s': illegal separator '%c'\n", sym_name, sep );
         rc = -EINVAL;
         break;
      }

      if ( vc_mem_size == 4 ) {
         if ( !WriteVideoCoreUInt32( vc_hndl, value, vc_mem_addr ) )
         {
            printf( "'%s': address %08x, size %zx - write failed\n", sym_name, vc_mem_addr, vc_mem_size );
            rc = -EINVAL;
            break;
         }
      }
      else
      {
         printf( "'%s': size %zx - expected 4\n", sym_name, vc_mem_size );
         rc = -EINVAL;
         break;
      }
   }

   CloseVideoCoreMemory( vc_hndl );

   return rc;
}

/****************************************************************************
*
*  UBoot command table
*
***************************************************************************/

static cmd_tbl_t cmd_vc_sub[] = {
   U_BOOT_CMD_MKENT(args,     2, 0, do_vc_args,    "", "" ),
   U_BOOT_CMD_MKENT(boot,     2, 0, do_vc_boot,    "", "" ),
   U_BOOT_CMD_MKENT(bootfs,   7, 0, do_vc_bootfs,  "", "" ),
   U_BOOT_CMD_MKENT(display,  2, 0, do_vc_display, "", "" ),
   U_BOOT_CMD_MKENT(dump,     3, 1, do_vc_dump,    "", "" ),
   U_BOOT_CMD_MKENT(load,     2, 0, do_vc_load,    "", "" ),
   U_BOOT_CMD_MKENT(log,      2, 0, do_vc_log,     "", "" ),
   U_BOOT_CMD_MKENT(base,     2, 0, do_vc_base,    "", "" ),
   U_BOOT_CMD_MKENT(run,      2, 0, do_vc_run,     "", "" ),
   U_BOOT_CMD_MKENT(status,   1, 0, do_vc_status,  "", "" ),
   U_BOOT_CMD_MKENT(sym,      2, 0, do_vc_sym,     "", "" ),
   U_BOOT_CMD_MKENT(syms,     1, 0, do_vc_syms,    "", "" ),
};

extern int _do_help(cmd_tbl_t *cmd_start, int cmd_items,
          cmd_tbl_t *cmdtp, int flag,
          int argc, char * const argv[]);

/****************************************************************************
*
*  do_vc
*
***************************************************************************/

static int do_vc (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   cmd_tbl_t *cp;

   if (argc < 2) {
      printf( "Expecting vc sub-command\n" );
      return cmd_usage(cmdtp);
   }

   /* drop initial "vc" arg */
   argc--;
   argv++;

   cp = find_cmd_tbl(argv[0], cmd_vc_sub, ARRAY_SIZE(cmd_vc_sub));
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

U_BOOT_CMD(
		bootvc, CONFIG_SYS_MAXARGS, 0, do_vc_run,
		"boot videocore with application image at address 'addr'",
		"[addr] \n        - boot videocore with application image at address 'addr'\n"
		"          arguments in hexadecimal"
		);

U_BOOT_CMD(
		vc, 7, 1, do_vc,
		"VideoCore sub system",
      "load partition-name                  - Loads VC image from the named partition\n"
      "vc args name[=hex] ...                  - Sets the named symbols to the given values (or 1)\n"
      "vc base [address]                       - Displays/sets the base where the VC image starts\n" 
      "vc boot partition-name                  - Loads and boots VC image from the named partition\n"
      "vc bootfs list                          - Lists the files stored in the videocore BootFS\n"
      "vc bootfs add filename addr len [descr] - Adds a file to the videocore BootFS\n"
      "vc bootfs add_dtblob                    - Adds the current dt-blob to the videocore BootFS\n"
      "vc display power [on, off]              - Control display power within videocore\n"
      "vc display fb init                      - Initialise the videocore framebuffer\n"
      "vc display fb update (0,1)              - Update the framebuffer using the desired buffer\n"
      "vc dump[.b, .w, .l] addr [# of objects] - Dumps memory from videocore address 'addr'\n"
      "vc log assert                           - Displays the contents of the videocore assert log\n"
      "vc log msg                              - Displays the contents of the videocore message log\n"
      "vc run [address]                        - Starts the videocore running (assumes image already loaded)\n"
      "vc status                               - Prints some status information about the videocore\n"
      "vc sym name                             - Displays information about symbol 'name'\n"
      "vc syms                                 - Displays symbols available from the videocore image\n"
		);

