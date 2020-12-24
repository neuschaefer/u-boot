/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 register definitions.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


#ifndef _GCTRL_REGS_WPCM450_H
#define _GCTRL_REGS_WPCM450_H

/************************************************************************/
/* Global Control Register                                              */
/************************************************************************/

#define GCR_BA      0xB0000000
#define PDID        VPlong(GCR_BA+0x000)
#define PWRON       VPlong(GCR_BA+0x004)
#define MFSEL1      VPlong(GCR_BA+0x00C)
#define MFSEL2      VPlong(GCR_BA+0x010)
#define MISCPE      VPlong(GCR_BA+0x014)
#define GPIOP1PE    VPlong(GCR_BA+0x01C)
#define GPIOP2PE    VPlong(GCR_BA+0x020)
#define GPIOP3PE    VPlong(GCR_BA+0x024)
#define GPIOP5PE    VPlong(GCR_BA+0x02C)
#define GPIOP6PE    VPlong(GCR_BA+0x030)
#define GPIOP7PE    VPlong(GCR_BA+0x034)
#define SPSWC       VPlong(GCR_BA+0x038)
#define INTCR       VPlong(GCR_BA+0x03C)
#define XBCR        VPlong(GCR_BA+0x04C)
#define HIFCR       VPlong(GCR_BA+0x050)
#define INTCR2      VPlong(GCR_BA+0x060)
#define ETSR        VPlong(GCR_BA+0x108)


/* PDID - product identifier register fields */
#define PDID_Z1     0x00926450
#define PDID_Z2     0x03926450
#define PDID_Z21    0x04926450
#define PDID_A1     0x08926450
#define PDID_A2     0x09926450
#define PDID_A3     0x0A926450

#define PDID_Z1_CHIP_VER    0x0
#define PDID_Z2_CHIP_VER    0x3
#define PDID_Z21_CHIP_VER   0x4
#define PDID_A1_CHIP_VER    0x8
#define PDID_A2_CHIP_VER    0x9
#define PDID_A3_CHIP_VER    0xA

#define PDID_CHRID          0x926450


/* PWON - power-on setting register fields */
#define PWRON_VGA_BIOS_BIT    12
#define PWRON_FPROG_EN_BIT    11
#define PWRON_CPU_CORE_CLK_S   3
#define PWRON_CPU_CORE_CLK_P   8
#define PWRON_BIOSEN_BIT       7
#define PWRON_RAM_SIZE_S       2
#define PWRON_RAM_SIZE_P       2
#define PWRON_BIOS_FL_SIZE_S   2
#define PWRON_BIOS_FL_SIZE_P   0


/* MFSEL1 - multiple function pin select register 1 fields */
#define MF_SSPISEL      31
#define MF_SDIOSEL      30

#define MF_XCS1SEL      29
#define MF_XCS2SEL      28

#define MF_MBEN_BIT     26

#define MF_GSPISEL_BIT  24
#define MF_UINCSEL_BIT  23
#define MF_SMISEL_BIT   22
#define MF_CLKOSEL_BIT  21
#define MF_DVOSEL_S      3
#define MF_DVOSEL_P     18
#define MF_KBCCSEL_BIT  17
#define MF_R2MDSEL_BIT  16
#define MF_R2ERRSEL_BIT 15
#define MF_RMII2SEL_BIT 14
#define MF_R1MDSEL_BIT  13
#define MF_R1ERRSEL_BIT 12
#define MF_HSP2SEL_BIT  11
#define MF_HSP1SEL_BIT  10
#define MF_BSPSEL_BIT    9
#define MF_SMB2SEL_BIT   8
#define MF_SMB1SEL_BIT   7
#define MF_SMB0SEL_BIT   6
#define MF_SCS3SEL_BIT   5
#define MF_SCS2SEL_BIT   4
#define MF_SCS1SEL_BIT   3
#define MF_SMB5SEL_BIT   2
#define MF_SMB4SEL_BIT   1
#define MF_SMB3SEL_BIT   0


