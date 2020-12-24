/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 EMC register definitions .
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */

#ifndef _EMC_REGS_WPCM450_H
#define _EMC_REGS_WPCM450_H


/*-----------------------------------------------------------------------
 * WPCM450 EMC register definitions
 *-----------------------------------------------------------------------*/
#define EMC1_BA  0xB0002000
#define EMC2_BA  0xB0003000


/* use EMC1 as default */
UINT32 EMC_BA = EMC1_BA;


/* EMC Control Registers */
#define CAMCMR       (VPlong(EMC_BA + 0x00)) /* CAM Registers */
#define CAMEN        (VPlong(EMC_BA + 0x04))
#define CAM0M_Base   EMC_BA + 0x08
#define CAM0L_Base   EMC_BA + 0x0C
#define CAMxM_Reg(x) (VPlong(CAM0M_Base + x * 0x08))
#define CAMxL_Reg(x) (VPlong(CAM0L_Base + x * 0x08))

#define TXDLSA   (VPlong(EMC_BA + 0x88)) /* Transmit Descriptor Link List Start Address Register */
#define RXDLSA   (VPlong(EMC_BA + 0x8C)) /* Receive Descriptor Link List Start Address Register */
#define MCMDR    (VPlong(EMC_BA + 0x90)) /* MAC Command Register */
#define MIID     (VPlong(EMC_BA + 0x94)) /* MII Management Data Register */
#define MIIDA    (VPlong(EMC_BA + 0x98)) /* MII Management Control and Address Register */
#define FFTCR    (VPlong(EMC_BA + 0x9C)) /* FIFO Threshold Control Register */
#define TSDR     (VPlong(EMC_BA + 0xA0)) /* Transmit Start Demand Register */
#define RSDR     (VPlong(EMC_BA + 0xA4)) /* Receive Start Demand Register */
#define DMARFC   (VPlong(EMC_BA + 0xA8)) /* Maximum Receive Frame Control Register */
#define MIEN     (VPlong(EMC_BA + 0xAC)) /* MAC Interrupt Enable Register */

/* EMC Status Registers */
#define MISTA    (VPlong(EMC_BA + 0xB0)) /* MAC Interrupt Status Register */
#define MGSTA    (VPlong(EMC_BA + 0xB4)) /* MAC General Status Register */
#define MPCNT    (VPlong(EMC_BA + 0xB8)) /* Missed Packet Count Register */
#define MRPC     (VPlong(EMC_BA + 0xBC)) /* MAC Receive Pause Count Register */
#define MRPCC    (VPlong(EMC_BA + 0xC0)) /* MAC Receive Pause Current Count Register */
#define MREPC    (VPlong(EMC_BA + 0xC4)) /* MAC Remote Pause Count Register */
#define DMARFS   (VPlong(EMC_BA + 0xC8)) /* DMA Receive Frame Status Register */
#define CTXDSA   (VPlong(EMC_BA + 0xCC)) /* Current Transmit Descriptor Start Address Register */
#define CTXBSA   (VPlong(EMC_BA + 0xD0)) /* Current Transmit Buffer Start Address Register */
#define CRXDSA   (VPlong(EMC_BA + 0xD4)) /* Current Receive Descriptor Start Address Register */
#define CRXBSA   (VPlong(EMC_BA + 0xD8)) /* Current Receive Buffer Start Address Register */

/* EMC Diagnostic Registers */
#define RXFSM   (VPlong(EMC1_BA + 0x200)) /* Receive Finite State Machine Register */
#define TXFSM   (VPlong(EMC1_BA + 0x204)) /* Transmit Finite State Machine Register */
#define FSM0    (VPlong(EMC1_BA + 0x208)) /* Finite State Machine Register 0 */
#define FSM1    (VPlong(EMC1_BA + 0x20C)) /* Finite State Machine Register 1 */
#define DCR     (VPlong(EMC1_BA + 0x210)) /* Debug Configuration Register */
#define DMMIR   (VPlong(EMC1_BA + 0x214)) /* Debug Mode MAC Information Register */
#define BISTR   (VPlong(EMC1_BA + 0x300)) /* BIST Mode Register */


#endif  /* _EMC_REGS_WPCM450_H */
