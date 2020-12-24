/******************************************************************************
 *
 * Copyright (c) 2003 Windond Electronics Corp.
 * All rights reserved.
 *
 * $Workfile: wbl_info.c $
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





/************************************************************************/
/*  GLOBAL VARIABLES                                                    */
/************************************************************************/

const UINT32 CLK_DIV_REG_Val[] = {
  CLK_180MHZ_DIV_REG_CFG,
  CLK_133MHZ_DIV_REG_CFG,
  CLK_250MHZ_DIV_REG_CFG,
  CLK_160MHZ_DIV_REG_CFG,
  CLK_220MHZ_DIV_REG_CFG,
  CLK_125MHZ_DIV_REG_CFG,
  CLK_200MHZ_DIV_REG_CFG
};

const UINT32 CLK_PLLCON_REG_Val[] = {
  CLK_180MHZ_PLLCON_REG_CFG,
  CLK_133MHZ_PLLCON_REG_CFG,
  CLK_250MHZ_PLLCON_REG_CFG,
  CLK_160MHZ_PLLCON_REG_CFG,
  CLK_220MHZ_PLLCON_REG_CFG,
  CLK_125MHZ_PLLCON_REG_CFG,
  CLK_200MHZ_PLLCON_REG_CFG
};



/************************************************************************/
/* EXTERNAL VARIABLES                                                   */
/************************************************************************/



/*****************************************************************************
 *  Name : WBL_ClockInit
 *
 *  Description:
 *
 *  Params :
 *       ClkScheme_t cfg_scheme
 *
 *  Return : none
 *
 *****************************************************************************/
void WBL_ClockInit (void)
{
  
  UINT32 pwron_cfg = PWRON;
  UINT32 idx1;
  UINT32 idx2;


  /************************************************************************/
  /* If watchdog reset was done. Don't do any clock init                  */
  /************************************************************************/
  if(IS_BIT_SET(WTCR, WTCR_RESET_FLAG_BIT))
  {
    return;
  }

  /************************************************************************/
  /*  Till PLL's not configured all clock's sources will from             */
  /*  RMII reference clock. It's default value(0x0000022A). But will done */
  /*  for more .......                                                    */
  /************************************************************************/


  /************************************************************************/
  /*  Read STRAP clock configuration                                      */
  /************************************************************************/
  idx1 = GET_FIELD(pwron_cfg, PWRON_CPU_CORE_CLK);

  if(idx1 != CLK_BYPASS_MODE)
  {
    idx1 -= 1; /* Clock configuration arrays not contain BY_PASS mode configuration values  */

    SET_BIT(CLK_PLLCON0,CLK_PRST_BIT);
      
      
    CLK_PLLCON0 = CLK_PLLCON_REG_Val[idx1];
      
    /* Wait Loop */
    CLK_DELAY_10_MICRO_SEC(idx2);
        
    CLEAR_BIT(CLK_PLLCON0,CLK_PRST_BIT);
  
    /* Wait Loop */
    CLK_DELAY_500_MICRO_SEC(idx2);

    /************************************************************************/
    /* Set clock divider accordingly setted PLL configuration               */
    /************************************************************************/
    CLK_DIV     = CLK_DIV_REG_Val[idx1];

    /* Wait Loop 200 cycles */
    CLK_DELAY_10_MICRO_SEC(idx2);
        
    /************************************************************************/
    /* Set clock sources selection to PLLCON0                               */
    /************************************************************************/
    SET_FIELD(CLK_SEL,CLK_CPU_CLK, CLK_PLL0_SRC_SEL );
    SET_FIELD(CLK_SEL,CLK_CPU_OUT, CLK_PLL0_SRC_SEL );
    SET_FIELD(CLK_SEL,CLK_PIX_SRC, CLK_PIX_PLL_GFX_SEL );
  }
  else
  {
    /************************************************************************/
    /* By pass mode                                                         */
    /* Bypass mode: Core clock from reference clock PWM3/GPIO83 pin as      */
    /* input and pixel clock from GPI96. Reference clock 24 MHz and the SPI */
    /* clock is 3 MHz by default register values.                           */
    /************************************************************************/
      
    SET_BIT(MFSEL2, MF_PWM3SEL_BIT);
    SET_FIELD(CLK_SEL,CLK_CPU_CLK, CLK_PWM3_GPIO83_SRC_SEL);
    CLEAR_BIT(MFSEL1, MF_CLKOSEL_BIT);
    SET_FIELD(CLK_SEL,CLK_PIX_SRC, CLK_PIX_GPI96_SEL );
  }

  SET_FIELD(CLK_SEL,CLK_USB1CKSEL, CLK_PLL1_SRC_SEL );

	return;
}