/* MFSEL2 - multiple function pin select register 2 fields */
#define MF_HG7SEL_BIT   31
#define MF_HG6SEL_BIT   30
#define MF_HG5SEL_BIT   29
#define MF_HG4SEL_BIT   28
#define MF_HG3SEL_BIT   27
#define MF_HG2SEL_BIT   26
#define MF_HG1SEL_BIT   25
#define MF_HG0SEL_BIT   24
#define MF_PWM7SEL_BIT  23
#define MF_PWM6SEL_BIT  22
#define MF_PWM5SEL_BIT  21
#define MF_PWM4SEL_BIT  20
#define MF_PWM3SEL_BIT  19
#define MF_PWM2SEL_BIT  18
#define MF_PWM1SEL_BIT  17
#define MF_PWM0SEL_BIT  16
#define MF_FI15SEL_BIT  15
#define MF_FI14SEL_BIT  14
#define MF_FI13SEL_BIT  13
#define MF_FI12SEL_BIT  12
#define MF_FI11SEL_BIT  11
#define MF_FI10SEL_BIT  10
#define MF_FI9SEL_BIT   9
#define MF_FI8SEL_BIT   8
#define MF_FI7SEL_BIT   7
#define MF_FI6SEL_BIT   6
#define MF_FI5SEL_BIT   5
#define MF_FI4SEL_BIT   4
#define MF_FI3SEL_BIT   3
#define MF_FI2SEL_BIT   2
#define MF_FI1SEL_BIT   1
#define MF_FI0SEL_BIT   0


/* MISCPE - miscellaneous pin pull-up/down enable register fields */
#define MISCPE_R1CRSDV_R1RXD1_0     15
#define MISCPE_NSPICS               14
#define MISCPE_SPIDI                13
#define MISCPE_NXWR                 10
#define MISCPE_NXRD                 9
#define MISCPE_NXCS0                8
#define MISCPE_XD7                  7
#define MISCPE_XD6                  6
#define MISCPE_XD5                  5
#define MISCPE_XD4                  4
#define MISCPE_XD3                  3
#define MISCPE_XD2                  2
#define MISCPE_XD1                  1
#define MISCPE_XD0                  0


/* GPIOP1PE - GPIO port 1 pin pull-up/down enable register fields */
/* GPIOP2PE - GPIO port 2 pin pull-up/down enable register fields */
/* GPIOP3PE - GPIO port 3 pin pull-up/down enable register fields */
/* GPIOP5PE - GPIO port 5 pin pull-up/down enable register fields */
/* GPIOP6PE - GPIO port 6 pin pull-up/down enable register fields */
/* GPIOP7PE - GPIO port 7 pin pull-up/down enable register fields */
/* same as GPIO definitions */


/* SPSWC - serial port switch control register */
#define SPSWC_SPMOD_S           2
#define SPSWC_SPMOD_P           0

#define SERIAL_MODE1_SNOOP      0
#define SERIAL_MODE2_TAKEOVER   1
#define SERIAL_MODE3_DIRECT     2
#define SERIAL_MODE_RESERVED    3


/* INTCR registers fields */
#define INTCR_VGAIOEN_BIT   6


/* INTCR2 registers fields */
#define INTCR2_SDSD_BIT     0


/* ESTR register fields */
#define ESTR_P1_BIT         0


/************************************************************************/
/* Shared Memory Core Control Register (SMC_CTL)                        */
/************************************************************************/
#define SMC_CTL             VPchar(0xC8001001)


#define SMC_CTL_HOSTWAIT    7


