/* ******************************************************************************
 *
 * Copyright (c) 2003 Windond Electronics Corp.
 * All rights reserved.
 *
 * $Workfile: init.s $
 *
 * $Author$
 ******************************************************************************/

#include <config.h>
#include <version.h>

_TEXT_BASE:
    .word   TEXT_BASE   /* sdram load addr from config.mk */

.globl lowlevel_init
lowlevel_init:


#if 0
#define LL_DEBUG
#endif


/*  
;----------------------------------------------------------
; Disable Interrupt, This is for safe ...
;----------------------------------------------------------
*/
    LDR     r0, =AIC_MDCR
    LDR     r1, =AIC_MASKALL
    STR     r1, [r0]
    MRS     r0, CPSR
    ORR     r0, r0, #0xC0
    MSR     CPSR_c, r0


/*
;----------------------------------------------------------
; If Watchdog Reset was set and code arrive to this point
; that mean FIU work properly . In this case flag bit in 
; INTCR2 register and Watchdog Register must be reset to
; to default values.
;----------------------------------------------------------
*/
#if 1
    LDR     r0, =TMR_WTCR
    LDR     r1, [r0]
    /* TST     r1, #0x2 */
    
#ifdef LL_DEBUG
    MOV     r3, #0x88
    STR     r1, [r3]
#endif
    
    /* check the watchdog timer reset flag */
    AND     r2, r1, #0x00000004
    CMP     r2, #0x00000004
    
    /* jump if it is not a watchdog timer reset */
    BNE     init_clocks
    
    /* clear watchdog timer reset flag */
    MOV     r1, #0x4
    STR     r1, [r0]
    
#ifdef LL_DEBUG
    MOV     r3, #0xa0
    STR     r1, [r3]
#endif
    
    /* restore the default setting for watchdog timer control register */
    /* MOV     r1, #0x400 */
    MOV     r1, #0x00   /* intend to write a value different than default */
    STR     r1, [r0]
    
#ifdef LL_DEBUG
    MOV     r3, #0xa4
    STR     r1, [r3]
#endif
    
    LDR     r0, =GCR_INTCR2
    LDR     r1, [r0]
    /* MOV     r2, #0x80000000 */
    /* BIC     r1, r2 */
    BIC     r1, r1, #0x80000000
    STR     r1, [r0]
    
#ifdef LL_DEBUG
    MOV     r0, #0x80
    MOV     r1, #0x01
    STR     r1, [r0]
#endif
    
