
#ifndef _CLK_REGS_WPCS410_H
#define _CLK_REGS_WPCS410_H

/************************************************************************/
/* WPCS410 registers definition                                         */
/************************************************************************/

#define CLK_BA            0xB0000200
#define CLK_EN            VPlong(CLK_BA+0x0)     /* Clock Enable Register (defaul value = 0xFFFFFFFF)*/
#define CLK_SEL           VPlong(CLK_BA+0x4)     /* Clock Select Register (defaul value = 0x00000202)*/
#define CLK_DIV           VPlong(CLK_BA+0x8)     /* Clock Divider Control Register (defaul value = 0x00000300)*/
#define CLK_PLLCON0       VPlong(CLK_BA+0xC)     /* PLL Control Register 0 (defaul value = 0x00001323)*/
#define CLK_PLLCON1       VPlong(CLK_BA+0x10)    /* PLL Control Register 1 (defaul value = 0x00001323)*/
#define CLK_PMCON         VPlong(CLK_BA+0x14)    /* Power Management Control Register (defaul value = 0x00000000)*/
#define CLK_IRQWAKECON    VPlong(CLK_BA+0x18)    /* IRQ Wake-Up Control Register (defaul value = 0x00000000)*/
#define CLK_IRQWAKEFLAG   VPlong(CLK_BA+0x1C)    /* IRQ Wake-Up Flag Register (defaul value = 0x00000000)*/
#define CLK_IPSRST        VPlong(CLK_BA+0x20)    /* IP Software Reset Register (defaul value = 0x00000000) */


/* CLK_EN */
#define CLK_SMB1_BIT    31
#define CLK_SMB0_BIT    30
#define CLK_SSPI_BIT    29
#define CLK_SDIO_BIT    28
#define CLK_ADC_BIT     27
#define CLK_WDT_BIT     26
#define CLK_MFT1_BIT    25
#define CLK_MFT0_BIT    24
#define CLK_TIMER4_BIT  23
#define CLK_TIMER3_BIT  22
#define CLK_TIMER2_BIT  21
#define CLK_TIMER1_BIT  20
#define CLK_TIMER0_BIT  19
#define CLK_PWM_BIT     18
#define CLK_HUART_BIT   17
#define CLK_SMB5_BIT    16
#define CLK_SMB4_BIT    15
#define CLK_SMB3_BIT    14
#define CLK_SMB2_BIT    13
#define CLK_UART1_BIT   12
#define CLK_UART0_BIT   11
#define CLK_AES_BIT     10
#define CLK_USB1_BIT     9
#define CLK_USB0_BIT     8
#define CLK_EMC1_BIT     7
#define CLK_EMC0_BIT     6
#define CLK_RNG_BIT      5
#define CLK_SHM_BIT      4
#define CLK_GDMA_BIT     3
#define CLK_KCS_BIT      2
#define CLK_XBUS_BIT     1
#define CLK_FIU_BIT      0

/* CLK_SEL register bits */
#define CLK_HUART_SRC_BIT   10
#define CLK_UART_SRC_P       8
#define CLK_UART_SRC_S       2
#define CLK_USB1CKSEL_P      6
#define CLK_USB1CKSEL_S      2
#define CLK_PIX_SRC_P        4
#define CLK_PIX_SRC_S        2
#define CLK_CPU_OUT_P        0
#define CLK_CPU_OUT_S        2
#define CLK_CPU_CLK_P        0
#define CLK_CPU_CLK_S        2


#define CLK_ADCCKDIV_P      28
#define CLK_ADCCKDIV_S       2
#define CLK_APBCKDIV_P      26
#define CLK_APBCKDIV_S       2
#define CLK_AHBCKDIV_P      24
#define CLK_AHBCKDIV_S       2
#define CLK_UARTDIV_P       16
#define CLK_UARTDIV_S        4
#define CLK_AHB3CKDIV_P      8
#define CLK_AHB3CKDIV_S      2
#define CLK_MCCKDIV_P        0
#define CLK_MCCKDIV_S        3


/* CLK_PLLCON0 CLK_PLLCON1 fields */
#define CLK_FBDV_S           9
#define CLK_FBDV_P           16    
#define CLK_PRST_BIT         13
#define CLK_PWDEN_BIT        12
#define CLK_OTDV_S           3
#define CLK_OTDV_P           8 
#define CLK_INDV_S           6
#define CLK_INDV_P           0



#define CLK_RESET_CTRL_BIT    3
#define CLK_MIDLE_ENABLE_BIT  2
#define CLK_PDOWN_ENABLE_BIT  1
#define CLK_IDLE_ENABLE_BIT   0

#define CLK_IRQWAKEUPPOL_P   16
#define CLK_IRQWAKEUPPOL_S   16
#define CLK_IRQWAKEUPEN_P     0
#define CLK_IRQWAKEUPEN_S    16
#define CLK_IRQWAKEFLAG_P     0
#define CLK_IRQWAKEFLAG_S    16

/* CLK_IPSRST fields */
#define CLK_IPSRST_SMB1_BIT    31
#define CLK_IPSRST_SMB0_BIT    30
#define CLK_IPSRST_SSPI_BIT    29
#define CLK_IPSRST_SDIO_BIT    28
#define CLK_IPSRST_ADC_BIT     27
#define CLK_IPSRST_TIMER_BIT   19
#define CLK_IPSRST_PWM_BIT     18
#define CLK_IPSRST_SMB5_BIT    16
#define CLK_IPSRST_SMB4_BIT    15
#define CLK_IPSRST_SMB3_BIT    14
#define CLK_IPSRST_SMB2_BIT    13
#define CLK_IPSRST_MC_BIT      12
#define CLK_IPSRST_UART_BIT    11
#define CLK_IPSRST_AES_BIT     10
#define CLK_IPSRST_PECI_BIT     9
#define CLK_IPSRST_USB2_BIT     8
#define CLK_IPSRST_EMC1_BIT     7
#define CLK_IPSRST_EMC0_BIT     6
#define CLK_IPSRST_USB1_BIT     5
#define CLK_IPSRST_GDMA_BIT     3


/************************************************************************/
/*                                                                      */
/************************************************************************/

#define RMII_RCKREF_CLK_RATE 48000000

#endif /* _CLK_REGS_WPCS410_H */
