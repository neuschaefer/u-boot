/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 ethernet driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(CONFIG_WPCM450NIC)

#include <net.h>

#include "cdefs.h"
#include "com_defs.h"
#include "wpcm450_eth.h"
#include "wpcm450_gctrl_regs.h"
#include "wpcm450_emc_regs.h"
#include "wpcm450_ncsi.h"


#if 0
#define EMC_DEBUG   1
#endif

/* enable to print out message */
#ifdef EMC_DEBUG
#define DEBUGP(format, args...) printf("EMC: " format, ## args)
#else
#define DEBUGP(format, args...)
#endif


struct eth_device emc_device;

static sFrameDescriptor RxFD[MaxRxFrameDescriptors] __attribute__ ((aligned(32)));
static sFrameDescriptor TxFD[MaxTxFrameDescriptors] __attribute__ ((aligned(32)));

static sFrameDescriptor *curRxFD;
static sFrameDescriptor *curTxFD;

/* PHY address */
UINT32 PHYAD = 0x00000100;
UINT8 emc_phy_state = PHY_STATE_RESET;


volatile UINT32 gCam0M, gCam0L;


/* define the ethernet port information */
static eth_info_type eth_info[] =
{
    /* {UINT32 emc_reg, UINT8 is_ncsi, UINT8 channel_id, char name[16]}, */
    
#ifdef CONFIG_WPCM450_SVB

#ifdef CONFIG_WPCM450NIC_USE_EMC2
    {EMC2_BA, 0, 0, "Port2"},
    {EMC1_BA, 0, 0, "Port1"},
#else
    {EMC1_BA, 0, 0, "Port1"},
    {EMC2_BA, 0, 0, "Port2"},
#endif

#else

#ifdef CONFIG_WPCM450_WHOVILLE
    {EMC1_BA, 1, CHNL_ID(0, 0), "LOM0 Gb1"},
    {EMC1_BA, 1, CHNL_ID(0, 1), "LOM0 Gb2"},
    {EMC1_BA, 1, CHNL_ID(1, 0), "LOM1 Gb3"},
    {EMC1_BA, 1, CHNL_ID(1, 1), "LOM1 Gb4"},
    {EMC2_BA, 0, 0, "AMEA"},
#endif

#endif
};


/* active ethernet port */
static UINT8 eth_port_select;

/* total number of available port */
static UINT8 eth_ports = sizeof(eth_info) / sizeof(eth_info_type);

/* ncsi initiation state */
static UINT8 forward_to_ncsi = 0;


volatile UINT32 gMCMDR = MCMDR_EnMDC | MCMDR_SPCRC | MCMDR_AEP | MCMDR_ACP 
                         | MCMDR_ARP | MCMDR_RXON;


/* MII interface station management register write */
void emc_mii_write(UINT32 PhyInAddr, UINT32 PhyAddr, UINT32 PhyWrData)
{
    MIID = PhyWrData;
    MIIDA = PhyInAddr | PhyAddr | PHYWR | MDCCR;
    MIIDA = MIIDA | PHYBUSY;
    while (MIIDA & PHYBUSY);
    MIID = 0;
}


/* MII interface station management register read */
UINT32 emc_mii_read(UINT32 PhyInAddr, UINT32 PhyAddr)
{
    UINT32 PhyRdData = 0;
    
    MIIDA = PhyInAddr | PhyAddr | PHYBUSY | MDCCR;
    MIIDA = MIIDA | PHYBUSY;
    
    while (MIIDA & PHYBUSY);
    
    PhyRdData = MIID;
    
    return PhyRdData;
}