/************************************************************************/
/* General Purpose Input/Output Register                                */
/************************************************************************/
#define GPIO_BA     0xB8003000
#define GPEVTYPE    VPlong(GPIO_BA+0x000)
#define GPEVPOL     VPlong(GPIO_BA+0x004)
#define GPEVDBNC    VPlong(GPIO_BA+0x008)
#define GPEVEN      VPlong(GPIO_BA+0x00C)
#define GPEVST      VPlong(GPIO_BA+0x010)
#define GP0CFG0     VPlong(GPIO_BA+0x014) /* tri-state or output buffer */
#define GP0CFG1     VPlong(GPIO_BA+0x018) /* push-pull or open-drain */
#define GP0DOUT     VPlong(GPIO_BA+0x01C) /* data output */
#define GP0DIN      VPlong(GPIO_BA+0x020) /* data input */
#define GP1CFG0     VPlong(GPIO_BA+0x024)
#define GP1CFG1     VPlong(GPIO_BA+0x028)
#define GP1CFG2     VPlong(GPIO_BA+0x02C) /* power by VSB or VDD */
#define GP1DOUT     VPlong(GPIO_BA+0x034)
#define GP1DIN      VPlong(GPIO_BA+0x038)
#define GP2CFG0     VPlong(GPIO_BA+0x03C)
#define GP2CFG1     VPlong(GPIO_BA+0x040)
#define GP2CFG2     VPlong(GPIO_BA+0x044)
#define GP2DOUT     VPlong(GPIO_BA+0x048)
#define GP2DIN      VPlong(GPIO_BA+0x04C)
#define GP3CFG0     VPlong(GPIO_BA+0x050)
#define GP3CFG1     VPlong(GPIO_BA+0x054)
#define GP3CFG2     VPlong(GPIO_BA+0x058)
#define GP3DOUT     VPlong(GPIO_BA+0x05C)
#define GP3DIN      VPlong(GPIO_BA+0x060)
#define GP4CFG0     VPlong(GPIO_BA+0x064)
#define GP4CFG1     VPlong(GPIO_BA+0x068)
#define GP4CFG2     VPlong(GPIO_BA+0x06C)
#define GP4DOUT     VPlong(GPIO_BA+0x070)
#define GP4DIN      VPlong(GPIO_BA+0x074)
#define GP5CFG0     VPlong(GPIO_BA+0x078)
#define GP5CFG1     VPlong(GPIO_BA+0x07C)
#define GP5CFG2     VPlong(GPIO_BA+0x080)
#define GP5DOUT     VPlong(GPIO_BA+0x084)
#define GP5DIN      VPlong(GPIO_BA+0x088)
#define GP6DIN      VPlong(GPIO_BA+0x08C)
#define GP7CFG0     VPlong(GPIO_BA+0x090)
#define GP7CFG1     VPlong(GPIO_BA+0x094)
#define GP7CFG2     VPlong(GPIO_BA+0x098)
#define GP7DOUT     VPlong(GPIO_BA+0x09C)
#define GP7DIN      VPlong(GPIO_BA+0x0A0)

/* Configuration Register 0 */
#define GPIO_TRI_STATE      0
#define GPIO_OUTPUT_MODE    1

/* Configuration Register 1 */
#define GPIO_PUSH_PULL      0
#define GPIO_OPEN_DRAIN     1

/* Configuration Register 2 */
#define GPIO_POWER_BY_VSB   0
#define GPIO_POWER_BY_VDD   1

/* Data Output Register */
#define GPIO_DRIVE_LOW      0
#define GPIO_DRIVE_HIGH     1

/* Data Input Register */
#define GPIO_LEVEL_LOW      0
#define GPIO_LEVEL_HIGH     1

/* GPIO Port 0 */
#define GPIOE0      0
#define GPIOE1      1
#define GPIOE2      2
#define GPIOE3      3
#define GPIOE4      4
#define GPIOE5      5
#define GPIOE6      6
#define GPIOE7      7
#define GPIOE8      8
#define GPIOE9      9
#define GPIOE10     10
#define GPIOE11     11
#define GPIOE12     12
#define GPIOE13     13
#define GPIOE14     14
#define GPIOE15     15

/* GPIO Port 1 */
#define GPIO16      0
#define GPIO17      1
#define GPIO18      2
#define GPIO19      3
#define GPIO20      4
#define GPIO21      5
#define GPIO22      6
#define GPIO23      7
#define GPIO24      8
#define GPIO25      9
#define GPIO26      10
#define GPIO27      11
#define GPIO28      12
#define GPIO29      13
#define GPIO30      14
#define GPIO31      15