#ifdef CONFIG_COMPATIBLE_Z1_CHIP
    /* check chip version is Z1 */
    LDR     r8, =GCR_BA
    LDR     r0, [r8, #0x00]     /* Read PDID, product identifier register */
    
    MOVS    r0, r0, LSR #24     /* shift right 24 bits */
    
    /* 
    PDID product identifier register
    #define PDID_Z1     0x00926450
    #define PDID_Z2     0x03926450
    #define PDID_Z21    0x04926450
    #define PDID_A1     0x08926450
    #define PDID_A2     0x09926450
    #define PDID_A3     0x0A926450
    */
    
    CMP     r0, #0x03
    BLO     init_clocks         /* still initiate if less than 3, Z1 chip */
#endif
    
    /* skip clock init and memory controller init if it is a watchdog reset */
    B       skip_init
#else

    LDR     r0, =TMR_WTCR
    LDR     r1, [r0]
    TST     r1, #0x2
    BEQ     init_clocks
    MOV     r1, #0x400
    STR     r1, [r0]
    LDR     r0, =GCR_INTCR2
    LDR     r1, [r0]
    /* MOV     r2, #0x80000000 */
    /* BIC     r1, r2 */
    BIC     r1, r1, #0x80000000
    STR     r1, [r0]
    
#ifdef LL_DEBUG
    MOV     r0, #0x80
    MOV     r1, #0x02
    STR     r1, [r0]
#endif
    
    /* skip clock init and memory controller init if it is a watchdog reset */
    B       skip_init

#endif


init_clocks:

#ifdef LL_DEBUG
    MOV     r0, #0x84
    MOV     r1, #0x00
    STR     r1, [r0]
#endif


#ifdef CONFIG_WPCM450_RAM_VER
    B       skip_init   /* bypass low level init */
#endif


/************************************************************************/
/*   Init Clocks                                                        */
/************************************************************************/

#ifdef CONFIG_COMPATIBLE_Z1_CHIP
    /* check chip version is not Z1 */
    LDR     r8, =GCR_BA
    LDR     r0, [r8, #0x00]     /* Read PDID, product identifier register */
    
    MOVS    r0, r0, LSR #24     /* shift right 24 bits */
    
    /* 
    PDID product identifier register
    #define PDID_Z1     0x00926450
    #define PDID_Z2     0x03926450
    #define PDID_Z21    0x04926450
    #define PDID_A1     0x08926450
    #define PDID_A2     0x09926450
    #define PDID_A3     0x0A926450
    */
    
    CMP     r0, #0x03
    BLO     init_clocks_start   /* skip checking if less than 3, Z1 chip */
    
#ifdef LL_DEBUG
TEST_REG_0:     .word   0x00000090

    LDR     R2, TEST_REG_0
    MOV     R3, #0x11
    STR     R3, [R2, #0x0]
#endif
    
#endif


/************************************************************************/
/* If clock source isn't 48MHz that mean clock initialisation done by   */
/* ROM Code.                                                            */
/************************************************************************/
/*
    if (GET_FIELD(CLK_SEL,CLK_CPU_CLK) !=  CLK_48M_SRC_SEL )
    {
    return;
    }
*/
    
    LDR     r8, =GCR_BA
    LDR     r0, [r8, #0x204]    /* Read CLKSEL register */
    
    AND     r2, r0, #0x00000003
    CMP     r2, #0x00000002
    
    BNE     end_clk_init        /* skip initiating clock if rom did already */


init_clocks_start:


/************************************************************************/
/*  Read STRAP clock configuration                                      */
/************************************************************************/
/* idx1 = GET_FIELD(power_cfg, PWRON_CPU_CORE_CLK);                     */
/************************************************************************/
    /* LDR     r8, =GCR_BA */
    LDR     r0, [r8, #4]        /* Read PWRON register */
    MOVS    r2, r0, LSL #21     /* LSL   r2,r0,#21 */
    MOVS    r2, r2, LSR #29     /* LSR   r2,r2,#29 */


/************************************************************************/
/* if(idx1 == CLK_BYPASS_MODE)                                          */
/************************************************************************/
    BEQ     clk_bypass_mode_init
    
    ADD     r7, pc, #0
    B       clk_init_func


/************************************************************************/
/* PLL Clock Divider + PLLCON configurayion values                      */
/************************************************************************/
clk_div_pll0con_reg_val:
.word  CLK_180MHZ_DIV_REG_CFG, CLK_180MHZ_PLLCON_REG_CFG
.word  CLK_133MHZ_DIV_REG_CFG, CLK_133MHZ_PLLCON_REG_CFG
.word  CLK_250MHZ_DIV_REG_CFG, CLK_250MHZ_PLLCON_REG_CFG
.word  CLK_160MHZ_DIV_REG_CFG, CLK_250MHZ_PLLCON_REG_CFG
.word  CLK_220MHZ_DIV_REG_CFG, CLK_220MHZ_PLLCON_REG_CFG
.word  CLK_125MHZ_DIV_REG_CFG, CLK_125MHZ_PLLCON_REG_CFG
.word  CLK_200MHZ_DIV_REG_CFG, CLK_200MHZ_PLLCON_REG_CFG


clk_init_func:         
/************************************************************************/         
/* {                                                                    */
/*   Clock configuration arrays not contain BY_PASS mode configuration values  */
/*   idx1 -= 1;                                                         */
/************************************************************************/
    SUB     r4, r2, #1

/************************************************************************/
/*   SET_BIT(CLK_PLLCON0,CLK_PRST_BIT);                                 */
/************************************************************************/
/*
    LDR     r2, [r8, #0x20c]
    ORR     r2, r2, #0x2000
    STR     r2, [r8, #0x20c]
*/          
/************************************************************************/          
/*   CLK_PLLCON0 = CLK_PLLCON_REG_Val[idx1];                            */
/************************************************************************/

    /* check chip version is prior to A2 */
    LDR     r6, =GCR_BA
    LDR     r0, [r6, #0x00]     /* Read PDID, product identifier register */
    
    MOVS    r0, r0, LSR #24     /* shift right 24 bits */
    
    /* 
    PDID product identifier register
    #define PDID_Z1     0x00926450
    #define PDID_Z2     0x03926450
    #define PDID_Z21    0x04926450
    #define PDID_A1     0x08926450
    #define PDID_A2     0x09926450
    #define PDID_A3     0x0A926450
    */
    
    CMP     r0, #0x09
    BLO     slowdown_clocks         /* slowdown if less than 9, A2 chip */
    
normal_clock:
    ADD     r9, r7, #4
    LDR     r2, [r9, r4, LSL #3]
    STR     r2, [r8, #0x20c]
    
    B       pllcon_init_end
    
/* slow down clcok from 220 to 216 MHz to workaround for the Winbond reset 
   errata, this only applies for A1 and earlier silicon */
slowdown_clocks:
    /* if clock is set to 220 MHz */
    CMP     r4, #4
    
    BEQ     do_slowdown_clocks
    B       normal_clock
    
do_slowdown_clocks:
    LDR     r2, SLOWDOWN_CLK_220MHZ_PLLCON_REG_CFG
    STR     r2, [r8, #0x20c]
    
    B       pllcon_init_end
    
SLOWDOWN_CLK_220MHZ_PLLCON_REG_CFG:     
    .word   0x00352005
    
pllcon_init_end:


/************************************************************************/
/*   Wait Loop                                                          */
/*   WAIT_DELAY(idx2, DELAY_10_MICRO_SEC);                              */
/************************************************************************/
    MOV     r1, #0
    MOV     r2, #20
    
wait_pll0_10micro:
    CMP     r2, r1
    ADDHI   r1, r1, #1
    BHI     wait_pll0_10micro

/************************************************************************/
/*   CLEAR_BIT(CLK_PLLCON0,CLK_PRST_BIT);                               */
/************************************************************************/
    LDR     r1, [r8, #0x20c]
    BIC     r1, r1, #0x2000
    STR     r1, [r8, #0x20c]

/************************************************************************/
/*   Wait Loop                                                          */
/*   WAIT_DELAY(idx2, DELAY_500_MICRO_SEC);                             */
/************************************************************************/
    MOV      r1, #0
    MOV      r2, #1000
    
wait_reset_500micro:
    CMP     r2, r1
    ADDHI   r1, r1, #1
    BHI     wait_reset_500micro


/************************************************************************/
/* Set clock divider accordingly setted PLL configuration               */
/************************************************************************/
/*   CLK_DIV     = CLK_DIV_REG_Val[idx1];                               */
/************************************************************************/
    LDR     r1, [r7, r4, LSL #3]
    STR     r1, [r8, #0x208]

/************************************************************************/    
/*   Wait Loop 200 cycles                                               */
/*   WAIT_DELAY(idx2, DELAY_10_MICRO_SEC);                              */
/************************************************************************/
    MOV     r1, #0
    MOV     r2, #20
    
wait_div_10micro:      
    CMP     r2, r1
    ADDHI   r1, r1, #1
    BHI     wait_div_10micro


/************************************************************************/
/* Set clock sources selection to PLLCON0                               */
/************************************************************************/
/*   SET_FIELD(CLK_SEL,CLK_CPU_CLK, CLK_PLL0_SRC_SEL );                 */
/************************************************************************/
    LDR     r1, [r8, #0x204]
    BIC     r1, r1, #0x003

/************************************************************************/
/*   SET_FIELD(CLK_SEL,CLK_CPU_OUT, CLK_PLL0_SRC_SEL );                 */
/************************************************************************/
    BIC     r1, r1, #0x00c
    ORR     r1, r1, #0x008

/************************************************************************/
/*   SET_FIELD(CLK_SEL,CLK_PIX_SRC, CLK_PIX_PLL_GFX_SEL );              */
/************************************************************************/
    BIC     r1, r1, #0x030

/************************************************************************/
/*   set USB1CKSEL to 01                                                */
/************************************************************************/
    BIC     r1, r1, #0x0c0
    ORR     r1, r1, #0x040

/************************************************************************/
/*   set UARTCKSEL to 10                                                */
/************************************************************************/
    BIC     r1, r1, #0x300
    ORR     r1, r1, #0x200

/************************************************************************/
/*   set HUARTSEL to 00                                                 */
/************************************************************************/
    BIC     r1, r1, #0xc00
    
    STR     r1, [r8, #0x204]
    B       end_clk_init
/* 
   }
   else
   { 
*/

clk_bypass_mode_init:

/************************************************************************/
/* By pass mode                                                         */
/* Bypass mode: Core clock from reference clock PWM3/GPIO83 pin as      */
/* input and pixel clock from GPI96. Reference clock 24 MHz and the SPI */
/* clock is 3 MHz by default register values.                           */
/************************************************************************/
/*   SET_BIT(MFSEL2, MF_PWM3SEL_BIT);                                   */
/*   SET_FIELD(CLK_SEL,CLK_CPU_CLK, CLK_PWM3_GPIO83_SRC_SEL);           */
/*   CLEAR_BIT(MFSEL1, MF_CLKOSEL_BIT);                                 */
/*   SET_FIELD(CLK_SEL,CLK_PIX_SRC, CLK_PIX_GPI96_SEL );                */
/************************************************************************/
    /* STR     r1,[r8,#0x204] */

/************************************************************************/
/* }                                                                    */
/* SET_FIELD(CLK_SEL,CLK_USB1CKSEL, CLK_PLL1_SRC_SEL );                 */
/************************************************************************/
    LDR     r1, [r8, #0x204]
    BIC     r1, r1, #0xc0
    ORR     r1, r1, #0x40
    STR     r1, [r8, #0x204]

/************************************************************************/
/* Set HUART clock source to 24Mhz                                      */
/************************************************************************/
/*  SET_BIT(CLK_SEL, CLK_HUART_SRC_BIT);                                */
/************************************************************************/
    LDR     r1, [r8, #0x204]
    ORR     r1, r1, #0x400
    STR     r1, [r8, #0x204]

end_clk_init:


/*
;----------------------------------------------------------
; --- Init Memory Controller
;----------------------------------------------------------
*/


/************************************************************************/
/*  idx1 is RAM size index                                              */
/************************************************************************/
/*     idx1 = GET_FIELD(power_cfg, PWRON_RAM_SIZE);                     */
/************************************************************************/
    LDR     r8, =GCR_BA
    LDR     r0, [r8, #4]        /* Read PWRON register */

    /* Get STRAP 3-4 value bits 2-3 */
    MOVS    r2, r0, LSL #28     /* LSL   r2,r0,#28 */
    MOVS    r1, r2, LSR #30     /* LSR   r1,r2,#30 */

/************************************************************************/
/* Release memory Controller from reset by                              */
/* clearing Memory Controller reset bit                                 */
/************************************************************************/
/*        if(idx1 == MEM_128MB_MODE)                                    */
/*        {                                                             */
/*          SET_BIT(MFSEL1, MF_MBEN_BIT);                               */
/*        }                                                             */
/************************************************************************/
#ifdef CONFIG_WPCM450_SVB
    CMP     r1, #0
    LDREQ   r2, [r8, #0xc]
    ORREQ   r2, r2, #0x4000000
    STREQ   r2, [r8, #0xc]
    
    CMP     r1, #1
    LDREQ   r2, [r8, #0xc]
    ORREQ   r2, r2, #0x4000000
    STREQ   r2, [r8, #0xc]
#else

#ifndef CONFIG_WPCM450_WHOVILLE_X00
    LDR     r2, [r8, #0xc]
    ORR     r2, r2, #0x4000000
    STR     r2, [r8, #0xc]
#endif

#endif


/************************************************************************/
/* Set memory Controller to reset by                                    */
/* setting Memory Controller reset bit                                  */
/* fix problem with MC init in ROM                                      */
/************************************************************************/
/* SET_BIT(CLK_IPSRST,CLK_IPSRST_MC_BIT);                               */
/************************************************************************/
    LDR   r9, =CLK_BA
    LDR   r2, [r9, #0x20]
    ORR   r2, r2, #0x1000
    STR   r2, [r9, #0x20]


/************************************************************************/
/* Wait Loop                                                            */
/* idx2 used as register variable conuter into wait loop macro and      */
/* init value there.                                                    */
/************************************************************************/
/* WAIT_MC_RESET(idx2);                                                 */
/* #define WAIT_MC_RESET(cnt)  {for(cnt=0;cnt<10000;){cnt++;}}          */
/************************************************************************/
    MOV     r2, #0
    
wait_reset_mc_loop:
   
    CMP     r2, #0x5000
    ADD     r2, r2, #1
    BNE     wait_reset_mc_loop


/************************************************************************/
/* Release memory Controller from reset by                              */
/* clearing Memory Controller reset bit                                 */
/************************************************************************/
/* CLEAR_BIT(CLK_IPSRST,CLK_IPSRST_MC_BIT);                             */
/************************************************************************/

/************************************************************************/
/*        CLEAR_BIT(CLK_IPSRST,CLK_IPSRST_MC_BIT);                      */
/************************************************************************/
    LDR     r9, =CLK_BA
    LDR     r2, [r9, #0x20]
    BIC     r2, r2, #0x1000
    STR     r2, [r9, #0x20]


/************************************************************************/
/* Wait Loop                                                            */
/* idx2 used as register variable conuter into wait loop macro and      */
/* init value there.                                                    */
/************************************************************************/
/* WAIT_MC_RESET(idx2);                                                 */
/* #define WAIT_MC_RESET(cnt)  {for(cnt=0;cnt<10000;){cnt++;}}          */
/************************************************************************/

/************************************************************************/
/*        Wait Loop  500 micro sec                                      */
/************************************************************************/
    MOV     r2, #0
    
wait_clear_mc_loop:
    
    CMP     r2, #0x5000
    ADD     r2, r2, #1
    BNE     wait_clear_mc_loop


/************************************************************************/
/*  Save MC Config Table base Addr                                      */
/************************************************************************/
    ADD     r7, pc, #0
    B       mc_init_func


mc_128M_cfg:

         /* 180 MHz    133 MHz     250 MHz     160MHz      220 MHz     125MHz      200 MHz  */
.word    0x241B457C, 0x2312240E, 0x3624079E, 0x239AA4E0, 0x362406B4, 0x231223CE, 0x241B4618
.word    0x22162323, 0x21162223, 0x22162424, 0x22162323, 0x22162424, 0x21162223, 0x22162323
.word    0x40000643, 0x40000443, 0x40000843, 0x40000643, 0x40000843, 0x40000443, 0x40000643
/* set drive strength to 100%, might change again for A1 chip */
.word    0x4
.word    0xA,        0x7,        0xD,        0x8,        0xD,        0x7,        0xA
.word    0x00190202
.word    0x00000000
.word    0x00000001
.word    0x00000023
.word    0x00000005
.word    0x2A,       0x1E,       0x30,       0x24,       0x30,       0x1E,       0x2A
.word    0x00000001
.word    0x00000064
.word    0x00000000
.word    0x00000064
.word    0x00000000
.word    0x000001B8
.word    0x00000005
.word    0x0000000C
.word    0x00000000
.word    0x00000002
.word    0x00000002
.word    0x00000004
.word    0x00000004
.word    0x00010106
.word    0x00000106


mc_64M_cfg:

         /* 180 MHz     133 MHz     250 MHz     160MHz      220 MHz    125MHz      200 MHz  */
.word    0x241AA57C, 0x2311C40E, 0x3623679E, 0x239A24E0, 0x362366B4, 0x2311C3CE, 0x241AA618
.word    0x22122323, 0x21122223, 0x22122424, 0x22122323, 0x22122424, 0x21122223, 0x22122323
.word    0x40000643, 0x40000443, 0x40000843, 0x40000643, 0x40000843, 0x40000443, 0x40000643
.word    0x6
.word    0xA,        0x7,        0xD,        0x8,        0xD,        0x7,        0xA 
.word    0x550202
.word    0x0
.word    0x1
.word    0x23
.word    0x5
.word    0x2A,       0x1E,       0x30,       0x24,       0x30,       0x1E,       0x2A
.word    0x1
.word    0x64
.word    0x0
.word    0x64
.word    0x0
.word    0x1B8
.word    0x5
.word    0xC
.word    0x0
.word    0x2
.word    0x2
.word    0x4
.word    0x4
.word    0x00010106
.word    0x00000106


mc_32M_cfg:

         /*180 MHz     133 MHz     250 MHz     160MHz      220 MHz     125MHz      200 MHz   */
.word    0x249B857C, 0x2312A40E, 0x3624E79E, 0x241B24E0, 0x352466B4, 0x231283CE, 0x249BE618
.word    0x220E2324, 0x210E2224, 0x220E2424, 0x220E2324, 0x220E2424, 0x110E2224, 0x220E2324
.word    0x40000643, 0x40000443, 0x40000843, 0x40000643, 0x40000843, 0x40000443, 0x40000643
.word    0x6
.word    0x9,        0x7,        0xD,        0x8,        0xB,        0x7,        0xA
.word    0x310202
.word    0x0
.word    0x1
.word    0x23
.word    0x5
.word    0x30,       0x1E,       0x30,       0x24,       0x30,       0x1E,      0x30
.word    0x1
.word    0x64
.word    0x0
.word    0x64
.word    0x0
.word    0x1B8
.word    0x5
.word    0xC
.word    0x0
.word    0x2
.word    0x2
.word    0x4
.word    0x4
.word    0x00010106
.word    0x00000106


mc_init_func:          
/************************************************************************/
/*  idx2 is clock frequency index                                       */
/************************************************************************/
/* idx2 = GET_FIELD(power_cfg, PWRON_CPU_CORE_CLK);                     */
/*  idx2 -= 1;    Clock Bypass Mode not present in Config table         */
/************************************************************************/
    /* Get STRAP 9-11 value bits 8-10 */
    MOVS    r2, r0, LSL #21     /* LSL   r2,r0,#21 */
    MOVS    r2, r2, LSR #29     /* LSR   r2,r2,#29 */
    
    /* if idx2 equals clock bypass mode, set idx2 = CLK_250MHz_MODE(3) */
    BNE     clock_bypass_detect
    ADD     r2, r2, #3


clock_bypass_detect:
    SUB     r2, r2, #1
    
    /* if strap 3-4 = 0, set default ram size to 64Mb for whoville X00 */
    CMP     r1, #0
    
#ifdef CONFIG_WPCM450_WHOVILLE_X00
    MOVEQ   r1, #RAM_SIZE_64M
#else
    MOVEQ   r1, #RAM_SIZE_128M
#endif
    
    /* r7 is the start address of memory controller setting table */
    CMP     r1, #RAM_SIZE_128M
    MOVEQ   r3, r7
    BEQ     start_mc_init
    
    /* 0xe0 is the size of one setting table */
    CMP     r1, #RAM_SIZE_64M
    ADDEQ   r7, r7, #0xe0
    MOVEQ   r3, r7
    BEQ     start_mc_init
    
    CMP   r1,#RAM_SIZE_32M
    ADDEQ   r7, r7, #0x1c0
    MOVEQ   r3, r7
    BEQ     start_mc_init
    
    #MOV     r3, #mc_128M_cfg
    #MOV     r3, #mc_64M_cfg
    
start_mc_init:
    
    /* Read PDID, product identifier register */
    LDR     r8, =GCR_BA
    LDR     r0, [r8, #0x00]
    
    /* shift right 24 bits */
    MOVS    r0, r0, LSR #24
    
    /* 
    PDID product identifier register
    #define PDID_Z1     0x00926450
    #define PDID_Z2     0x03926450
    #define PDID_Z21    0x04926450
    #define PDID_A1     0x08926450
    #define PDID_A2     0x09926450
    #define PDID_A3     0x0A926450
    */
    
    LDR     r6, =MC_BA          /* r12 - base address MC Configuration registers */
    ADD     r5, r3, r2, LSL #2  /* idx2 *= 4; create clock index shift value */


/************************************************************************/
/*        MC_CFG0     = MC_Cfg[idx1].mc_cfg0[idx2];                     */
/************************************************************************/
    LDR      r4, [r5, #0]
    STR      r4, [r6, #0]

/************************************************************************/
/*        MC_CFG1     = MC_Cfg[idx1].mc_cfg1[idx2];
/************************************************************************/
    LDR      r4, [r5, #0x1c]
    STR      r4, [r6, #4]

/************************************************************************/
/*        MC_CFG2     = MC_Cfg[idx1].mc_cfg2[idx2];
/************************************************************************/
    LDR      r4, [r5, #0x38]
    STR      r4, [r6, #8]

/************************************************************************/
/*        MC_CFG3     = MC_Cfg[idx1].mc_cfg3;
/************************************************************************/
    LDR      r4, [r3, #0x54]
    STR      r4, [r6, #0xc]

/************************************************************************/
/*        MC_CFG4     = MC_Cfg[idx1].mc_cfg4[idx2];
/************************************************************************/
    LDR      r4, [r5, #0x58]
    STR      r4, [r6, #0x10]

/************************************************************************/
/*        MC_CFG5     = MC_Cfg[idx1].mc_cfg5;
/************************************************************************/
    LDR      r4, [r3, #0x74]
    STR      r4, [r6, #0x14]

/************************************************************************/
/*        MC_CFG6     = MC_Cfg[idx1].mc_cfg6;
/************************************************************************/
    LDR      r4, [r3, #0x78]
    STR      r4, [r6, #0x18]

/************************************************************************/
/*        MC_P1_ARBT  = MC_Cfg[idx1].mc_p1_arbt;
/************************************************************************/
    LDR      r4, [r3, #0x80]
    STR      r4, [r6, #0x24]

/************************************************************************/
/*        MC_P1_CNT   = MC_Cfg[idx1].mc_p1_cnt;
/************************************************************************/
    LDR      r4, [r3, #0x84]
    STR      r4, [r6, #0x20]

/************************************************************************/
/*        MC_P2_ARBT  = MC_Cfg[idx1].mc_p2_arbt;
/************************************************************************/
    LDR      r4, [r5, #0x88]
    STR      r4, [r6, #0x2c]

/************************************************************************/
/*        MC_P2_CNT   = MC_Cfg[idx1].mc_p2_cnt;
/************************************************************************/
    LDR      r4, [r3, #0xa4]
    
    /* adjust memory controller setting to prevent underruns for A3 or above */
    
    /* check chip version is A3 or above */
    CMP     r0, #0x0A
    BLO     STORE_MC_P2_ARBT
    
    /* overwrite setting */
    MOV     r4, #0x02
    
STORE_MC_P2_ARBT:
    STR      r4, [r6, #0x28]

/************************************************************************/
/*        MC_P3_ARBT  = MC_Cfg[idx1].mc_p3_arbt;
/************************************************************************/
    LDR      r4, [r3, #0xa8]
    STR      r4, [r6, #0x34]

/************************************************************************/
/*        MC_P3_CNT   = MC_Cfg[idx1].mc_p3_cnt;
/************************************************************************/
    LDR      r4, [r3, #0xac]
    STR      r4, [r6, #0x30]

/************************************************************************/
/*        MC_P4_ARBT  = MC_Cfg[idx1].mc_p4_arbt;
/************************************************************************/
    LDR      r4, [r3, #0xb0]
    STR      r4, [r6, #0x3c]

/************************************************************************/
/*        MC_P4_CNT   = MC_Cfg[idx1].mc_p4_cnt;
/************************************************************************/
    LDR      r4, [r3, #0xb4]
    STR      r4, [r6, #0x38]

/************************************************************************/
/*        MC_P5_ARBT  = MC_Cfg[idx1].mc_p5_arbt;
/************************************************************************/
    LDR      r4, [r3, #0xb8]
    STR      r4, [r6, #0x44]

/************************************************************************/
/*        MC_P5_CNT   = MC_Cfg[idx1].mc_p5_cnt;
/************************************************************************/
    LDR      r4, [r3, #0xbc]
    STR      r4, [r6, #0x40]

/************************************************************************/
/*        MC_P6_ARBT  = MC_Cfg[idx1].mc_p6_arbt;
/************************************************************************/
    LDR      r4, [r3, #0xc0]
    
    /* adjust memory controller setting to prevent underruns for A3 or above */
    
    /* check chip version is A3 or above */
    CMP     r0, #0x0A
    BLO     STORE_MC_P6_ARBT
    
    /* overwrite setting */
    MOV     r4, #0x08
    
STORE_MC_P6_ARBT:
    STR      r4, [r6, #0x4c]

/************************************************************************/
/*        MC_P6_CNT   = MC_Cfg[idx1].mc_p6_cnt;
/************************************************************************/
    LDR      r4, [r3, #0xc4]
    
    /* adjust memory controller setting to prevent underruns for A3 or above */
    
    /* check chip version is A3 or above */
    CMP     r0, #0x0A
    BLO     STORE_MC_P6_CNT
    
    /* overwrite setting */
    MOV     r4, #0x01
    
STORE_MC_P6_CNT:
    STR      r4, [r6, #0x48]

/************************************************************************/
/*        MC_P1_INCRS = MC_Cfg[idx1].mc_p1_incrs;
/************************************************************************/
    LDR      r4, [r3, #0xc8]
    STR      r4, [r6, #0x50]

/************************************************************************/
/*        MC_P2_INCRS = MC_Cfg[idx1].mc_p2_incrs;
/************************************************************************/
    LDR      r4, [r3, #0xcc]
    STR      r4, [r6, #0x54]

/************************************************************************/
/*        MC_P3_INCRS = MC_Cfg[idx1].mc_p3_incrs;
/************************************************************************/
    LDR      r4, [r3, #0xd0]
    STR      r4, [r6, #0x58]

/************************************************************************/
/*        MC_P4_INCRS = MC_Cfg[idx1].mc_p4_incrs;
/************************************************************************/
    LDR      r4, [r3, #0xd4]
    STR      r4, [r6, #0x5c]

/************************************************************************/
/*        MC_DLL_0    = MC_Cfg[idx1].mc_dll_0;
/************************************************************************/
    LDR      r4, [r3, #0xd8]
    
    /* set MC_DLL_0.DQSI_ENDLY, memory controller register offset 0x60 bit 10 \
       if chip version is A1 or above */
    
    /* check chip version is A1 or above */
    CMP     r0, #0x08
    BLO     STORE_MC_DLL_0
    
SET_MC_DLL_DQSI_ENDLY:
    
    /* set bit 10 */
    ORR     r4, r4, #0x400
    
STORE_MC_DLL_0:
    
    STR      r4, [r6, #0x60]


/************************************************************************/
/*        MC_DLL_1    = MC_Cfg[idx1].mc_dll_1;
/************************************************************************/
    LDR      r4, [r3, #0xdc]
    STR      r4, [r6, #0x64]


/*
;----------------------------------------------------------
; Set the high exception vector of CP15 control bit
;----------------------------------------------------------
*/
    MRC     p15, 0, r0, c1, c0, 0   /* read control register  */
    BIC     r0, r0, #0x2000         /* clear bit 13           */
    MCR     p15, 0, r0, c1, c0, 0   /* write control register */

skip_init:

/*
;----------------------------------------------------------
; --- back to arch calling code 
;----------------------------------------------------------
*/
    MOV pc, lr

/* the literal pools origin */
    .ltorg


/*----------------------------------------------------------
 *  WPCM450 platform definitions
 *----------------------------------------------------------
 */


/*
;----------------------------------------------------------
; AIC Control Registers
;----------------------------------------------------------
*/
.equ AIC_BA,            0xB8002000
.equ AIC_MDCR,          AIC_BA + 0x124  /* AIC Mask Disable Control Register */
.equ AIC_MASKALL,       0xFFFFFFFE

/*
;----------------------------------------------------------
;
;----------------------------------------------------------
*/
.equ GCR_BA,            0xB0000000
.equ GCR_INTCR2,        0xB0000060

/*
;----------------------------------------------------------
;
;----------------------------------------------------------
*/
.equ TMR_BA,            0xB8001000
.equ TMR_WTCR,          TMR_BA + 0x1C

/*
;----------------------------------------------------------
;
;----------------------------------------------------------
*/
.equ CLK_BA,            0xB0000200

/*
;----------------------------------------------------------
;
;----------------------------------------------------------
*/
.equ MC_BA,             0xB0001000
.equ RAM_SIZE_128M,     0x01
.equ RAM_SIZE_64M,      0x02
.equ RAM_SIZE_32M,      0x03

/************************************************************************/
/* CLOCK DEVIDERS REGISTER CONFIGURATIONS                               */
/************************************************************************/
.equ CLK_125MHZ_DIV_REG_CFG,     0x04010300
.equ CLK_133MHZ_DIV_REG_CFG,     0x05010200
.equ CLK_160MHZ_DIV_REG_CFG,     0x05010200
.equ CLK_180MHZ_DIV_REG_CFG,     0x05010300
.equ CLK_200MHZ_DIV_REG_CFG,     0x05010300
/* change AHB3 clock to 27.5 MHz */
/* .equ CLK_220MHZ_DIV_REG_CFG,     0x05010200 */
.equ CLK_220MHZ_DIV_REG_CFG,     0x05010300
.equ CLK_250MHZ_DIV_REG_CFG,     0x05010300
.equ CLK_BYPASS_DIV_REG_CFG,     0x05010300

/************************************************************************/
/* CLOCK PLL0 REGISTER CONFIGURATIONS                                   */
/************************************************************************/
.equ CLK_125MHZ_PLLCON_REG_CFG,  0x007C210B
.equ CLK_133MHZ_PLLCON_REG_CFG,  0x00632108
.equ CLK_160MHZ_PLLCON_REG_CFG,  0x00272005
.equ CLK_180MHZ_PLLCON_REG_CFG,  0x002C2005
.equ CLK_200MHZ_PLLCON_REG_CFG,  0x00312005
/* slow down clcok from 220 to 216 MHz to workaround for the Winbond reset 
   errata, this only applies for A1 and earlier silicon */
.equ CLK_220MHZ_PLLCON_REG_CFG,  0x00362005
.equ CLK_250MHZ_PLLCON_REG_CFG,  0x007C200B

/************************************************************************/
/* Clock Source selection                                               */
/************************************************************************/
.equ CLK_PLL0_SRC_SEL,           0x00
.equ CLK_PLL1_SRC_SEL,           0x01
.equ CLK_48M_SRC_SEL,            0x02
.equ CLK_PWM3_GPIO83_SRC_SEL,    0x03

/************************************************************************/
/* Pixel Clock Source Select Bits                                       */
/************************************************************************/
.equ CLK_PIX_PLL_GFX_SEL,        0x00
.equ CLK_PIX_GPI96_SEL,          0x01
.equ CLK_PIX_48M_SEL,            0x02