/* PHY initation */
int emc_phy_init(void)
{
    UINT32 RdValue;
    UINT32 retry;
    int i;
    
    /* scan all possible PHY address*/
    for (i = 0; i < 32; i++)
    {
        DEBUGP("reset PHY %d\n",i);
        
        emc_mii_write(PHY_CNTL_REG, i << 8, RESET_PHY);
        
        RdValue = emc_mii_read(PHY_CNTL_REG, i << 8);
        
        DEBUGP("\PHY read = %d", RdValue);
        
        if ((RdValue & 0xF000) == 0x3000)
        {
            break;
        }
    }
    
    if (i == 32)
    {
        printf("\n*** fail to auto-scan ***\n");
        return -1 ;
    }
    
    /* save PHY address to variable */
    PHYAD = i << 8;
    
#ifdef CONFIG_DM9161A_PHY
    RdValue = emc_mii_read(PHY_DSC_REG, PHYAD);
    
    if (!(RdValue & 0x0100))
    {
        printf("WARNING, RMII is not enabled, set it by software !\n");
        RdValue |= 0x0100;
        emc_mii_write(PHY_DSC_REG, PHYAD, RdValue);
    }
#endif
    
    retry = 100000;
    
    while (1)
    {
        RdValue = emc_mii_read(PHY_CNTL_REG, PHYAD);
        
        if ((RdValue & RESET_PHY) == 0)
        {
            /* setup auto-negotiation advertisement register */
            emc_mii_write(PHY_ANA_REG,
                          PHYAD,
                          DR100_TX_FULL | DR100_TX_HALF | DR10_TX_FULL 
                          | DR10_TX_HALF | IEEE_802_3_CSMA_CD);
            
            RdValue = emc_mii_read(PHY_CNTL_REG, PHYAD);
            RdValue |= (ENABLE_AN | RESTART_AN);
            emc_mii_write(PHY_CNTL_REG, PHYAD, RdValue);
            
            break;
        }
        
        if (!(retry--))
        {
            printf("*** failed to reset phy ***\n");
            return -1;
        }
    }
    
    DEBUGP("PHY 1, CTRL REG   = %x\n", emc_mii_read(PHY_CNTL_REG, PHYAD));
    DEBUGP("PHY 1, STATUS REG = %x\n", emc_mii_read(PHY_STATUS_REG, PHYAD));
    
    /* change current PHY state */
    emc_phy_state = PHY_STATE_ADDRESS_OK;
    
    return 0;
}


