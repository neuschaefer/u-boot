#ifndef _WPCM450_AIC_REGS_H
#define _WPCM450_AIC_REGS_H

/************************************************************************/
/* WPCS410 registers definition                                         */
/************************************************************************/

/************************************************************************/
/*    Interrupt Controller Group Registers                              */
/************************************************************************/

#define AIC_BASE	0xB8002000
#define AIC_SCR0	VPlong(AIC_BASE)
#define AIC_SCR1	VPlong(AIC_BASE+0x04)
#define AIC_SCR2	VPlong(AIC_BASE+0x08)
#define AIC_SCR3	VPlong(AIC_BASE+0x0C)
#define AIC_SCR4	VPlong(AIC_BASE+0x10)
#define AIC_SCR5	VPlong(AIC_BASE+0x14)
#define AIC_SCR6	VPlong(AIC_BASE+0x18)
#define AIC_SCR7	VPlong(AIC_BASE+0x1C)
#define AIC_SCR8	VPlong(AIC_BASE+0x20)
#define AIC_SCR9	VPlong(AIC_BASE+0x24)
#define AIC_SCR10	VPlong(AIC_BASE+0x28)
#define AIC_SCR11	VPlong(AIC_BASE+0x2C)
#define AIC_SCR12	VPlong(AIC_BASE+0x30)
#define AIC_SCR13	VPlong(AIC_BASE+0x34)
#define AIC_SCR14	VPlong(AIC_BASE+0x38)
#define AIC_SCR15	VPlong(AIC_BASE+0x3C)
#define AIC_SCR16	VPlong(AIC_BASE+0x40)
#define AIC_SCR17	VPlong(AIC_BASE+0x44)
#define AIC_SCR18	VPlong(AIC_BASE+0x48)

#define AIC_IRSR	VPlong(AIC_BASE+0x100)
#define AIC_IASR	VPlong(AIC_BASE+0x104)
#define AIC_ISR		VPlong(AIC_BASE+0x108)
#define AIC_IPER	VPlong(AIC_BASE+0x10C)
#define AIC_ISNR	VPlong(AIC_BASE+0x110)
#define AIC_IMR		VPlong(AIC_BASE+0x114)
#define AIC_OISR	VPlong(AIC_BASE+0x118)
#define AIC_MECR	VPlong(AIC_BASE+0x120)
#define AIC_MDCR	VPlong(AIC_BASE+0x124)
#define AIC_SSCR	VPlong(AIC_BASE+0x128)
#define AIC_SCCR	VPlong(AIC_BASE+0x12C)
#define AIC_EOSCR	VPlong(AIC_BASE+0x130)

typedef enum {
   WDT_INT      =1,  /* Watch Dog Timer Interrupt */
   GPIO_INT0    =2,  /* GPIO Interrupt Group 0 containing GPIOE3-0 */
   GPIO_INT1    =3,  /* GPIO Interrupt Group 1 containing GPIOE11  */
   GPIO_INT2    =4,  /* GPIO Interrupt Group 2 containing GPIOE15  */
   GPIO_INT3    =5,  /* GPIO Interrupt Group 3 containing GPIO25-24 */
   PECI_INT     =6,  /* PECI Interrupt */
   UART0_INT    =7,  /* UART0 Interrupt */
   UART1_INT    =8,  /* UART1 Interrupt  */
   KCS_HIB_INT  =9,  /* KCS/HIB Interrupt (from host interface) */
   FIU_SPI_INT  =10, /* FIU_SPI interrupt */
   SHM_INT      =11, /* SHM Interrupt */
   T_INT0       =12, /* Timer Interrupt 0 */
   T_INT1       =13, /* Timer Interrupt 1 */
   T_INT_GRP    =14, /* Timer Interrupt Group containing Timer2, Timer3, Timer4 */
   EMC0_RX_INT  =15, /* EMC1 Rx Interrupt */
   EMC0_TX_INT  =16, /* EMC1 Tx Interrupt */
   EMC1_RX_INT  =17, /* EMC2 Rx Interrupt */
   EMC1_TX_INT  =18, /* EMC2 Tx Interrupt */
   GDMA_GRP_INT =19, /* Reserved GDMA Interrupt Group containing GDMA0, GDMA1 */
   USBD2_INT    =20, /* USB Device2 Interrupt */
   USBD1_INT    =21, /* USB Device1 Interrupt */
   VCD_INT      =22, /* VCD interrupt */
   SMB3_INT     =23, /* SMBus3 Interrupt */
   MFT0_INT     =24, /* Tachometer Timer 0 (MFT0) Interrupt */
   MFT1_INT     =25, /* Tachometer Timer 1 (MFT1) Interrupt */
   SMB_INT_GRP  =26, /* SMBus Interrupt Group containing SMBus0, SMBus1, SMBus2 */
   SMB4_INT     =27, /* SMBus4 Interrupt */
   PWM_INT      =28, /* PWM Timer Interrupt Group containing PWM0, PWM1, PWM2, PWM3 */
   SMB5_INT     =29, /* SMBus5 Interrupt */
   MPG_INT      =30, /* Main power good indication from PWRGD_PS input pin. */
   ADC_INT      =31, /* ADC Interrupt */
   MAX_INT_NUM       /* Max Interrupt number. Have been last */
}wpcm450_irq_list_t;

#endif /* _WPCM450__AIC_REGS_H */
