#ifndef _TIMER_REGS_WPCS410_H
#define _TIMER_REGS_WPCS410_H
//------------------------------------------------------------------------------------------

/************************************************************************/
/* WPCS410 registers definition                                         */
/************************************************************************/


/************************************************************************/
/*  TIMER Register                                                      */
/************************************************************************/

#define TMR_BA  0xB8001000
#define TCSR0 VPlong(TMR_BA+0x000) /* R/W Timer Control and Status Register 0 0000_0005h */
#define TCSR1 VPlong(TMR_BA+0x004) /* R/W Timer Control and Status Register 1 0000_0005h */
#define TICR0 VPlong(TMR_BA+0x008) /* R/W Timer Initial Control Register 0 0000_0000h */
#define TICR1 VPlong(TMR_BA+0x00C) /* R/W Timer Initial Control Register 1 0000_0000h */
#define TDR0  VPlong(TMR_BA+0x010) /* RO Timer Data Register 0 0000_0000h */
#define TDR1  VPlong(TMR_BA+0x014) /* RO Timer Data Register 1 0000_0000h */
#define TISR  VPlong(TMR_BA+0x018) /* R/W1C Timer Interrupt Status Register 0000_0000h */
#define WTCR  VPlong(TMR_BA+0x01C) /* R/W Watchdog Timer Control Register 0000_0400h */
#define TCSR2 VPlong(TMR_BA+0x020) /* R/W Timer Control and Status Register 2 0000_0005h */
#define TCSR3 VPlong(TMR_BA+0x024) /* R/W Timer Control and Status Register 3 0000_0005h */
#define TICR2 VPlong(TMR_BA+0x028) /* R/W Timer Initial Control Register 2 0000_0000h */
#define TICR3 VPlong(TMR_BA+0x02C) /* R/W Timer Initial Control Register 3 0000_0000h */
#define TDR2  VPlong(TMR_BA+0x030) /* RO Timer Data Register 2 0000_0000h */
#define TDR3  VPlong(TMR_BA+0x034) /* RO Timer Data Register 3 0000_0000h */
#define TCSR4 VPlong(TMR_BA+0x040) /* R/W Timer Control and Status Register 4 0000_0005h */
#define TICR4 VPlong(TMR_BA+0x048) /* R/W Timer Initial Control Register 4 0000_0000h */
#define TDR4  VPlong(TMR_BA+0x050) /* RO Timer Data Register 4 0000_0000h */

#define TIF0	0x1
#define TIF1	0x2

/* Watchdog Timer Control Register (WTCR) fields */
#define WTCR_WTE_BIT            7
#define WTCR_RESET_FLAG_BIT     2
#define WTCR_WTRE_BIT           1
#define WTCR_WTR_BIT            0



#endif /* _TIMER_REGS_WPCS410_H */