/* negotiation to link partner */
int emc_negotiation(void)
{
    unsigned int RdValue;
#ifdef CONFIG_BCM5221_PHY
    unsigned int advertise, link_partner;
#endif
    
    UINT32 retry;
    
    /* check if PHY is initiated */
    if (emc_phy_state < PHY_STATE_ADDRESS_OK)
    {
        if (emc_phy_init() != 0)
        {
            return -1;
        }
    }
    
#ifndef AUTO_NEGOTIATION_ENABLED
    
    printf("\nSelect speed 10M, Full Duplex...");
    
    RdValue = emc_mii_read(PHY_CNTL_REG, PHYAD);
    
    DEBUGP("\n--> PHY_CNTL_REG(1) = 0x%x|", RdValue);
    
    emc_mii_write(PHY_CNTL_REG, 
                  PHYAD, 
                  (RdValue | PHY_FULLDUPLEX) & ~(DR_100MB | ENABLE_AN));
    
    RdValue = emc_mii_read(PHY_CNTL_REG, PHYAD);
    
    DEBUGP("\n--> PHY_CNTL_REG(2) = 0x%x|",RdValue);
    
#else /* auto negotiation */
    
    printf("Wait for auto-negotiation complete ... ");
    
    retry = 100000;
    
    /* wait for auto-negotiation complete */
    while (1)
    {
        RdValue = emc_mii_read(PHY_STATUS_REG, PHYAD);
    
        if ((RdValue & AN_COMPLETE) != 0)
        {
            break;
        }
        
        if (!(retry--))
        {
            printf("Fail\n");
            
            /* check link status */
            RdValue = emc_mii_read(PHY_STATUS_REG, PHYAD);
            
            if ((RdValue & LINK_STATUS_VALID) != LINK_STATUS_VALID)
            {
                printf("*** check ethernet cable ***\n");
            }
            
            /* re-trigger auto negotation action */
            RdValue = emc_mii_read(PHY_CNTL_REG, PHYAD);
            RdValue |= (ENABLE_AN | RESTART_AN);
            emc_mii_write(PHY_CNTL_REG, PHYAD, RdValue);
            
            return -1;
        }
    }
    
    printf("OK   ");
    
#endif /* auto-negotiation */
    
#ifdef CONFIG_BCM5221_PHY
    advertise = emc_mii_read(PHY_ANA_REG, PHYAD);
    link_partner = emc_mii_read(PHY_ANLPA_REG, PHYAD);
    
    if ((advertise & link_partner) & PHY_100M_DUPLEX)
    {
        gMCMDR |= ((MCMDR_OPMOD) | MCMDR_FDUP);
        printf("100MB - Full Duplex\n");
    }
    else if ((advertise & link_partner) & PHY_100M_HALF)
    {
        gMCMDR |= MCMDR_OPMOD;
        gMCMDR &= ~MCMDR_FDUP;
        printf("100MB - Half Duplex\n");
    }
    else if ((advertise & link_partner) & PHY_10M_DUPLEX)
    {
        gMCMDR &= ~MCMDR_OPMOD;
        gMCMDR |= MCMDR_FDUP;
        printf("10MB - Full Duplex\n");
    }
    else if ((advertise & link_partner) & PHY_10M_HALF)
    {
        gMCMDR &= ~MCMDR_OPMOD;
        gMCMDR &= ~MCMDR_FDUP;
        printf("10MB - Half Duplex\n");
    }
#endif
    
#ifdef CONFIG_DM9161A_PHY
    
    DEBUGP("W90P710: ");
    
    RdValue = emc_mii_read(PHY_CNTL_REG, PHYAD);
    
    DEBUGP("RdValue = %x", RdValue);
    
    /* 100MB */
    if ((RdValue & DR_100MB) != 0)
    {
        printf("100MB - ");
        
        gMCMDR |= MCMDR_OPMOD;
    }
    else
    {
        printf("10MB - ");
        
        gMCMDR &= ~MCMDR_OPMOD;
    }
    
    /* Full Duplex */
    if ((RdValue&PHY_FULLDUPLEX) != 0)
    {
        printf("Full Duplex\n");
    
        gMCMDR |= MCMDR_FDUP;
    }
    else
    {
        printf("Half Duplex\n");
        
        gMCMDR &= ~MCMDR_FDUP;
    }
    
#endif
    
#ifdef CONFIG_DP83848C_PHY
    
    DEBUGP("DP83848C PHYSTS\n");
    
    RdValue = emc_mii_read(PHY_PHYSTS_REG, PHYAD);
    
    DEBUGP("RdValue = %x\n", RdValue);
    
    /* 10MB */
    if ((RdValue & PHY_10MB_SUPPORT) != 0)
    {
        printf("10MB - ");
        
        gMCMDR &= ~MCMDR_OPMOD;
    }
    else
    {
        printf("100MB - ");
        
        gMCMDR |= MCMDR_OPMOD;
    }
    
    /* Full Duplex */
    if ((RdValue & PHY_FULLDUPLEX_SUPPORT) != 0)
    {
        printf("Full Duplex\n");
        
        gMCMDR |= MCMDR_FDUP;
    }
    else
    {
        printf("Half Duplex\n");
        
        gMCMDR &= ~MCMDR_FDUP;
    }
    
#endif
    
    /* change current PHY state */
    emc_phy_state = PHY_STATE_AUTO_NEGOTIATION_OK;
    
    return 0;
}


void emc_enable_cam_entry(int entry)
{
    CAMEN |= 0x00000001 << entry;
}


void emc_diable_cam_entry(int entry)
{
    CAMEN &= ~(0x00000001 << entry);
}


void emc_fill_cam_entry(int entry, UINT32 msw, UINT32 lsw)
{
    CAMxM_Reg(entry) = msw;
    CAMxL_Reg(entry) = lsw;
    
    emc_enable_cam_entry(entry);
}


