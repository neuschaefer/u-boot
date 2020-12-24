/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 serial driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


#ifndef _UART_REGS_WPCM450_H
#define _UART_REGS_WPCM450_H


/************************************************************************/
/* WPCM450 registers definition                                         */
/************************************************************************/


/************************************************************************/
/* UART registers                                                       */
/************************************************************************/
#define UART0_BA                0xB8000000
#define UART1_BA                0xB8000100
#define UART_RBR(port)          VPchar(UART0_BA + (port * 0x100) + 0x0000)
#define UART_THR(port)          VPchar(UART0_BA + (port * 0x100) + 0x0000)
#define UART_IER(port)          VPchar(UART0_BA + (port * 0x100) + 0x0004)

/* Modem status interrupt enable (Irpt_MOS) */
#define MSIE                    (0x1 << 3)

/* Receive line status interrupt enable (Irpt_RLS) */
#define RLSIE                   (0x1 << 2)

/* Transmit holding register empty interrupt enable (Irpt_THRE) */
#define THREIE                  (0x1 << 1)

/* Receive data available interrupt enable (Irpt_RDA) */
#define RDAIE                   0x1 
#define rUART_IER               (MSIE + RLSIE + THREIE + RDAIE)
#define UART_DLL(port)          VPchar(UART0_BA + (port * 0x100) + 0x0000)
#define UART_DLM(port)          VPchar(UART0_BA + (port * 0x100) + 0x0004)
#define UART_IIR(port)          VPchar(UART0_BA + (port * 0x100) + 0x0008)


/* UART FCR */
#define UART_FCR(port)          VPchar(UART0_BA + (port * 0x100) + 0x0008)

/* RX FIFO Interrupt trigger level */
#define RFITL                   (0x2 << 6)

/* TX FIFO Reset, 00 = 1 bytes, 01 = 4 bytes, 10 = 8 bytes, 11 = 14 bytes */
#define TFR                     (0x1 << 2)

/* RX FIFO Reset, 0 = no effect, 1 = reset  */
#define RFR                     (0x1 << 1)

/* FIFO mode enable, 0 = no effect, 1 = reset  */
#define FME                     0x1

/* 0 = can't write command, 1 = write command to FCR */
#define rUART_FCR               (RFITL + TFR + RFR + FME)


/* UART_LCR */
#define UART_LCR(port)          VPchar(UART0_BA + (port * 0x100) + 0x000C)
#define DLAB                    (0x1 <<  7)
#define BCB                     (0x1 <<  6)
#define SPE                     (0x1 <<  5)
#define EPE                     (0x1 <<  4)
#define PBE                     (0x1 <<  3)
#define NSB                     (0x1 <<  2)
#define UART_5bit               0x0
#define UART_6bit               0x1
#define UART_7bit               0x2
#define UART_8bit               0x3

#define UART_MSR(port)          VPchar(UART0_BA + (port * 0x100) + 0x0018)
#define UART_TOR(port)          VPchar(UART0_BA + (port * 0x100) + 0x001C)
#define TOIE                    0x80
#define RX_FIFO_LEVEL_1         (0x00)
#define RX_FIFO_LEVEL_4         (0x40)
#define RX_FIFO_LEVEL_8         (0x80)
#define RX_FIFO_LEVEL_14        (0xC0)

#define UART_MCR(port)          VPchar(UART0_BA + (port * 0x100) + 0x0010)
#define UART_LSR(port)          VPchar(UART0_BA + (port * 0x100) + 0x0014)
#define ERR_RX                  (1 << 7)
#define TRANS_EMPTY             (1 << 6)
#define TRANS_HOLD_REG_EMPTY    (1 << 5)
#define BREAK_INT               (1 << 4)
#define FRAME_ERR               (1 << 3)
#define PARITY_ERR              (1 << 2)
#define OVER_RUN                (1 << 1)
#define RX_FIFO_DATA_READY      0x1


/* UART primitives */
#define GET_STATUS(port)        (UART_LSR(port))
#define RX_DATA(s)              ((s) & RX_FIFO_DATA_READY)
#define GET_CHAR(port)          (UART_RBR(port) & 0xFF)
#define TX_READY(s)             ((s) & TRANS_EMPTY)
#define PUT_CHAR(port, c)       (UART_THR(port) = (c & 0xFF))
#define UART_IERSET(port, v)    (UART_IER(port) = (v) )
#define UART_FIFOSET(port, v)   (UART_FCR(port) = (v) )
#define UART_TORSET(port, v)    (UART_TOR(port) = (v) )


#define ARM_BAUD_300            (300)
#define ARM_BAUD_1200           (1200)
#define ARM_BAUD_2400           (2400)
#define ARM_BAUD_4800           (4800)
#define ARM_BAUD_9600           (9600)
#define ARM_BAUD_14400          (14400)
#define ARM_BAUD_19200          (19200)
#define ARM_BAUD_28800          (28000)
#define ARM_BAUD_38400          (38400)
#define ARM_BAUD_57600          (57600)
#define ARM_BAUD_115200         (115200)
#define ARM_BAUD_230400         (230400)
#define ARM_BAUD_460800         (460800)


#endif /* _UART_REGS_WPCM450_H */
