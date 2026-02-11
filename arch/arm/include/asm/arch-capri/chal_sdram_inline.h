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

/****************************************************************************
This file is meant to be called by a target that needs to configure the 
memory.  If further memc access is required after initializing the memory,
the memhandle should be requested via the getHandleMEMC function.  The 
pointer should then be used for any memc accesses.
****************************************************************************/
#ifndef _CHAL_SDRAM_INLINE_H_
#define _CHAL_SDRAM_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */
#include <asm/arch/chal_common.h>
#include <asm/arch/chipregHw_inline.h>
#include <asm/arch/brcm_rdb_aphy_csr.h>
#include <asm/arch/mm_io.h>
#include <asm/arch/brcm_rdb_sysmap.h>
//#include <asm/arch/chal_ccu_inline.h>
#include <asm/arch/chal_memc.h>
//#include <asm/arch/chal_sdram_configs.h>
//#include <cfg_global.h>

/* ---- External Variable Declarations ----------------------------------- */

/* ---- External Function Prototypes ------------------------------------- */
extern MEMC_EXPORT BCM_ERR_CODE chal_memc_temperature_polling(
    CHAL_MEMC_HANDLE handle,
    _Bool enable
    );
extern MEMC_EXPORT BCM_ERR_CODE chal_memc_init (
    CHAL_MEMC_HANDLE handle
    );
/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
#define MEMC_TIMING0            0x2323323  /* these values not configured in driver, defaults are used */
#define MEMC_TIMING1            0x332082   /* these values not configured in driver, defaults are used */

   #define MEMC_REFRESH_CTRL       0x4340065  /* these values not configured in driver, defaults are used */

/* ---- Private Variables ------------------------------------------------ */
static CHAL_MEMC_HANDLE_T handleMEMC[MEMC_ID_MAX];
static MEMC_DDR_TYPE      manualDDRType[MEMC_ID_MAX] = { MEMC_DDR_TYPE_UNKNOWN, MEMC_DDR_TYPE_UNKNOWN };
    
/* ---- Private Function Prototypes -------------------------------------- */
static BCM_ERR_CODE reinitHandle(MEMC_ID controllerId);
static BCM_ERR_CODE setDDRType(MEMC_ID controllerId);
static BCM_ERR_CODE setContollerBaseAddress(MEMC_ID controllerId);
static BCM_ERR_CODE setDeviceChipSelect(MEMC_ID controllerId, _Bool dualChipSelect);
static BCM_ERR_CODE setOperationMode(MEMC_ID controllerId);

/* ==== Public Functions ================================================= */

/*
 * ===========================================================================
 * 
 *   Function Name: chal_sdram_getMemcHandle
 * 
 *   Description:
 *       Returns MEMC data structure and initialize if required, but don't reinit controller.
 *       The controller has already been initialized and SDRAM is already
 *          operational.
 *       If requried to re-init the data structure then use configuration already in MEMC
 *          contoller rather than with compile time defines
 * 
 * ===========================================================================
 */