int emc_oem_init(bd_t *bis)
{
#ifdef CONFIG_WPCM450_WHOVILLE
    if (strcmp(eth_info[eth_port_select].name, "AMEA") == 0)
    {
#ifdef CONFIG_WPCM450_WHOVILLE_X00
        
        /* select GPIO69 */
        CLEAR_BIT(MFSEL2, MF_FI5SEL_BIT);
        
        /* reset AMEA board, set GPIO69 to low */
        CLEAR_BIT(GP4DOUT, GPIO69);
        
        /* set GPIO69 as output */
        SET_BIT(GP4CFG0, GPIO69);
        
        /* delay */
        udelay(500*1000);
        
        /* set GPIO69 to high */
        SET_BIT(GP4DOUT, GPIO69);
#else
        
#ifdef CONFIG_WPCM450_WHOVILLE_X01
        
        /* select GPIO79 */
        CLEAR_BIT(MFSEL2, MF_FI15SEL_BIT);
        
        /* set GPIO79 to high */
        SET_BIT(GP4DOUT, GPIO79);
        
        /* set GPIO79 as output */
        SET_BIT(GP4CFG0, GPIO79);
        
#endif /* CONFIG_WPCM450_WHOVILLE_X01 */

#endif /* CONFIG_WPCM450_WHOVILLE_X00 */
    }
#endif /* CONFIG_WPCM450_WHOVILLE */
    
    return 0;
}


UINT8 emc_get_port(void)
{
    /* setup default ethernet port */
    eth_port_select = 0;
           
#ifdef CONFIG_WPCM450_WHOVILLE
    /* if amea is present, cpld base 0xC4000000, offset 0x12, bit 1 -> 0 */
    if ((*((UINT8 *) 0xC4000012) & 0x02) == 0)
    {
        eth_port_select = 4;
    }
#endif
    
    return eth_port_select;
}


static int emc_init(struct eth_device* dev, bd_t* bis)
{
    UINT32 RdValue;
    UINT8 i;
    int err;
    
    DEBUGP("emc_init\n");
    
    /* reset MAC  */
    MCMDR |= MCMDR_SWR;
    
    while (MCMDR & MCMDR_SWR);
    
    /* initialize Rx frame descriptor area-buffers */
    curRxFD = (sFrameDescriptor *) ((UINT32) &RxFD[0] | 0x80000000);
    
    RXDLSA = (UINT32) curRxFD;
    
    for (i = 0; i < MaxRxFrameDescriptors; i++) 
    {
        RxFD[i].Status1 = RXfOwnership_DMA;
        RxFD[i].FrameDataPtr = (u32) NetRxPackets[i] | 0x80000000;
        RxFD[i].Status2 = (UINT32)0x00;
        
        if (i == (MaxRxFrameDescriptors - 1))
        {
            RxFD[i].NextFrameDescriptor = (UINT32) curRxFD;
        }
        else
        {
            RxFD[i].NextFrameDescriptor = (UINT32) &RxFD[i + 1] | 0x80000000;
        }
    }
    
    /* initialize Tx frame descriptor area-buffers */
    curTxFD = (sFrameDescriptor *) ((UINT32) &TxFD[0] | 0x80000000);
    
    TXDLSA = (UINT32) curTxFD;
    
    for (i = 0; i < MaxTxFrameDescriptors; i++) 
    {
        TxFD[i].Status1 = (PaddingMode | CRCMode);
        TxFD[i].FrameDataPtr = (UINT32) 0x00;
        TxFD[i].Status2 = (UINT32) 0x00;
        
        if (i == (MaxTxFrameDescriptors - 1))
        {
            TxFD[i].NextFrameDescriptor = (UINT32) curTxFD;
        }
        else
        {
            TxFD[i].NextFrameDescriptor = (UINT32) &TxFD[i + 1] | 0x80000000;
        }
    }
    
    /* set MAC address to CAM */
    
    /* copy MAC address to global variable */
    for (i = 0; i < MAC_ADDR_SIZE - 2; i++)
    {
        gCam0M = (gCam0M << 8) | bis->bi_enetaddr[i];
    }
    
    for (i = (MAC_ADDR_SIZE - 2); i < MAC_ADDR_SIZE; i++)
    {
        gCam0L = (gCam0L << 8) | bis->bi_enetaddr[i];
    }
    
    gCam0L = (gCam0L << 16) ;
    
    emc_fill_cam_entry(0, gCam0M, gCam0L);
    
    /* set the CAM control register and the MAC address value */
    CAMCMR = CAM_ECMP | CAM_ABP | CAM_AUP;
    
    MIEN = 0;
    
    MCMDR = gMCMDR;
    
    if (!eth_info[eth_port_select].is_ncsi)
    {
        /* check link status and auto-negotiation complete flags */
        RdValue = emc_mii_read(PHY_STATUS_REG, PHYAD);
        
        if (((RdValue & LINK_STATUS_VALID) != LINK_STATUS_VALID)
            || ((RdValue & AN_COMPLETE) != AN_COMPLETE))
        {
            /* force to negotiate again */
            emc_phy_state = PHY_STATE_ADDRESS_OK;
        }
        
        /* PHY negotiation */
        if (emc_phy_state < PHY_STATE_AUTO_NEGOTIATION_OK)
        {
            if (emc_negotiation() != 0)
            {
                DEBUGP("fail to negotiation phy\n");
                return 0;
            }
            else
            {
                MCMDR = gMCMDR;
                
                DEBUGP("emc_negotiation\n");
            }
        }
    }
    else
    {
        /* forward packet to ncsi handler */
        forward_to_ncsi = 1;
        
        /* NC-SI init */
        ncsi_init();
        
        /* channel ID is pre-defined by NIC */
        if ((err = ncsi_open(eth_info[eth_port_select].channel_id, bis)) != 0)
        {
            DEBUGP("wpcm450_ncsi_open fail err code = %x\n", err);
        }
        
        /* disable unused ncsi port */
        for (i = 0; i < eth_ports; i++)
        {
            if ((eth_info[i].is_ncsi > 0) && (i != eth_port_select))
            {
                DEBUGP("ncsi_close[%d].channel_id=%x\n", 
                      i, eth_info[i].channel_id);
                
                ncsi_close(eth_info[i].channel_id);
            }
        }
        
        /* do not forward packet to ncsi handler */
        forward_to_ncsi = 0;
    }
    
    DEBUGP("MCMDR        %4x %4x\n", (MCMDR >> 16), 
                                     (MCMDR & 0xffff));
    DEBUGP("CAMCMR       %4x %4x\n", (CAMCMR >> 16), 
                                     (CAMCMR & 0xffff));
    DEBUGP("CAMEN        %4x %4x\n", (CAMEN >> 16), 
                                     (CAMEN & 0xffff));
    DEBUGP("CAMxM_Reg(0) %4x %4x\n", (CAMxM_Reg(0) >> 16), 
                                     (CAMxM_Reg(0) & 0xffff));
    DEBUGP("CAMxL_Reg(0) %4x %4x\n", (CAMxL_Reg(0) >> 16), 
                                     (CAMxL_Reg(0) & 0xffff));
    DEBUGP("exit emc_init\n");
    
    return 1;
}


