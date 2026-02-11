/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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

/*
 * ===========================================================================
 *  inlcude header file declarations
 */

#include <asm/arch/chal_common.h>
#include <asm/arch/chal_ipc.h>
#include <asm/string.h>

#include <asm/arch//mm_io.h>
#include <asm/arch/brcm_rdb_ipcopen.h>
#include <asm/arch/brcm_rdb_ipcsec.h>

#include <asm/kona-common/secure.h>

/*
 * ===========================================================================
 *  global variable declarations
 *
 */

#define LOADADDR_SRAM_ARMMEMSPACE       CONFIG_SSRAM_BASE+CONFIG_VC4BOOT_SRAM_OFFSET
#define VC4_WAKEUPADDR_SRAM_VCMEMSPACE  (LOADADDR_SRAM_ARMMEMSPACE-0x34000000)+0x60000000

/*
 * ===========================================================================
 * Secure API function define 
 *
 */
#define   hw_sec_pub_dispatcher(appln_id, flag, addr) \
	((void (*)(uint32_t, uint32_t, uint32_t))(0x1004101)) \
	(appln_id, flag, addr)
/*
 * ===========================================================================
 *  extern variable declarations
 *
 */

/*
 * ===========================================================================
 *  static function prototype declarations
 *
 */

/*
 * ===========================================================================
 *  local macro declarations
 *
 */
#if 1
#define IPC_DBG_OUT(a) {BCM_DBG_OUT(a);}
#else
#define IPC_DBG_OUT(a) {}
#endif


typedef struct CHAL_IPC_DEV_T
{
    uint32_t             ipc_open_reg_base;
    uint32_t             ipc_secure_reg_base;
    CHAL_IPC_CONFIG_T   ipc_config;

} CHAL_IPC_DEV_T;


/*
 * ===========================================================================
 *  static variables declarations
 *
 */

static CHAL_IPC_DEV_T ipcDevice;
/* IPCSEC_IPCAWAKE values saved in variables to avoid secured register read/write */ 
static uint32_t wakeup_addr; 
static uint32_t wakeup_en; 

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_config
 *
 *  Description:
 *
 * ******************************************************************************
 */
