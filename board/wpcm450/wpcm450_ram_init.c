/******************************************************************************
 *
 * Copyright (c) 2003 Windond Electronics Corp.
 * All rights reserved.
 *
 * $Workfile: ram_init.c $
 *
 * $Author$
 ******************************************************************************/

#include "cdefs.h"
#include "com_defs.h"
#include "wpcm450_gctrl_regs.h"
#include "wpcm450_clk_regs.h"
#include "wpcm450_timer_regs.h"
#include "wpcm450_mc_regs.h"
#include "clock_init.h"
#include "ram_init.h"



/************************************************************************/
/* TYPES DEFINITIONS                                                    */
/************************************************************************/


#define WAIT_MC_RESET(cnt)  {for(cnt=0;cnt<10000;){cnt++;}}


/************************************************************************/
/* TYPES DEFINITIONS                                                    */
/************************************************************************/
typedef struct {
  UINT32 mc_cfg0[7];
  UINT32 mc_cfg1[7];
  UINT32 mc_cfg2[7];
  UINT16 mc_cfg3;
  UINT8  mc_cfg4[7];
  UINT32 mc_cfg5;
  UINT32 mc_cfg6;
  UINT32 mc_cfg7;
  UINT32 mc_p1_arbt;
  UINT32 mc_p1_cnt;
  UINT32 mc_p2_arbt[7];
  UINT32 mc_p2_cnt;
  UINT32 mc_p3_arbt;
  UINT32 mc_p3_cnt;
  UINT32 mc_p4_arbt;
  UINT32 mc_p4_cnt;
  UINT32 mc_p5_arbt;
  UINT32 mc_p5_cnt;
  UINT32 mc_p6_arbt;
  UINT32 mc_p6_cnt;
  UINT32 mc_p1_incrs;
  UINT32 mc_p2_incrs;
  UINT32 mc_p3_incrs;
  UINT32 mc_p4_incrs;
  UINT32 mc_dll_0;
  UINT32 mc_dll_1;
}MemCtrlCfg_t;


/************************************************************************/
/* GLOBAL VARIABLES                                                     */
/************************************************************************/

/************************************************************************/
/* AHB/STREAMING DDR-2 CONTROLLER Configuration                         */
/************************************************************************/


const MemCtrlCfg_t MC_Cfg[3] =
{
  /* MEM_128MB_MODE */
  {
    /* 180 MHz    133 MHz     250 MHz     160MHz      220 MHz     125MHz      200 MHz  */
    {0x241B457C, 0x2312240E, 0x3624079E, 0x239AA4E0, 0x362406B4, 0x231223CE, 0x241B4618},
    {0x22162323, 0x21162223, 0x22162424, 0x22162323, 0x22162424, 0x21162223, 0x22162323},
    {0x40000643, 0x40000443, 0x40000843, 0x40000643, 0x40000843, 0x40000443, 0x40000643},
    0x6,
    {0xA,        0x7,        0xD,        0x8,        0xD,        0x7,        0xA       },
    0x190202, 
    0x0, 
    0x1,
    0x23,
    0x5,
    {0x2A,       0x1E,       0x30,       0x24,       0x30,       0x1E,       0x2A    },
    0x1,
    0x64,
    0x0,
    0x64,
    0x0,
    0x1B8,
    0x5,
    0xC,
    0x0,
    0x2,
    0x2,
    0x4,
    0x4,
    0x00010106,
    0x00000106
   },
  /* MEM_64MB_MODE */
  {
    /* 180 MHz     133 MHz     250 MHz     160MHz      220 MHz    125MHz      200 MHz  */
    {0x241AA57C, 0x2311C40E, 0x3623679E, 0x239A24E0, 0x362366B4, 0x2311C3CE, 0x241AA618},
    {0x22122323, 0x21122223, 0x22122424, 0x22122323, 0x22122424, 0x21122223, 0x22122323},
    {0x40000643, 0x40000443, 0x40000843, 0x40000643, 0x40000843, 0x40000443, 0x40000643},
    0x6,
    {0xA,        0x7,        0xD,        0x8,        0xD,        0x7,        0xA       },
    0x550202,
    0x0,
    0x1,
    0x23,
    0x5,
    {0x2A,       0x1E,       0x30,       0x24,       0x30,       0x1E,       0x2A      },
    0x1,
    0x64,
    0x0,
    0x64,
    0x0,
    0x1B8,
    0x5,
    0xC,
    0x0,
    0x2,
    0x2,
    0x4,
    0x4,
    0x00010106,
    0x00000106
  },
  /* MEM_32MB_MODE */
  {
    /*180 MHz     133 MHz     250 MHz     160MHz      220 MHz     125MHz      200 MHz   */
    {0x249B857C, 0x2312A40E, 0x3624E79E, 0x241B24E0, 0x352466B4, 0x231283CE, 0x249BE618},
    {0x220E2324, 0x210E2224, 0x220E2424, 0x220E2324, 0x220E2424, 0x110E2224, 0x220E2324},
    {0x40000643, 0x40000443, 0x40000843, 0x40000643, 0x40000843, 0x40000443, 0x40000643},
    0x6,
    {0x9,        0x7,        0xD,        0x8,        0xB,        0x7,        0xA       },
    0x310202,
    0x0,
    0x1,
    0x23,
    0x5,
    {0x30,       0x1E,       0x30,       0x24,       0x30,       0x1E,      0x30       },
    0x1,
    0x64,
    0x0,
    0x64,
    0x0,
    0x1B8,
    0x5,
    0xC,
    0x0,
    0x2,
    0x2,
    0x4,
    0x4,
    0x00010106,
    0x00000106
  }
};