static int emc_send(struct eth_device* dev, 
                           volatile void *packet, 
                           int length)
{
    int     status = -1;
    int     i;
    
    DEBUGP("emc_send -----------------------------------------------\n");
    
    DEBUGP("TXDLSA %4x %4x\n", (TXDLSA >> 16), (TXDLSA & 0xffff));
    DEBUGP("RXDLSA %4x %4x\n", (RXDLSA >> 16), (RXDLSA & 0xffff));
    DEBUGP("CTXDSA %4x %4x\n", (CTXDSA >> 16), (CTXDSA & 0xffff));
    DEBUGP("CTXBSA %4x %4x\n", (CTXBSA >> 16), (CTXBSA & 0xffff));
    DEBUGP("CRXDSA %4x %4x\n", (CRXDSA >> 16), (CRXDSA & 0xffff));
    DEBUGP("CRXBSA %4x %4x\n", (CRXBSA >> 16), (CRXBSA & 0xffff));
    
    DEBUGP("MISTA  %4x %4x\n", (MISTA >> 16), (MISTA & 0xffff));
    DEBUGP("MCMDR  %4x %4x\n", (MCMDR >> 16), (MCMDR & 0xffff));
    
    if (length <= 0)
    {
        printf("%s: bad packet size: %d\n", dev->name, length);
        return status;
    }
    
    DEBUGP("curTxFD->Status1 %4x %4x\n", (curTxFD->Status1 >> 16), 
                                         (curTxFD->Status1 & 0xffff));
    
    DEBUGP("curTxFD->Status2 %4x %4d\n", (curTxFD->Status2 >> 16), 
                                         (curTxFD->Status2 & 0xffff));
    
    DEBUGP("check tx df owner\n");
    
    for (i = 0; curTxFD->Status1 & TXfOwnership_DMA; i++)
    {
        if (i >= TOUT_LOOP)
        {
            printf("%s: tx error buffer not ready\n", dev->name);
            return status;
        }
    }
    
    DEBUGP("owner is cpu\n");
    
    /* cheange ownership to DMA */
    curTxFD->Status1 |= TXfOwnership_DMA;
    curTxFD->FrameDataPtr = (u32) packet | 0x80000000;
    
    /* set TX frame flag & length field */
    curTxFD->Status2 = (UINT32) (length & 0xffff);
    
    DEBUGP("curTxFD->FrameDataPtr 0x%x%4x\n", (curTxFD->FrameDataPtr >> 16), 
                                              (curTxFD->FrameDataPtr & 0xffff));
    
    DEBUGP("curTxFD->Status1 %4x %4x\n", (curTxFD->Status1 >> 16), 
                                         (curTxFD->Status1 & 0xffff));
    
    DEBUGP("curTxFD->Status2 %4x %4d\n", (curTxFD->Status2 >> 16), 
                                         (curTxFD->Status2 & 0xffff));
    
#ifdef EMC_DEBUG
    for (i = 0; i < length; i++)
    {
        if (i % 16)
        {
            printf("%02x ", *((u8 *) packet + i));
        }
        else
        {
            printf("\n%02x ", *((u8 *) packet + i));
        }
        
        if ((i % 16) == 7)
        {
            printf(" ");
        }
    }
    
    printf("\n");
#endif
    
    /* make TxDMA exit halt state */
    TSDR = 0;
    
    DEBUGP("enable TXON\n");
    
    MCMDR |= MCMDR_TXON;
    
    DEBUGP("check tx df owner\n");
    
    DEBUGP("MISTA  %4x %4x\n", (MISTA >> 16), (MISTA & 0xffff));
    
    DEBUGP("curTxFD->Status1 %4x %4x\n", (curTxFD->Status1 >> 16), 
                                         (curTxFD->Status1 & 0xffff));
    
    DEBUGP("curTxFD->Status2 %4x %4d\n", (curTxFD->Status2 >> 16), 
                                         (curTxFD->Status2 & 0xffff));
    
    for (i = 0; curTxFD->Status1 & TXfOwnership_DMA; i++)
    {
        if (i >= TOUT_LOOP) 
        {
            printf("%s: tx buffer not ready\n", dev->name);
            
            /* reset tx status of tx descriptor word 2 */
            curTxFD->Status2 = 0;
            
            /* reset mac interrupt status */
            MISTA = 0xffff0000;
            
            DEBUGP("curTxFD->Status1 %4x %4x\n", (curTxFD->Status1 >> 16), 
                                                 (curTxFD->Status1 & 0xffff));
            
            DEBUGP("curTxFD->Status2 %4x %4d\n", (curTxFD->Status2 >> 16), 
                                                 (curTxFD->Status2 & 0xffff));
            
            DEBUGP("MISTA            %4x %4x\n", (MISTA >> 16), 
                                                 (MISTA & 0xffff));
            
            return status;
        }
    }
    
    DEBUGP("owner returns to cpu i = %d\n", i);
    
    DEBUGP("check tx df status\n");
    DEBUGP("MISTA            %4x %4x\n", (MISTA >> 16), (MISTA & 0xffff));
    
    DEBUGP("curTxFD->Status1 %4x %4x\n", (curTxFD->Status1 >> 16), 
                                         (curTxFD->Status1 & 0xffff));
    
    DEBUGP("curTxFD->Status2 %4x %4d\n", (curTxFD->Status2 >> 16), 
                                         (curTxFD->Status2 & 0xffff));
    
    /* if transmission is complete */
    if ((curTxFD->Status2 >> 16) & TXFD_TXCP)
    {
        status = length;
        
        DEBUGP("length of tx frame %d\n", length);
        
        curTxFD = (sFrameDescriptor *) curTxFD->NextFrameDescriptor;
    }
    else
    {
        printf("tx is not complete %x\n", (curTxFD->Status2 >> 16));
    }
    
    /* reset tx status of tx descriptor word 2 */
    curTxFD->Status2 = 0;
    
    DEBUGP("reset mac int status\n");
    
    /* reset mac interrupt status */
    MISTA = 0xffff0000;
    
    DEBUGP("curTxFD->Status1 %4x %4x\n", (curTxFD->Status1 >> 16), 
                                         (curTxFD->Status1 & 0xffff));
    
    DEBUGP("curTxFD->Status2 %4x %4d\n", (curTxFD->Status2 >> 16), 
                                         (curTxFD->Status2 & 0xffff));
    
    DEBUGP("MISTA            %4x %4x\n", (MISTA >> 16), (MISTA & 0xffff));
    
    DEBUGP("exit emc_send ------------------------------------------\n");
    
    return status;
}