CHAL_IPC_HANDLE chal_ipc_config (
    CHAL_IPC_CONFIG_T *pConfig
    )
{
   CHAL_UNUSED( pConfig );

   BCM_DBG_ENTER();

   ipcDevice.ipc_open_reg_base = MM_IO_BASE_IPC_NS;
   ipcDevice.ipc_secure_reg_base = MM_IO_BASE_IPC_S;

   BCM_DBG_EXIT();
   return( (CHAL_IPC_HANDLE)&ipcDevice );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_write_mailbox
 *
 *  Description:
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_write_mailbox (
    CHAL_IPC_HANDLE handle,
    IPC_MAILBOX_ID mailboxId,
    uint32_t value
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   /* Check boundry conditions
    * IPC_MAILBOX_ID is an unsigned value and does not need to check less than zero */
   if ( mailboxId >= IPC_MAILBOX_ID_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   device = (CHAL_IPC_DEV_T*)handle;

   CHAL_REG_WRITE32( device->ipc_secure_reg_base + IPCSEC_IPCMAIL0_OFFSET + mailboxId*sizeof(uint32_t), value );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_read_mailbox
 *
 *  Description:
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_read_mailbox (
    CHAL_IPC_HANDLE handle,
    IPC_MAILBOX_ID mailboxId,
    uint32_t *value
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   /* Check boundry conditions
    * IPC_MAILBOX_ID is an unsigned value and does not need to check less than zero */
   if ( mailboxId >= IPC_MAILBOX_ID_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   device = (CHAL_IPC_DEV_T*)handle;

   *value = CHAL_REG_READ32( device->ipc_secure_reg_base + IPCSEC_IPCMAIL0_OFFSET + mailboxId*sizeof(uint32_t) );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_query_wakeup_vc
 *
 *  Description: Queries and returns the value of the wakeup register.
 *
 * ******************************************************************************
 */

BCM_ERR_CODE chal_ipc_query_wakeup_vc (
    CHAL_IPC_HANDLE handle,
    uint32_t *result
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   *result = wakeup_addr | wakeup_en;

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

extern void start_vc4();

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_wakeup_vc
 *
 *  Description: Videocore wakeup
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_wakeup_vc (
    CHAL_IPC_HANDLE handle,
    uint32_t address
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;
   wakeup_addr = address;
   wakeup_en =  IPCSEC_IPCAWAKE_WAKEUP_MASK;

   /*	Write to sec reg replaced with SECURE API call for non-secure mode */
   if (0/*running_in_non_secure_mode()*/)
   {
	   hw_sec_pub_dispatcher(0x0e000008,0xF, address  | IPCSEC_IPCAWAKE_WAKEUP_MASK);
   }
   else
   {
	   CHAL_REG_WRITE32( device->ipc_secure_reg_base + IPCSEC_IPCAWAKE_OFFSET,
			     address | IPCSEC_IPCAWAKE_WAKEUP_MASK );
   }

   if (*(unsigned*)MM_ADDR_IO_SRAM == 0xBADDCAFE)
      start_vc4();

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_sleep_vc
 *
 *  Description: VideoCore sleep
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_sleep_vc (
    CHAL_IPC_HANDLE handle
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;
   wakeup_en =  ~IPCSEC_IPCAWAKE_WAKEUP_MASK;

   /*	Write to sec reg replaced iwth SECURE API call for non-secure mode */
   if (0/*running_in_non_secure_mode()*/)
   {
	   hw_sec_pub_dispatcher(0x0e000008,0xF, wakeup_addr  & (~IPCSEC_IPCAWAKE_WAKEUP_MASK));
   }
   else
   {

   CHAL_REG_WRITE32( device->ipc_secure_reg_base + IPCSEC_IPCAWAKE_OFFSET,
      CHAL_REG_READ32( device->ipc_secure_reg_base + IPCSEC_IPCAWAKE_OFFSET ) &
      ~IPCSEC_IPCAWAKE_WAKEUP_MASK );
   }

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_boot_vc
 *
 *  Description: Videocore boot
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_boot_vc (
    CHAL_IPC_HANDLE handle,
    uint32_t address,
    CHAL_IPC_BOOTMODE bootmode
    )
{
   BCM_ERR_CODE retVal = BCM_ERROR;
   uint32_t * loadAddr;

   BCM_DBG_ENTER();

   if (bootmode == IPC_BOOTMODE_DIRECT)
   {
      retVal = chal_ipc_wakeup_vc( handle, address);
   }
   else if (bootmode == IPC_BOOTMODE_SPLITREMAP)
   {
      loadAddr = (uint32_t *)(LOADADDR_SRAM_ARMMEMSPACE);
      /* Fill memory with hand-compiled VC4 code */
      *(unsigned int*)(loadAddr++) = 0x1800B019;     // mov %sp,0x1800
      *(unsigned int*)(loadAddr++) = 0x1A00B01B;     // mov %usp,0x1a00
      *(unsigned int*)(loadAddr++) = 0x1C00B01C;     // mov %ssp,0x1c00
      *(unsigned int*)(loadAddr++) = 0x3004e800;
      *(unsigned int*)(loadAddr++) = 0xE8017ee0;     // mov %r0,0x7ee03004  // Split re-map reg
      *(unsigned int*)(loadAddr++) = 0x00020102;     // mov %r1,0x00020102 
      *(unsigned int*)(loadAddr++) = 0xe81a0901;     // st %r1,(%r0)
      *(unsigned int*)(loadAddr++) = address;        // mov %lr, 0xc0000200
      *(unsigned int*)(loadAddr++) = 0x0000005a;     // b %lr
     retVal = chal_ipc_wakeup_vc(handle, VC4_WAKEUPADDR_SRAM_VCMEMSPACE );
   }

   BCM_DBG_EXIT();
   return( retVal );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_int_vcset
 *
 *  Description: Videocore interrupt set
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_int_vcset (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE irqNum
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   if ( irqNum >= IPC_INTERRUPT_SOURCE_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   /* Note: since IPCOPEN_IPCASET_OFFSET is a write-only register it is not
    * valid to read the register then OR the appropriate bit and then write the
    * new value.  Simply write the appropriate bit to set the interrupt */
   CHAL_REG_WRITE32(device->ipc_open_reg_base + IPCOPEN_IPCASET_OFFSET, 1 << irqNum );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_int_clr
 *
 *  Description: Clear ARM interrupt
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_int_clr (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE irqNum
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   if ( irqNum >= IPC_INTERRUPT_SOURCE_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   /* Note: since IPCOPEN_IPCACLR_OFFSET is a write-only register it is not
    * valid to read the register then OR the appropriate bit and then write the
    * new value.  Simply write the appropriate bit to clear the interrupt */
   CHAL_REG_WRITE32(device->ipc_open_reg_base + IPCOPEN_IPCACLR_OFFSET, 1 << irqNum );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_int_mode
 *
 *  Description: Set ARM interrupt to be secure or open
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_int_secmode (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE irqNum,
    IPC_INTERRUPT_MODE intMode
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   if ( irqNum >= IPC_INTERRUPT_SOURCE_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   if ( intMode == IPC_INTERRUPT_MODE_OPEN )
   {
      CHAL_REG_CLRBIT32(device->ipc_secure_reg_base + IPCSEC_IPCASECURE_OFFSET, 1 << irqNum );
   }
   else if ( intMode == IPC_INTERRUPT_MODE_SECURE )
   {
      CHAL_REG_SETBIT32(device->ipc_secure_reg_base + IPCSEC_IPCASECURE_OFFSET, 1 << irqNum );
   }
   else
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_get_int_status
 *
 *  Description: Get ARM interrupt status
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_get_int_status (
    CHAL_IPC_HANDLE handle,
    uint32_t *status
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   *status = CHAL_REG_READ32( device->ipc_open_reg_base + IPCOPEN_IPCASTATUS_OFFSET );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_get_int_source
 *
 *  Description: Get ARM interrupt source
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_get_int_source (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE *source
    )
{
   uint32_t status;
   IPC_INTERRUPT_SOURCE i = IPC_INTERRUPT_SOURCE_0;

   chal_ipc_get_int_status( handle, &status );

   for ( i = IPC_INTERRUPT_SOURCE_0; i < IPC_INTERRUPT_SOURCE_MAX; i++ )
   {
      if ( status & ( IPC_INTERRUPT_STATUS_ENABLED << i ) )
      {
         *source = i;
         return BCM_SUCCESS;
      }
   }

   *source = IPC_INTERRUPT_SOURCE_NULL;
   return BCM_ERROR;
}


/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_get_error_status
 *
 *  Description: Get ARM error status
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_get_error_status (
    CHAL_IPC_HANDLE handle,
    uint32_t *status
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   *status = CHAL_REG_READ32(device->ipc_open_reg_base + IPCOPEN_IPCERR_OFFSET );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}


//#if defined( __KERNEL__ )
//
//#include <linux/module.h>
//
///* Export the following symbols so that the videocore driver can use them.*/
//EXPORT_SYMBOL( chal_ipc_config );
//EXPORT_SYMBOL( chal_ipc_query_wakeup_vc );
//EXPORT_SYMBOL( chal_ipc_wakeup_vc );
//EXPORT_SYMBOL( chal_ipc_sleep_vc );
//EXPORT_SYMBOL( chal_ipc_boot_vc );
//EXPORT_SYMBOL( chal_ipc_int_secmode );
//EXPORT_SYMBOL( chal_ipc_int_clr );
//EXPORT_SYMBOL( chal_ipc_int_vcset );
//EXPORT_SYMBOL( chal_ipc_get_int_status );
//EXPORT_SYMBOL( chal_ipc_get_int_source );
//EXPORT_SYMBOL( chal_ipc_write_mailbox );
//EXPORT_SYMBOL( chal_ipc_read_mailbox );
//EXPORT_SYMBOL( chal_ipc_get_error_status );
//
//#endif