/************************************************************************/
/* EXTERNAL VARIABLES                                                   */
/************************************************************************/


/************************************************************************/
/* EXTERNAL FUNCTIONS                                                   */
/************************************************************************/


void WBL_RamInit(void)
{
     
  UINT32 idx1;
  UINT32 idx2;
  UINT32 pwron_cfg = PWRON;

  /************************************************************************/
  /* If watchdog reset was done. Don't do any memory controller init      */
  /************************************************************************/
  if(IS_BIT_SET(WTCR, WTCR_RESET_FLAG_BIT)) return;

  /************************************************************************/
  /*  RAM Configuration Part                                              */
  /************************************************************************/


  /************************************************************************/
  /*  idx1 is RAM size index                                              */
  /************************************************************************/
  idx1 = GET_FIELD(pwron_cfg, PWRON_RAM_SIZE);


  if(idx1 == MEM_SKIP_INIT_MODE)
  {
    idx1 = MEM_128MB_MODE;
  }
  
  if(idx1 == MEM_128MB_MODE)
  {
    SET_BIT(MFSEL1, MF_MBEN_BIT);
  }
  
  /************************************************************************/
  /* Release memory Controller from reset by                              */
  /* clearing Memory Controller reset bit                                 */
  /************************************************************************/
  
  CLEAR_BIT(CLK_IPSRST,CLK_IPSRST_MC_BIT);    
  

  /* Wait Loop */
  WAIT_MC_RESET(idx2);

    
  /************************************************************************/
  /*  idx2 is clock frequency index                                       */
  /************************************************************************/
  idx2 = GET_FIELD(pwron_cfg, PWRON_CPU_CORE_CLK);


  
  idx1 -=1; /* MC_Cfg array not contain SKIP mode configuration values */

  if((idx2 != CLK_BYPASS_MODE))
  {
    idx2 = CLK_250MHz_MODE; /* Configured MC to 250MHz mode */
  }

  idx2 -=1; /* MC_CFG array not contain BY_PASS mode configuration values  */


  MC_CFG0     = MC_Cfg[idx1].mc_cfg0[idx2];
  MC_CFG1     = MC_Cfg[idx1].mc_cfg1[idx2];
  MC_CFG2     = MC_Cfg[idx1].mc_cfg2[idx2];
  MC_CFG3     = MC_Cfg[idx1].mc_cfg3;
  MC_CFG4     = MC_Cfg[idx1].mc_cfg4[idx2];
  MC_CFG5     = MC_Cfg[idx1].mc_cfg5;
  MC_CFG6     = MC_Cfg[idx1].mc_cfg6;
  //MC_CFG7     = MC_Cfg[idx1].mc_cfg7;
  MC_P1_ARBT  = MC_Cfg[idx1].mc_p1_arbt;
  MC_P1_CNT   = MC_Cfg[idx1].mc_p1_cnt;
  MC_P2_ARBT  = MC_Cfg[idx1].mc_p2_arbt[idx2];
  MC_P2_CNT   = MC_Cfg[idx1].mc_p2_cnt;
  MC_P3_ARBT  = MC_Cfg[idx1].mc_p3_arbt;
  MC_P3_CNT   = MC_Cfg[idx1].mc_p3_cnt;
  MC_P4_ARBT  = MC_Cfg[idx1].mc_p4_arbt;
  MC_P4_CNT   = MC_Cfg[idx1].mc_p4_cnt;
  MC_P5_ARBT  = MC_Cfg[idx1].mc_p5_arbt;
  MC_P5_CNT   = MC_Cfg[idx1].mc_p5_cnt;
  MC_P6_ARBT  = MC_Cfg[idx1].mc_p6_arbt;
  MC_P6_CNT   = MC_Cfg[idx1].mc_p6_cnt;
  MC_P1_INCRS = MC_Cfg[idx1].mc_p1_incrs;
  MC_P2_INCRS = MC_Cfg[idx1].mc_p2_incrs;
  MC_P3_INCRS = MC_Cfg[idx1].mc_p3_incrs;
  MC_P4_INCRS = MC_Cfg[idx1].mc_p4_incrs;

  return;
  
}