/* GPIO Port 2 */
#define GPIO32      0 
#define GPIO33      1 
#define GPIO34      2 
#define GPIO35      3 
#define GPIO36      4 
#define GPIO37      5 
#define GPIO38      6 
#define GPIO39      7 
#define GPIO40      8 
#define GPIO41      9 
#define GPIO42      10
#define GPIO43      11
#define GPIO44      12
#define GPIO45      13
#define GPIO46      14
#define GPIO47      15

/* GPIO Port 3 */
#define GPIO48      0 
#define GPIO49      1 
#define GPIO50      2 
#define GPIO51      3 
#define GPIO52      4 
#define GPIO53      5 
#define GPIO54      6 
#define GPIO55      7 
#define GPIO56      8 
#define GPIO57      9 
#define GPIO58      10
#define GPIO59      11
#define GPIO60      12
#define GPIO61      13
#define GPIO62      14
#define GPIO63      15

/* GPIO Port 4 */
#define GPIO64      0 
#define GPIO65      1 
#define GPIO66      2 
#define GPIO67      3 
#define GPIO68      4 
#define GPIO69      5 
#define GPIO70      6 
#define GPIO71      7 
#define GPIO72      8 
#define GPIO73      9 
#define GPIO74      10
#define GPIO75      11
#define GPIO76      12
#define GPIO77      13
#define GPIO78      14
#define GPIO79      15

/* GPIO Port 5 */
#define GPIO80      0 
#define GPIO81      1 
#define GPIO82      2 
#define GPIO83      3 
#define GPIO84      4 
#define GPIO85      5 
#define GPIO86      6 
#define GPIO87      7 
#define GPIO88      8 
#define GPIO89      9 
#define GPIO90      10
#define GPIO91      11
#define GPIO92      12
#define GPIO93      13
#define GPIO94      14

/* GPIO Port 6 */
#define GPI96       0 
#define GPI97       1 
#define GPI98       2 
#define GPI99       3 
#define GPI100      4 
#define GPI101      5 
#define GPI102      6 
#define GPI103      7 
#define GPI104      8 
#define GPI105      9 
#define GPI106      10
#define GPI107      11
#define GPI108      12
#define GPI109      13
#define GPI110      14
#define GPI111      15
#define GPI112      16
#define GPI113      17

/* GPIO Port 7 */
#define GPIO114     0 
#define GPIO115     1 
#define GPIO116     2 
#define GPIO117     3 
#define GPIO118     4 
#define GPIO119     5 
#define GPIO120     6 
#define GPIO121     7 
#define GPIO122     8 
#define GPIO123     9 
#define GPIO124     10
#define GPIO125     11
#define GPIO126     12
#define GPIO127     13


/************************************************************************/
/* Watchdog Timer Control Register                                      */
/************************************************************************/
#define WTCR        VPlong(0xB800101C)


/************************************************************************/
/* Pulse Width Modulation Register                                      */
/************************************************************************/
#define PWMM0_BA            0xB8007000
#define PWMM1_BA            0xB8007100
#define PPP(module)         VPlong(PWMM0_BA + (0x100 * module) + 0x000)
#define CSR(module)         VPlong(PWMM0_BA + (0x100 * module) + 0x004)
#define PCR(module)         VPlong(PWMM0_BA + (0x100 * module) + 0x008)
#define CNR(module, port)   VPlong(PWMM0_BA + (0x100 * module) + 0x00C + (0x0C * port))
#define CMR(module, port)   VPlong(PWMM0_BA + (0x100 * module) + 0x010 + (0x0C * port))
#define PDR(module, port)   VPlong(PWMM0_BA + (0x100 * module) + 0x014 + (0x0C * port))
#define PIER(module)        VPlong(PWMM0_BA + (0x100 * module) + 0x03C)
#define PIIR(module)        VPlong(PWMM0_BA + (0x100 * module) + 0x040)

#endif /* _GCTRL_REGS_WPCM450_H */