static int emc_recv(struct eth_device* dev)
{
    int length = 0;
    
#ifdef EMC_DEBUG
    int i = 0;
#endif
    
    DEBUGP("emc_recv -----------------------------------------------\n");
    
    DEBUGP("MCMDR        %4x %4x\n", (MCMDR >> 16), (MCMDR & 0xffff));
    DEBUGP("MISTA        %4x %4x\n", (MISTA >> 16), (MISTA & 0xffff));
    
    /* enable dma */
    if (MISTA & MISTA_RDU)
    {
        DEBUGP("reset RSDR\n");
        RSDR = 0;
    }
    
    while(1)
    {
        DEBUGP("curRxFD->Status1 %4x %4d\n", (curRxFD->Status1 >> 16), 
                                             (curRxFD->Status1 & 0xffff));
        
        DEBUGP("MISTA            %4x %4x\n", (MISTA >> 16), (MISTA & 0xffff));
        
        if ((curRxFD->Status1 | RXfOwnership_CPU) == RXfOwnership_CPU)
        {
            /* if rx frame is good, then process received frame */
            if (((curRxFD->Status1 >> 16) & 0xffff) & RXFD_RXGD)
            {
                /* get the receive byte count */
                length = (curRxFD->Status1 & 0xffff);
                
                DEBUGP("length of rx frame %d\n", length);
                
#ifdef EMC_DEBUG
                for (i = 0; i < length; i++)
                {
                    if (i % 16)
                    {
                        printf("%02x ", *((u8 *) curRxFD->FrameDataPtr + i));
                    }
                    else
                    {
                        printf("\n%02x ", *((u8 *) curRxFD->FrameDataPtr + i));
                    }
                    
                    if ((i % 16) == 7)
                    {
                        printf(" ");
                    }
                    
                }
                
                printf("\n");
#endif
                
                /* pass packets to the protocol layer */
                if (forward_to_ncsi == 0)
                {
                    NetReceive((uchar *) curRxFD->FrameDataPtr, length);
                }
                else
                {
                    ncsi_receive((uchar *) curRxFD->FrameDataPtr, length);
                }
                
                RSDR = 0;
            }
            else
            {
                printf("detect rx error status\n");
            }
            
            /* change ownership to DMA for next use */
            curRxFD->Status1 = RXfOwnership_DMA;
            
            DEBUGP("curRxFD->NextFrameDescriptor %4x %4x\n", 
                   (curRxFD->NextFrameDescriptor >> 16), 
                   (curRxFD->NextFrameDescriptor & 0xffff));
            
            curRxFD = (sFrameDescriptor *) curRxFD->NextFrameDescriptor;
            
            DEBUGP("curRxFD %4x %4x\n", ((UINT32) curRxFD >> 16), 
                   ((UINT32) curRxFD & 0xffff));
            
            DEBUGP("curRxFD->NextFrameDescriptor %4x %4x\n", 
                   (curRxFD->NextFrameDescriptor >> 16), 
                   (curRxFD->NextFrameDescriptor & 0xffff));
            
            break;
        }
        else
        {
            DEBUGP("no good frame %x\n", (curRxFD->Status1 >> 16));
            break;
        }
        
    }
    
    DEBUGP("MISTA        %4x %4x\n", (MISTA >> 16), (MISTA & 0xffff));
    
    DEBUGP("exit emc_recv ------------------------------------------\n");
    
    return length;
}