static inline CHAL_MEMC_HANDLE_T* chal_sdram_getMemcHandle(MEMC_ID controllerId)
{
    CHAL_MEMC_HANDLE memHandle;
    
    if ( controllerId == MEMC_ID_SYS || controllerId == MEMC_ID_VC )
    {
       memHandle = &handleMEMC[controllerId];
    }
    else
    {
       return NULL;    
    }    

    /* Determine if MEMC structure has been previously intialized */
    if ( memHandle->memc_open_reg_base == 0 )
    {
       /* Re-init structure only not the actual memory controller */
       reinitHandle( controllerId );
    }

    return memHandle;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_sdram_set_ddr_type
 * 
 *   Description:
 *       Manually sets the DDR type - this will override auto-detection.
 *       Set to MEMC_DDR_TYPE_UNKNOWN to re-engage auto-detection. Else
 *       set it to MEMC_DDR_TYPE_LPDDR2 or MEMC_DDR_TYPE_DDR3.
 * 
 * ===========================================================================
 */
static inline void chal_sdram_set_ddr_type(MEMC_ID controllerId, MEMC_DDR_TYPE ddrType)
{
    manualDDRType[controllerId] = ddrType;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_sdram_get_ddr_type
 * 
 *   Description: Returns the autodected DDR type.
 *                Returns the manual override if it was set prior.
 *
 * 
 * ===========================================================================
 */
static inline void chal_sdram_get_ddr_type(MEMC_ID controllerId, MEMC_DDR_TYPE *ddrType)
{
    CHAL_MEMC_HANDLE memHandle;
    if ( controllerId == MEMC_ID_SYS || controllerId == MEMC_ID_VC )
    {
       memHandle = &handleMEMC[controllerId];
    }
    else
    {
       *ddrType = MEMC_DDR_TYPE_UNKNOWN;
       return;
    }    
    
    setDDRType(controllerId);
    memHandle = &handleMEMC[controllerId];

    *ddrType = memHandle->memc_ddr_type;

    return;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_sdram_dual_chipselect_config
 * 
 *   Description: Configure dual chip select option in the data structure.
 *                This is used primarily in applications that did not intialize
 *                the memory controller but instead want to query the configuration.
 *                Applicable for LPDDR2 dual chip select only.
 * 
 * ===========================================================================
 */
static inline void chal_sdram_dual_chipselect_config ( MEMC_ID controllerId, _Bool dualChipSelect )
{
   setDeviceChipSelect( controllerId, dualChipSelect );
}

#if 0
/*
 * ===========================================================================
 * 
 *   Function Name: chal_sdram_init
 * 
 *   Description:
 *       Initialize both the MEMC controller and corresponding data structure.
 *       Primarily used by boot code.
 * 
 * ===========================================================================
 */
static inline BCM_ERR_CODE chal_sdram_init(MEMC_ID controllerId, CHAL_SDRAM_CONFIG_T* devCfg)
{
    BCM_ERR_CODE errorCode = BCM_SUCCESS;
    uint32_t ramBaseAddr;
    CHAL_MEMC_HANDLE memHandle;
    uint32_t chipId;
    
    if ( controllerId == MEMC_ID_SYS || controllerId == MEMC_ID_VC )
    {
       memHandle = &handleMEMC[controllerId];
    }
    else
    {
       return BCM_ERROR;
    }    

    if ( setDDRType(controllerId) == BCM_ERROR )
    {
       return BCM_ERROR;
    }

    if ( setContollerBaseAddress( controllerId ) == BCM_ERROR )
    {
       return BCM_ERROR;    
    }    

    /* By default single chip select */
    if ( setDeviceChipSelect( controllerId, FALSE ) == BCM_ERROR )
    {
       return BCM_ERROR;    
    }    

    if ( setOperationMode( controllerId ) == BCM_ERROR )
    {
       return BCM_ERROR;    
    }    
   
    if ( controllerId == MEMC_ID_SYS )
    {
#ifdef LITTLE_ISLAND_MODE
       ramBaseAddr = 0x80000000;    /* this needs replacement with a appropriate CONFIG value */ 
#else
       ramBaseAddr = CONFIG_RAM_BASE;
#endif
    }
    else if ( controllerId == MEMC_ID_VC )
    {
       ramBaseAddr = CONFIG_VCRAM_BASE;
    }
    else
    {
       return BCM_ERROR;
    }

    chipId = chipregHw_getChipId();

    switch ( memHandle->memc_ddr_type )
    {
       case MEMC_DDR_TYPE_LPDDR2:
       {
          _Bool dualChipSelect = FALSE;
          /* On getting handle,initialize memc */
          /* Fill parameters needed to initialize memory controller and required by MEMC CHAL to init */
          if (devCfg == NULL )
          {
             /* Configuration for BCM911160SV board */
             CHAL_SDRAM_CONFIG_T defaultDevConfig = CHAL_SDRAM_CONFIG_LPDDR2_MICRON_MT42L64M32D1KL_300MHz;
              
             memHandle->mem_device[0].dev_config = defaultDevConfig.dev_config.lpddr2_dev_config.lpddr2_cfg;
             memHandle->clock_Mhz = defaultDevConfig.dev_config.lpddr2_dev_config.clk_mhz;

             dualChipSelect = defaultDevConfig.dev_config.lpddr2_dev_config.dual_chip_select;
          }
          else
          {
             if ( devCfg->sdram_type != MEMC_DDR_TYPE_LPDDR2 )
             {
                return BCM_ERROR;
             }

             /* override with input configuration if there is custom configuration*/
             memHandle->mem_device[0].dev_config = devCfg->dev_config.lpddr2_dev_config.lpddr2_cfg;
             memHandle->clock_Mhz = devCfg->dev_config.lpddr2_dev_config.clk_mhz;

             dualChipSelect = devCfg->dev_config.lpddr2_dev_config.dual_chip_select;
          }
        
          if ( dualChipSelect == TRUE )
          {
             /* Configure for dual chip select */
             memHandle->mem_device[1].dev_cs      = MEMC_CS_1;
             memHandle->mem_device[1].dev_type    = MEMC_TYPE_DRAM;

             /* It is a requirement that for dual chip select DRAM configurations that both devices must be identical */
             memHandle->mem_device[1].dev_config.MR1_device_feature1    = memHandle->mem_device[0].dev_config.MR1_device_feature1;
             memHandle->mem_device[1].dev_config.MR2_device_feature2    = memHandle->mem_device[0].dev_config.MR2_device_feature2;
             memHandle->mem_device[1].dev_config.MR3_device_ioconfig    = memHandle->mem_device[0].dev_config.MR3_device_ioconfig;
             memHandle->mem_device[1].dev_config.device_zq_calibration  = memHandle->mem_device[0].dev_config.device_zq_calibration;
          }

          memHandle->memc_memory_base_dram  = ramBaseAddr;
          memHandle->memc_memory_base_nvm   = 0;
          
          if ( controllerId == MEMC_ID_VC && chipregHw_getChipId() == 0x111600A0 )
          {
             /* A0 Version only.  Reduce LPDDR2 speed on VC LPDDR2
               * Souce Syncronizer workaround to match LPDDR2 speed to VC4 core speed
               */
             memHandle->clock_Mhz = 234;
          }
          memHandle->timing0                = MEMC_TIMING0;
          memHandle->timing1                = MEMC_TIMING1;
          memHandle->refresh_ctrl           = MEMC_REFRESH_CTRL;

          memHandle->boot_mode              = MEMC_BOOT_MODE_COLD;
          break;
       }
       case MEMC_DDR_TYPE_DDR3:
       {
          memHandle->memc_memory_base_dram  = ramBaseAddr;

          if (devCfg == NULL )
          {
              /* Configuration for BCM9CHIPIT_4DDR3x8_EDC & BCM911160SV board */
              CHAL_SDRAM_CONFIG_T defaultDevConfig = CHAL_SDRAM_CONFIG_DDR3_2Gbx8_400MHz;
              
              memHandle->mem_device_ddr3.dev_config = defaultDevConfig.dev_config.ddr3_dev_config;
          }
          else
          {
             if ( devCfg->sdram_type != MEMC_DDR_TYPE_DDR3 )
             {
                return BCM_ERROR;
             }
              /* override with input configuration if there is custom configuration*/
              memHandle->mem_device_ddr3.dev_config = devCfg->dev_config.ddr3_dev_config;
          }

          /* Configure DDR3 clock */
          memHandle->clock_Mhz = (memHandle->mem_device_ddr3.dev_config.clock_hz + 500000) / 1000000;

          memHandle->boot_mode              = MEMC_BOOT_MODE_COLD;

          break;
       }
       case MEMC_DDR_TYPE_UNKNOWN:
       /* fall through */
       default:
       {
          return BCM_ERROR;
       }
         
    }
       
    if ( (chipId == 0x710) || (chipId == 0x711) ) /* Big Island */
    {
       /* lower the APB clock frequency to allow memory controller programming */
       chal_ccu_set_khubaon_policy_freq(chal_ccu_policy_0, chal_ccu_khubaon_policy_freq_104_52);
       chal_ccu_set_khubaon_policy_freq(chal_ccu_policy_1, chal_ccu_khubaon_policy_freq_104_52);
       chal_ccu_set_khubaon_policy_freq(chal_ccu_policy_2, chal_ccu_khubaon_policy_freq_104_52);
       chal_ccu_set_khubaon_policy_freq(chal_ccu_policy_3, chal_ccu_khubaon_policy_freq_104_52);
    }    

    errorCode = chal_memc_init(memHandle);

    /* Disable temperature polling */
    chal_memc_temperature_polling(memHandle,0);

    return errorCode;
};
#endif
/*
 * ===========================================================================
 * 
 *   Function Name: reinitHandle
 * 
 *   Description:
 *       Initialize the corresponding MEMC data structure, but don't reinit controller.
 *       The controller has already been initialized and SDRAM is already
 *          operational.
 *       Re-init the data structure with minimum required for chal_mem API's
 * 
 * 
 * ===========================================================================
 */
static BCM_ERR_CODE reinitHandle(MEMC_ID controllerId)
{
    
    if ( setDDRType( controllerId ) == BCM_ERROR )
    {
       return BCM_ERROR;
    }
    
    if ( setContollerBaseAddress( controllerId ) == BCM_ERROR )
    {
       return BCM_ERROR;    
    }
    
    /* By default single chip select */
    if ( setDeviceChipSelect( controllerId, FALSE ) == BCM_ERROR )
    {
       return BCM_ERROR;    
    }    

    if ( setOperationMode( controllerId ) == BCM_ERROR )
    {
       return BCM_ERROR;    
    }    

    return BCM_SUCCESS;
}


static BCM_ERR_CODE setDDRType(MEMC_ID controllerId)
{
    uint32_t chipId;
    CHAL_MEMC_HANDLE memHandle = NULL;

    if ( manualDDRType[controllerId] != MEMC_DDR_TYPE_UNKNOWN )
    {
       /* Manual DDR Type selected */
       memHandle = &handleMEMC[controllerId];

       memHandle->memc_ddr_type = manualDDRType[controllerId];
    }
    else if ( controllerId == MEMC_ID_SYS )
    {
       memHandle = &handleMEMC[controllerId];

       /* Auto-detect */
       chipId = chipregHw_getChipId();

       if ( (chipId & 0xfffff000) >> 12 == 0x11160) /* Hana: Masked out revision */
       {
           memHandle->memc_ddr_type = MEMC_DDR_TYPE_DDR3;
       }
       else if ( ( (chipId & 0xfffff000) >> 12 == 0x11140 ) || 
                 ( (chipId & 0xfffff000) >> 12 == 0x11130 ) || 
                 ( (chipId & 0xfffff000) >> 12 == 0x11351 ) ) /* Capri: Masked out revision */
       {
           uint32_t strap = CHAL_REG_READ32 (MM_IO_BASE_CHIPREG + CHIPREG_STRAP_OFFSET);
           if ( strap & CHIPREG_STRAP_SYS_EMI_DDR3_MODE_MASK )
           {
              memHandle->memc_ddr_type = MEMC_DDR_TYPE_DDR3;
           }
           else
           {
              memHandle->memc_ddr_type = MEMC_DDR_TYPE_LPDDR2; 
           }
       }
       else if ( (chipId == 0x710) || (chipId == 0x711) ) /* Big Island */
       {
           memHandle->memc_ddr_type = MEMC_DDR_TYPE_LPDDR2;
       }
       else if ( (chipId == 0x31F )) /* Hera */
       {
           memHandle->memc_ddr_type = MEMC_DDR_TYPE_LPDDR2;
       }
       else
       {
           memHandle->memc_ddr_type = MEMC_DDR_TYPE_UNKNOWN;
       }
    }
    else if ( controllerId == MEMC_ID_VC )
    {
       memHandle = &handleMEMC[controllerId];
       memHandle->memc_ddr_type = MEMC_DDR_TYPE_LPDDR2;
    }
    else
    {
       return BCM_ERROR;
    }

    return BCM_SUCCESS;
    
}    

static BCM_ERR_CODE setContollerBaseAddress(MEMC_ID controllerId)
{
    CHAL_MEMC_HANDLE memHandle;

    if ( controllerId == MEMC_ID_SYS || controllerId == MEMC_ID_VC )
    {
       memHandle = &handleMEMC[controllerId];
    }
    else
    {
       return BCM_ERROR;
    }    
    
    switch ( memHandle->memc_ddr_type )
    {
       case MEMC_DDR_TYPE_LPDDR2:
       {
          if ( controllerId == MEMC_ID_SYS )
          {
             memHandle->memc_secure_reg_base   = MEMC0_SECURE_BASE_ADDR;
             memHandle->memc_open_reg_base     = MEMC0_OPEN_BASE_ADDR;
             memHandle->memc_aphy_reg_base     = MEMC0_OPEN_APHY_BASE_ADDR;
             memHandle->memc_dphy_reg_base     = MEMC0_OPEN_DPHY_BASE_ADDR;
          }
          else if ( controllerId == MEMC_ID_VC )
          {
             memHandle->memc_secure_reg_base   = VC4_EMI_SECURE_BASE_ADDR;
             memHandle->memc_open_reg_base     = VC4_EMI_OPEN_BASE_ADDR;
             memHandle->memc_aphy_reg_base     = VC4_EMI_OPEN_APHY_BASE_ADDR;
             memHandle->memc_dphy_reg_base     = VC4_EMI_OPEN_DPHY_BASE_ADDR;
          }
          break;
       }
       case MEMC_DDR_TYPE_DDR3:
       {
          memHandle->memc_secure_reg_base   = MEMC0_SECURE_BASE_ADDR;
          memHandle->memc_open_reg_base     = MEMC0_OPEN_BASE_ADDR;
          memHandle->memc_open_ddr3_ctl_base           = MEMC0_OPEN_APHY_BASE_ADDR;
          memHandle->memc_open_ddr40_phy_addr_ctl_base = MEMC0_OPEN_DPHY_BASE_ADDR;
          memHandle->memc_open_ddr40_phy_wl_0_base     = SYS_EMI_DDR3_PHY_WL_0_BASE_ADDR;
          memHandle->memc_open_ddr40_phy_wl_1_base     = SYS_EMI_DDR3_PHY_WL_1_BASE_ADDR;
          break;
       }
       case MEMC_DDR_TYPE_UNKNOWN:
       /* fall through */
       default:
       {
          return BCM_ERROR;
       }
    }
    return BCM_SUCCESS;
}    


static BCM_ERR_CODE setDeviceChipSelect(MEMC_ID controllerId, _Bool dualChipSelect)
{
    CHAL_MEMC_HANDLE memHandle;

    if ( controllerId == MEMC_ID_SYS || controllerId == MEMC_ID_VC )
    {
       memHandle = &handleMEMC[controllerId];
    }
    else
    {
       return BCM_ERROR;
    }    
    
    if ( memHandle->memc_ddr_type == MEMC_DDR_TYPE_LPDDR2 )
    {
       /* On getting handle,initialize memc */
       /* Fill parameters needed to initialize memory controller and required by MEMC CHAL to init */
       memHandle->mem_device[0].dev_cs      = MEMC_CS_0;
       memHandle->mem_device[0].dev_type    = MEMC_TYPE_DRAM;

       if ( dualChipSelect )
       {
          /* Dual chip select */
          memHandle->mem_device[1].dev_cs      = MEMC_CS_1;
          memHandle->mem_device[1].dev_type    = MEMC_TYPE_DRAM;
       }
       else
       {
          /* Single chip select - Configure the 2nd select to NONE */
          memHandle->mem_device[1].dev_cs      = MEMC_CS_NONE;
          memHandle->mem_device[1].dev_type    = MEMC_TYPE_NONE;
       }
    }
    
    return BCM_SUCCESS;
}

static BCM_ERR_CODE setOperationMode(MEMC_ID controllerId)
{
    CHAL_MEMC_HANDLE memHandle;

    if ( controllerId == MEMC_ID_SYS || controllerId == MEMC_ID_VC )
    {
       memHandle = &handleMEMC[controllerId];
    }
    else
    {
       return BCM_ERROR;
    }    
    
    memHandle->operation_mode         = MEMC_OP_MODE_ASIC;
    
    return BCM_SUCCESS;
}

#endif