static void emc_halt(struct eth_device* dev)
{
    DEBUGP("emc_halt\n");
    
    /* disable rx and tx */
    MCMDR &= ~(MCMDR_RXON|MCMDR_TXON);
    
    DEBUGP("exit emc_halt\n");
    
    return;
}


int wpcm450nic_initialize(bd_t *bis)
{
    struct eth_device*  dev;
    UINT32 init_mac = 0;
    char parameter[32];
    int i;
    
    DEBUGP("eth_ports=%d\n", eth_ports);
    
    /* ethernet port selects by hardware signal */
    if (getenv("ethport") == NULL)
    {
        DEBUGP("port detects by hardware signal\n");
        
        /* get ethernet port */
        eth_port_select = emc_get_port();
        
        /* check if the eth_port_select is valid */
        if (eth_port_select >= eth_ports)
        {
            eth_port_select = 0;
        }
    }
    
    /* force to select by environment variable, ethport */
    else
    {
        DEBUGP("port detects by environment variable\n");
        
        sprintf(parameter, "%s", getenv("ethport"));
        
        DEBUGP("ethport=%s\n", getenv("ethport"));
        
        /* first entry is the default port */
        eth_port_select = 0;
        
        /* search for matching string */
        for (i = 0; i < eth_ports; i++)
        {
            if (strcmp(parameter, eth_info[i].name) == 0)
            {
                eth_port_select = i;
            }
        }
    }
    
    DEBUGP("eth_port_select = %d \n", eth_port_select);
    
    /* execute oem init */
    emc_oem_init(bis);
    
    /* setup emc register */
    EMC_BA = eth_info[eth_port_select].emc_reg;
    
    /* initiate multi function register */
    if (eth_info[eth_port_select].emc_reg == EMC1_BA)
    {
        /* management data I/O and clock don't need for NC-SI */
        if (!eth_info[eth_port_select].is_ncsi)
        {
            SET_BIT(MFSEL1, MF_R1MDSEL_BIT);
        }
        
        SET_BIT(MFSEL1, MF_R1ERRSEL_BIT);
    }
    else
    {
        /* management data I/O and clock don't need for NC-SI */
        if (!eth_info[eth_port_select].is_ncsi)
        {
            SET_BIT(MFSEL1, MF_R2MDSEL_BIT);
        }
        
        SET_BIT(MFSEL1, MF_R2ERRSEL_BIT);
        SET_BIT(MFSEL1, MF_RMII2SEL_BIT);
    }
    
    /* reset MAC  */
    MCMDR |= MCMDR_SWR;
    
    while (MCMDR & MCMDR_SWR);
    
    /* setup MCMDR register */
    if (eth_info[eth_port_select].is_ncsi)
    {
        gMCMDR = MCMDR_SPCRC | MCMDR_OPMOD | MCMDR_FDUP | MCMDR_ARP 
                 | MCMDR_ALP | MCMDR_RXON;
    }
    else
    {
        gMCMDR = MCMDR_EnMDC | MCMDR_SPCRC | MCMDR_AEP | MCMDR_ACP 
                 | MCMDR_ARP | MCMDR_RXON;
    }
    
    DEBUGP("emc_initialize\n");
    
    dev = &emc_device;
    
    sprintf(dev->name, "%s", eth_info[eth_port_select].name);
    
    dev->iobase = EMC_BA;
    
    dev->init   = emc_init;
    dev->halt   = emc_halt;
    dev->send   = emc_send;
    dev->recv   = emc_recv;
    
    eth_register(dev);
    
    /* initiate PHY to get PHY address */
    if (!eth_info[eth_port_select].is_ncsi)
    {
        /* enable MDC clock generation */
        MCMDR |= MCMDR_EnMDC;
        
        if (emc_phy_init() != 0)
        {
            return -1;
        }
    }
    
    init_mac = simple_strtoul(getenv("init_mac"), NULL, 16);
    
    /* force to initiate MAC controller at this time */
    if (init_mac)
    {
        /* init ethernet module */
        emc_init(0, bis);
    }
    
    return 0;
}


#endif  /* CFG_CMD_NET && CONFIG_NET_MULTI && CONFIG_WPCM450NIC */
