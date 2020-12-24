/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 SDHC driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */

#include "common.h"
#include "config.h"

#include <mmc.h>
#include <part.h>
#include <fat.h>

#include "cdefs.h"
#include "com_defs.h"
#include "wpcm450_gctrl_regs.h"
#include "wpcm450_clk_regs.h"
#include "wpcm450_fwupgrade.h"
#include "wpcm450_mmc.h"


#ifdef CONFIG_MMC


#if 0
#define PRINTD(fmt,args...) printf("MMC: " fmt ,##args)
#else
#define PRINTD(fmt,args...)
#endif

#if 0
#define MMC_DEBUG
#endif


/* card information */
static struct mmc_card_info card_info;

extern int fat_register_device(block_dev_desc_t *dev_desc, int part_no);

static block_dev_desc_t mmc_dev;

static UINT8 mmc_init_state = 0;

static int mmc_soft_reset(void)
{
    PRINTD("mmc_soft_reset\n");
    
    /* clear all errors */
    SDHC_ERR_INT_STS = 0xF3FF;
    
    /* software reset for DAT line and CMD line */
    SET_BIT(SDHC_SW_RST, SW_RST_DAT_P);
    SET_BIT(SDHC_SW_RST, SW_RST_CMD_P);
    
    /* wait for software reset */
    while(SDHC_SW_RST);
    
    return 0;
}


/* timeout unit = 1ms */
static int mmc_wait_complete(UINT32 timeout, struct mmc_data *data)
{
    UINT32 retry = 0;
    UINT32 boundary;
    static UINT16 print_idx = 0;
    
    PRINTD("mmc_wait_complete\n");
    
    while(1)
    {
        /* check command complete */
        if (READ_BIT(SDHC_NRML_INT_STS, CMD_COMPLETE_P))
        {
            PRINTD("00 SDHC_NRML_INT_STS=%lx\n", SDHC_NRML_INT_STS);
            PRINTD("*** SDHC_ERR_INT_STS=%lx ***\n", SDHC_ERR_INT_STS);
            
            /* clear command complete status */
            SDHC_NRML_INT_STS = 1 << CMD_COMPLETE_P;
            
            break;   
        }
        
        if (retry >= timeout)
        {
            PRINTD("*** command complete timeout %ld ms ***\n", timeout);
            PRINTD("*** SDHC_NRML_INT_STS=%lx ***\n", SDHC_NRML_INT_STS);
            PRINTD("*** SDHC_ERR_INT_STS=%lx ***\n", SDHC_ERR_INT_STS);
            
            /* software reset */
            mmc_soft_reset();
            
            return CARD_ERROR_TIMEOUT;
        }
        
        /* delay 2ms */
        udelay(2000);
        
        retry += 2;
    }
    
    /* Save Response */
    if (data->rsp_type_sel != RSP_NONE)
    {
        data->resp[0] = SDHC_RSP0;
    }
    
    if (data->rsp_type_sel == RSP_136)
    {
        data->resp[1] = SDHC_RSP1;
        data->resp[2] = SDHC_RSP2;
        data->resp[3] = SDHC_RSP3;
    }
    
    /* print respond data */
    PRINTD("resp[0]=%08lx, resp[1]=%08lx, resp[2]=%08lx, resp[3]=%08lx\n\n",
           data->resp[0],
           data->resp[1],
           data->resp[2],
           data->resp[3]
          );
    
#if 0
    /* reset retry count */
    retry = 0;
    
    /* transfer data */
    while (data->date_prsnt_sel)
    {
        PRINTD("data->buffer=%lx\n", &data->buffer[0]);
        
        if (data->is_writing)
        {
            PRINTD("writing\n");
            
            /* check buffer write enable */
            if (READ_BIT(SDHC_PRSNT_STATE, BUF_WRITE_EN_P))
            {
                PRINTD("buffer write enable\n");
                
                for (i = 0; i < 512; i = i + 4)
                {
                    /* write data to buffer data port register */
                    SDHC_BUF_DATA = *((UINT32 *) &data->buffer[i]);
                    
                    PRINTD("data->buffer[%ld]=%lx\n", i, *((UINT32 *) &data->buffer[i]));
                }
                
                break;   
            }
        }
        else
        {
            PRINTD("reading\n");
            
            /* check buffer read enable */
            if (READ_BIT(SDHC_PRSNT_STATE, BUF_READ_EN_P))
            {
                PRINTD("buffer read enable\n");
                
                for (i = 0; i < 512; i = i + 4)
                {
                    /* read data from buffer data port register */
                    *((UINT32 *) &data->buffer[i]) = SDHC_BUF_DATA;
                    
                    PRINTD("data->buffer[%ld]=%lx\n", i, *((UINT32 *) &data->buffer[i]));
                }
                
                break;   
            }
        }
        
        if (retry >= timeout)
        {
            PRINTD("*** transfer timeout %ld ms ***\n", timeout);
            PRINTD("*** SDHC_NRML_INT_STS=%lx ***\n", SDHC_NRML_INT_STS);
            PRINTD("*** SDHC_ERR_INT_STS=%lx ***\n", SDHC_ERR_INT_STS);
            
            /* software reset */
            mmc_soft_reset();
            
            return CARD_ERROR_TIMEOUT;
        }
        
        /* delay 1ms */
        udelay(1000);
        
        
        retry++;
    }
#endif
    
    /* reset retry count */
    retry = 0;
    
    /* calculate DMA boundary */ 
    boundary = (UINT32) data->buffer + (data->blocks * 512);
    
    /* check transfer complete */
    while ((data->date_prsnt_sel) || (data->rsp_type_sel == RSP_48_BUSY))
    {
#ifdef MMC_DEBUG
        if (SDHC_NRML_INT_STS)
        {
            printf("%lx", SDHC_NRML_INT_STS);
        }
#endif
        
        /* check transfer complete */
        if (READ_BIT(SDHC_NRML_INT_STS, XFR_COMPLETE_P))
        {
#ifdef MMC_DEBUG
            printf("N");
#endif
            
            PRINTD("detect transfer complete\n");
            
            PRINTD("02 SDHC_NRML_INT_STS=%lx\n", SDHC_NRML_INT_STS);
            
            /* clear transfer complete status */
            SDHC_NRML_INT_STS = 1 << XFR_COMPLETE_P;
            
            /* clear DMA interrupt status */
            SDHC_NRML_INT_STS = 1 << DMA_INT_P;
            
            PRINTD("03 SDHC_NRML_INT_STS=%lx\n", SDHC_NRML_INT_STS);
            
            if (print_idx != 0)
            {
                if (data->show_info)
                    printf("\n");
                
                print_idx = 0;
            }
            
            break;
        }
        
        /* check DMA interrupt */
        if (READ_BIT(SDHC_NRML_INT_STS, DMA_INT_P))
        {
#ifdef MMC_DEBUG
            printf("D");
#endif
            
            /* delay 2ms */
            udelay(2000);
            retry += 2;
            
            PRINTD("detect DMA interrupt\n");
            
            PRINTD("04 SDHC_NRML_INT_STS=%lx\n", SDHC_NRML_INT_STS);
            
            /* DMA boundary is 512K bytes, 0x80000 */
            data->buffer = data->buffer + 0x80000 - ((UINT32) data->buffer & 0x7FFFF);
            
            /* check if all data have been transferred */
            if ((UINT32) data->buffer >= boundary)
            {
                PRINTD("bf out of boundary, SDHC_NRML_INT_STS=%lx\n", SDHC_NRML_INT_STS);
                
                /* clear DMA interrupt status */
                SDHC_NRML_INT_STS = 1 << DMA_INT_P;
                
                PRINTD("af out of boundary, SDHC_NRML_INT_STS=%lx\n", SDHC_NRML_INT_STS);
            }
            else
            {

                /* clear DMA interrupt status */
                SDHC_NRML_INT_STS = 1 << DMA_INT_P;
                
                /* set system address register */
                SDHC_DMA_ADD = (UINT32) data->buffer;
                
                /* print # every 512K bytes */
                if (print_idx > 61)
                {
                    if (data->show_info)
                        printf("\n         ");
                    print_idx = 0;
                }
                else
                {
                    if (data->show_info)
                        printf("#");
                    print_idx++;
                }
                
                
            }
            
            PRINTD("05 SDHC_NRML_INT_STS=%lx\n", SDHC_NRML_INT_STS);
            
            /* reset retry counter */
            retry = 0;
            
            PRINTD("reset retry counter to %ld\n", retry);
        }
        
        if (retry >= timeout)
        {
            PRINTD("*** transfer complete timeout %ld ms, data->buffer=%p ***\n", timeout, data->buffer);
            PRINTD("*** SDHC_NRML_INT_STS=%lx ***\n", SDHC_NRML_INT_STS);
            PRINTD("*** SDHC_ERR_INT_STS=%lx ***\n", SDHC_ERR_INT_STS);
            
            /* software reset */
            mmc_soft_reset();
            
            print_idx = 0;
            
            return CARD_ERROR_TIMEOUT;
        }
        
        /* delay 1ms */
        udelay(1000);
        
        retry++;
    }
    
    /* check errors */
    if (SDHC_ERR_INT_STS)
    {
        printf("*** SDHC_ERR_INT_STS=%x, retry=%ld ***\n\n", SDHC_ERR_INT_STS, retry);
        
        /* software reset */
        mmc_soft_reset();
        
        return CARD_ERROR_PROTOCOL;
    }
    
    return 0;
}


#if 0
/* timeout unit = 1ms */
static int mmc_wait_adtc_complete(UINT32 timeout, 
                                  UINT32 size, 
                                  struct mmc_data *data)
{
    UINT32 retry = 0;
    UINT32 i;
    
    PRINTD("mmc_wait_complete\n");
    
    while(1)
    {
        /* check command complete */
        if (READ_BIT(SDHC_NRML_INT_STS, CMD_COMPLETE_P))
        {
            PRINTD("00 SDHC_NRML_INT_STS=%lx\n", SDHC_NRML_INT_STS);
            PRINTD("*** SDHC_ERR_INT_STS=%lx ***\n", SDHC_ERR_INT_STS);
            
            /* clear command complete status */
            SDHC_NRML_INT_STS = 1 << CMD_COMPLETE_P;
            
            break;   
        }
        
        if (retry >= timeout)
        {
            PRINTD("*** command complete timeout %ld ms ***\n", timeout);
            PRINTD("*** SDHC_NRML_INT_STS=%lx ***\n", SDHC_NRML_INT_STS);
            PRINTD("*** SDHC_ERR_INT_STS=%lx ***\n", SDHC_ERR_INT_STS);
            
            /* software reset */
            mmc_soft_reset();
            
            return CARD_ERROR_TIMEOUT;
        }
        
        /* delay 1ms */
        udelay(1000);
        
        retry++;
    }
    
    /* Save Response */
    if (data->rsp_type_sel != RSP_NONE)
    {
        data->resp[0] = SDHC_RSP0;
    }
    
    if (data->rsp_type_sel == RSP_136)
    {
        data->resp[1] = SDHC_RSP1;
        data->resp[2] = SDHC_RSP2;
        data->resp[3] = SDHC_RSP3;
    }
    
    /* print respond data */
    PRINTD("resp[0]=%08lx, resp[1]=%08lx, resp[2]=%08lx, resp[3]=%08lx\n\n",
           data->resp[0],
           data->resp[1],
           data->resp[2],
           data->resp[3]
          );
    
    /* reset retry count */
    retry = 0;
    
    /* transfer data */
    while (data->date_prsnt_sel)
    {
        /* check buffer read enable */
        if (READ_BIT(SDHC_PRSNT_STATE, BUF_READ_EN_P))
        {
            PRINTD("buffer read enable\n");
            
            for (i = 0; i < size; i = i + 4)
            {
                /* read data from buffer data port register */
                *((UINT32 *) &data->buffer[i]) = SDHC_BUF_DATA;
                
                PRINTD("data->buffer[%ld]=%lx\n", i, *((UINT32 *) &data->buffer[i]));
            }
            
            /* clear transfer complete status */
            SDHC_NRML_INT_STS = 1 << XFR_COMPLETE_P;
            
            break;   
        }
        
        if (retry >= timeout)
        {
            PRINTD("*** transfer timeout %ld ms ***\n", timeout);
            PRINTD("*** SDHC_NRML_INT_STS=%lx ***\n", SDHC_NRML_INT_STS);
            PRINTD("*** SDHC_ERR_INT_STS=%lx ***\n", SDHC_ERR_INT_STS);
            
            /* software reset */
            mmc_soft_reset();
            
            return CARD_ERROR_TIMEOUT;
        }
        
        /* delay 1ms */
        udelay(1000);
        
        retry++;
    }
    
    /* check errors */
    if (SDHC_ERR_INT_STS)
    {
        printf("*** SDHC_ERR_INT_STS=%x, retry=%ld ***\n\n", SDHC_ERR_INT_STS, retry);
        
        /* software reset */
        mmc_soft_reset();
        
        return CARD_ERROR_PROTOCOL;
    }
    
    return 0;
}
#endif


static int mmc_prepare_cmd(struct mmc_data *data)
{
    UINT8 rsp_type;
    
    /* PRINTD("mmc_prepare_cmd\n"); */
     
    data->cmd_type = CMD_NORMAL;
    data->date_prsnt_sel = 0;
    
    rsp_type = SDHC_RESP_ILLEGAL;
    
    /* Determine command validity and response type (and more...) */
    switch (data->cmd_idx)
    {
        case SDHC_CMD_GO_IDLE_STATE:            /* CMD0   */
        case SDHC_CMD_SET_DSR:                  /* CMD4   */
        case SDHC_CMD_GO_INACTIVE_STATE:        /* CMD15  */
            rsp_type = SDHC_RESP_NONE;
            break;
#if IS_ENABLED(LUN_MMC)
        case SDHC_CMD_SEND_OP_COND:             /* CMD1   */
            if (CARD_IS_MMC(card_info.card_type))
            {
                rsp_type = SDHC_RESP_R3;
            }
            break;
#endif
        case SDHC_CMD_ALL_SEND_CID:             /* CMD2   */
        case SDHC_CMD_SEND_CSD:                 /* CMD9   */
        case SDHC_CMD_SEND_CID:                 /* CMD10  */
            rsp_type = SDHC_RESP_R2;
            break;
        case SDHC_CMD_SEND_RELATIVE_ADDR:       /* CMD3   */
            if (CARD_IS_MMC(card_info.card_type))
            {
                rsp_type = SDHC_RESP_R1;
            }
            else
            {
                rsp_type = SDHC_RESP_R6;
            }
            break;
        case SDHC_CMD_SWITCH_FUNC:              /* CMD6   */
        case SDHC_CMD_LOCK_UNLOCK:              /* CMD42  */        
        case SDHC_CMD_GEN_CMD:                  /* CMD56  */        
            if (CARD_IS_MMC(card_info.card_type))
            {
                rsp_type = SDHC_RESP_R1b;
            }
            else
            {
                rsp_type = SDHC_RESP_R1;
            }
            break;      
        case SDHC_CMD_SELECT_DESELECT_CARD:     /* CMD7   */
        case SDHC_CMD_SET_WRITE_PROT:           /* CMD28  */
        case SDHC_CMD_CLR_WRITE_PROT:           /* CMD29  */
        case SDHC_CMD_ERASE:                    /* CMD38  */
            rsp_type = SDHC_RESP_R1b;
            break;
#if IS_ENABLED(LUN_MMC)             
        case SDHC_CMD_SEND_EXT_CSD:             /* CMD8   */
        case SDHC_CMD_READ_DAT_UNTIL_STOP:      /* CMD11  */
        case SDHC_CMD_WRITE_DAT_UNTIL_STOP:     /* CMD20  */
        case SDHC_CMD_BUSTEST_R:                /* CMD14  */
        case SDHC_CMD_BUSTEST_W:                /* CMD19  */
            if (CARD_IS_MMC(card_info.card_type))
            {
                data->date_prsnt_sel = 1;
                rsp_type = SDHC_RESP_R1;
            }
            break;
#endif
        case SDHC_CMD_STOP_TRANSMISSION:        /* CMD12  */
            data->cmd_type = CMD_ABORT;
            rsp_type = SDHC_RESP_R1b;                       
            break;
        case SDHC_CMD_SEND_STATUS:              /* CMD13  */
        case SDHC_CMD_SET_BLOCKLEN:             /* CMD16  */
        case SDHC_CMD_APP_CMD:                  /* CMD55  */        
        case SDHC_ACMD_SET_BUS_WIDTH:           /* ACMD6  */        
        case SDHC_ACMD_SET_WR_BLK_ERASE_COUNT:  /* ACMD23 */
        case SDHC_ACMD_SET_CLR_CARD_DETECT:     /* ACMD42 */        
            rsp_type = SDHC_RESP_R1;            
            break;
        case SDHC_CMD_READ_SINGLE_BLOCK:        /* CMD17  */
        case SDHC_CMD_READ_MULTIPLE_BLOCK:      /* CMD18  */
        case SDHC_CMD_WRITE_SINGLE_BLOCK:       /* CMD24  */
        case SDHC_CMD_WRITE_MULTIPLE_BLOCK:     /* CMD25  */
        case SDHC_CMD_PROGRAM_CSD:              /* CMD27  */
        case SDHC_CMD_SEND_WRITE_PROT:          /* CMD30  */
        case SDHC_ACMD_SD_STATUS:               /* ACMD13 */
        case SDHC_ACMD_SEND_NUM_WR_BLOCKS:      /* ACMD22 */
        case SDHC_ACMD_SEND_SCR:                /* ACMD51 */
            data->date_prsnt_sel = 1;
            rsp_type = SDHC_RESP_R1;
            break;
#if IS_ENABLED(LUN_MMC)
        case SDHC_CMD_SET_BLOCK_COUNT:          /* CMD23  */
        case SDHC_CMD_ERASE_GROUP_START:        /* CMD35  */
        case SDHC_CMD_ERASE_GROUP_END:          /* CMD36  */
            if (CARD_IS_MMC(card_info.card_type))
            {
                rsp_type = SDHC_RESP_R1;
            }
            break;
#endif
#if IS_ENABLED(LUN_SD)
        case SDHC_CMD_ERASE_WR_BLK_START:       /* CMD32  */
        case SDHC_CMD_ERASE_WR_BLK_END:         /* CMD33  */
            if (CARD_IS_SD(card_info.card_type))
            {
                rsp_type = SDHC_RESP_R1;
            }
            break;
#endif
#if IS_ENABLED(LUN_MMC)
        case SDHC_CMD_FAST_IO:                  /* CMD39  */
            if (CARD_IS_MMC(card_info.card_type))
            {
                rsp_type = SDHC_RESP_R4;
            }
            break;
        case SDHC_CMD_GO_IRQ_STATE:             /* CMD40  */
            if (CARD_IS_MMC(card_info.card_type))
            {
                rsp_type = SDHC_RESP_R5;
            }
            break;
#endif
#ifdef SUPPORT_SD_2_0
        case SDHC_CMD_SEND_IF_COND:             /* CMD8 */
            rsp_type = SDHC_RESP_R7;
            break;
#endif
        case SDHC_ACMD_SD_SEND_OP_COND:         /* ACMD41 */
            rsp_type = SDHC_RESP_R3;
            break;
        default:
            return CARD_ERROR_PROTOCOL;
    }
    
    /* Relation between parameters and the name of response type */
    switch (rsp_type)
    {
        case SDHC_RESP_NONE:
            data->cmd_idx_chk_en = 0;
            data->cmd_crc_chk_en = 0;
            data->rsp_type_sel = RSP_NONE;
            break;
            
        case SDHC_RESP_R1:
        case SDHC_RESP_R5:
        case SDHC_RESP_R6:
#ifdef SUPPORT_SD_2_0
        case SDHC_RESP_R7:
#endif
            data->cmd_idx_chk_en = 1;
            data->cmd_crc_chk_en = 1;
            data->rsp_type_sel = RSP_48;
            break;
            
        case SDHC_RESP_R1b:
        case SDHC_RESP_R5b:
            data->cmd_idx_chk_en = 1;
            data->cmd_crc_chk_en = 1;
            data->rsp_type_sel = RSP_48_BUSY;
            break;
            
        case SDHC_RESP_R2:
            data->cmd_idx_chk_en = 0;
            data->cmd_crc_chk_en = 1;
            data->rsp_type_sel = RSP_136;
            break;
            
        case SDHC_RESP_R3:
        case SDHC_RESP_R4:
            data->cmd_idx_chk_en = 0;
            data->cmd_crc_chk_en = 0;
            data->rsp_type_sel = RSP_48;
            break;
            
        default:
            return CARD_ERROR_PARAMETER;
    }
    
    return 0;
}


/* timeout unit is 1 ms */
static int mmc_send_cmd(SDHC_CMDS_TYPE cmd_idx, 
                        UINT32 arg, 
                        UINT32 timeout,
                        struct mmc_data *data)
{
    int ret;
    UINT16 cmd_reg = 0;
    
    /* PRINTD("mmc_send_cmd\n"); */
    
    /* feed and clean up internal data */
    data->cmd_idx = cmd_idx;
    data->arg = arg;
    data->resp[0] = 0;
    data->resp[1] = 0;
    data->resp[2] = 0;
    data->resp[3] = 0;
    
    ret = mmc_prepare_cmd(data);
    
    if(ret)
        return ret;
    
    PRINTD("cmd_idx=%d, arg=%lx, cmd_type=%d, \n",
           data->cmd_idx & CMD_MASK, 
           data->arg, 
           data->cmd_type
          );
    
#if 0
    PRINTD("cmd_idx=%d, arg=%lx, cmd_type=%d, \n" \
           "     date_prsnt_sel=%d, \n" \
           "     cmd_idx_chk_en=%d, \n" \
           "     cmd_crc_chk_en=%d, \n" \
           "     rsp_type_sel=%d\n\n",
           data->cmd_idx & CMD_MASK, 
           data->arg, 
           data->cmd_type, 
           data->date_prsnt_sel, 
           data->cmd_idx_chk_en, 
           data->cmd_crc_chk_en, 
           data->rsp_type_sel
          );
#endif
    
    /* Verify CMD line is not in use */
    if (READ_BIT(SDHC_PRSNT_STATE, CMD_INHIBIT_P))
    {
        printf("CMD_INHIBIT\n");
        
        /* software reset */
        mmc_soft_reset();
        
        return CARD_ERROR_PROTOCOL;
    }
    
    /* Verify DAT line is not in use */
    if (READ_BIT(SDHC_PRSNT_STATE, DAT_INHIBIT_P))
    {
        printf("DAT_INHIBIT\n");
        
        /* software reset */
        mmc_soft_reset();
        
        return CARD_ERROR_PROTOCOL;
    }
    
    /* Set Argument Register */
    SDHC_ARG = data->arg;

    /* Generate Command */
    SET_FIELD(cmd_reg, RSP_TYPE_SEL, data->rsp_type_sel);
    SET_FIELD(cmd_reg, CMD_CRC_CHK_EN, data->cmd_crc_chk_en);
    SET_FIELD(cmd_reg, CMD_IDX_CHK_EN, data->cmd_idx_chk_en);
    
    SET_FIELD(cmd_reg, DATA_PRSNT_SEL, data->date_prsnt_sel);
    SET_FIELD(cmd_reg, CMD_TYPE, data->cmd_type);
    SET_FIELD(cmd_reg, CMD_IDX, ((UINT8)data->cmd_idx & CMD_MASK));
    
    /* Set Command Register */
    SDHC_CMD = cmd_reg;
    
    /* set the default timeout */
    if(timeout == 0)
    {
        /* default is 1000 ms */
        timeout = 1000;
    }
    
    ret = mmc_wait_complete(timeout, data);
    
    if(ret)
        return ret;
    
    return CARD_SUCCESS;
}


#if 0
/* timeout unit is 1 ms */
static int mmc_send_adtc_cmd(SDHC_CMDS_TYPE cmd_idx, 
                             UINT32 arg, 
                             UINT32 size,
                             UINT32 timeout,
                             struct mmc_data *data)
{
    int ret;
    UINT16 cmd_reg = 0;
    
    /* PRINTD("mmc_send_cmd\n"); */
    
    /* feed and clean up internal data */
    data->cmd_idx = cmd_idx;
    data->arg = arg;
    data->resp[0] = 0;
    data->resp[1] = 0;
    data->resp[2] = 0;
    data->resp[3] = 0;
    
    ret = mmc_prepare_cmd(data);
    
    if(ret)
        return ret;
    
    PRINTD("cmd_idx=%d, arg=%lx, cmd_type=%d, \n",
           data->cmd_idx & CMD_MASK, 
           data->arg, 
           data->cmd_type
          );
    
#if 0
    PRINTD("cmd_idx=%d, arg=%lx, cmd_type=%d, \n" \
           "     date_prsnt_sel=%d, \n" \
           "     cmd_idx_chk_en=%d, \n" \
           "     cmd_crc_chk_en=%d, \n" \
           "     rsp_type_sel=%d\n\n",
           data->cmd_idx & CMD_MASK, 
           data->arg, 
           data->cmd_type, 
           data->date_prsnt_sel, 
           data->cmd_idx_chk_en, 
           data->cmd_crc_chk_en, 
           data->rsp_type_sel
          );
#endif
    
    /* Verify CMD line is not in use */
    if (READ_BIT(SDHC_PRSNT_STATE, CMD_INHIBIT_P))
    {
        printf("CMD_INHIBIT\n");
        
        /* software reset */
        mmc_soft_reset();
        
        return CARD_ERROR_PROTOCOL;
    }
    
    /* Verify DAT line is not in use */
    if (READ_BIT(SDHC_PRSNT_STATE, DAT_INHIBIT_P))
    {
        printf("DAT_INHIBIT\n");
        
        /* software reset */
        mmc_soft_reset();
        
        return CARD_ERROR_PROTOCOL;
    }
    
    /* Set Argument Register */
    SDHC_ARG = data->arg;

    /* Generate Command */
    SET_FIELD(cmd_reg, RSP_TYPE_SEL, data->rsp_type_sel);
    SET_FIELD(cmd_reg, CMD_CRC_CHK_EN, data->cmd_crc_chk_en);
    SET_FIELD(cmd_reg, CMD_IDX_CHK_EN, data->cmd_idx_chk_en);
    
    SET_FIELD(cmd_reg, DATA_PRSNT_SEL, data->date_prsnt_sel);
    SET_FIELD(cmd_reg, CMD_TYPE, data->cmd_type);
    SET_FIELD(cmd_reg, CMD_IDX, ((UINT8)data->cmd_idx & CMD_MASK));
    
    /* Set Command Register */
    SDHC_CMD = cmd_reg;
    
    /* set the default timeout */
    if(timeout == 0)
    {
        /* default is 500 ms */
        timeout = 500;
    }
    
    ret = mmc_wait_adtc_complete(timeout, size, data);
    
    if(ret)
        return ret;
    
    return CARD_SUCCESS;
}
#endif


static int mmc_block_read(struct mmc_data *data,
                          UINT32 offset,
                          UINT8 *buffer,
                          UINT32 blocks,
                          UINT8 show_info)
{
    PRINTD("mmc_block_read\n");
    
    int ret;
    
    data->buffer = buffer;
    data->is_writing = 0;
    data->show_info = show_info;
    
    if (data->show_info)
        printf("\nReading: ");
    
    /* reset transfer mode register */
    SDHC_XFR_MODE = 0;
    
    /* reset block count register */
    SDHC_BLK_CNT = 0;
    
    /* check if card is in transfer state */
    if (card_info.current_state != SDHC_STATE_TRAN)
    {
        /* select the card if card is in idle state */
        if (card_info.current_state == SDHC_STATE_STBY)
        {
            /* select the card */
            mmc_send_cmd(SDHC_CMD_SELECT_DESELECT_CARD, card_info.rca << 16, 0, data);
            
            /* change card state */
            card_info.current_state = SDHC_STATE_TRAN;
        }
        else
        {
            /* read card status register, CMD13 */
            mmc_send_cmd(SDHC_CMD_SEND_STATUS, card_info.rca << 16, 0, data);
            
            /* update card state */
            card_info.current_state = GET_FIELD(data->resp[0], SDHC_STS_CURRENT_STATE);
            
            PRINTD("card current state=%d\n", card_info.current_state);
        }
        
        switch (card_info.current_state)
        {
            case SDHC_STATE_PRG:
                /* delay 10ms */
                udelay(10000);
                card_info.current_state = SDHC_STATE_TRAN;
                break;
            case SDHC_STATE_TRAN:
                break;
            default:
                printf("card state error %d\n", card_info.current_state);
                return CARD_ERROR_PROTOCOL;
        }
    }
    
    /* set block size */
    mmc_send_cmd(SDHC_CMD_SET_BLOCKLEN, 512, 0, data);
    
    /* set as multi block */
    SET_BIT(SDHC_XFR_MODE, MULTI_BLK_SEL_P);
    
    /* set data transfer direction to read */
    SET_BIT(SDHC_XFR_MODE, DATA_XFR_DIR_SEL_P);
    
    /* enable auto CMD12 */
    SET_BIT(SDHC_XFR_MODE, AUTO_CMD12_EN_P);
    
    /* enable block count */
    SET_BIT(SDHC_XFR_MODE, BLK_CNT_EN_P);
    
    /* enable DMA */
    SET_BIT(SDHC_XFR_MODE, BLK_DMA_EN_P);
    
    while(1)
    {
        /* set block count register, block count 65535 */
        if (blocks > 0xFFFF)
        {
            PRINTD("blocks=%lx > 0xFFFF\n", blocks);
            
            /* keep handling blocks information */
            data->blocks = 0xFFFF;
            
            /* set block count register */
            SDHC_BLK_CNT = 0xFFFF;
            
            /* set system address register */
            SDHC_DMA_ADD = (UINT32) data->buffer;
            
            /* send read multi-block command */
            ret = mmc_send_cmd(SDHC_CMD_READ_MULTIPLE_BLOCK, offset, 0, data);
            
            if (ret)
                return ret;
            
            /* minus the handled blocks */
            blocks = blocks - 0xFFFF;
            
            /* adjust offset, block_count*block_size = 65535*512 = 33553920 */
            offset = offset + 33553920;
            
            /* adust buffer pointer */
            data->buffer = buffer + 33553920;
        }
        else
        {
            PRINTD("blocks=%lx\n", blocks);
            
            /* keep handling blocks information */
            data->blocks = blocks;
            
            /* set block count register */
            SDHC_BLK_CNT = blocks;
            
            /* set system address register */
            SDHC_DMA_ADD = (UINT32) data->buffer;
            
            /* send read multi-block command */
            ret = mmc_send_cmd(SDHC_CMD_READ_MULTIPLE_BLOCK, offset, 0, data);
            
            if (ret)
                return ret;
            
            break;
        }
    }
    
#if 0
    for (i = 0; i < blocks; i++)
    {
        mmc_send_cmd(SDHC_CMD_READ_SINGLE_BLOCK, offset, 0, data);
        offset = offset + 512;
        data->buffer = data->buffer + 512;
        
        /* print # every 64 Kbytes */
        if (hold_idx >> 7)
        {
            if (print_idx >> 6)
            {
                printf("\n         ");
                print_idx = 0;
            }
            else
            {
                printf("#");
                print_idx++;
            }
            
            hold_idx = 0;
        }
        else
        {
            hold_idx++;
        }
    }
#endif
    
    return 0;
}


static int mmc_block_write(struct mmc_data *data,
                           UINT32 offset,
                           UINT8 *buffer,
                           UINT32 blocks,
                           UINT8 show_info)
{
    PRINTD("mmc_block_write\n");
    
    int ret;
    
    data->buffer = buffer;
    data->is_writing = 1;
    data->show_info = show_info;
    
    if (data->show_info)
        printf("\nWriting: ");
    
    /* reset transfer mode register */
    SDHC_XFR_MODE = 0;
    
    /* reset block count register */
    SDHC_BLK_CNT = 0;
    
    /* check if card is in transfer state */
    if (card_info.current_state != SDHC_STATE_TRAN)
    {
        /* select the card if card is in standby state */
        if (card_info.current_state == SDHC_STATE_STBY)
        {
            /* select the card */
            mmc_send_cmd(SDHC_CMD_SELECT_DESELECT_CARD, card_info.rca << 16, 0, data);
            
            /* change card state */
            card_info.current_state = SDHC_STATE_TRAN;
        }
        else
        {
            /* read card status register, CMD13 */
            mmc_send_cmd(SDHC_CMD_SEND_STATUS, card_info.rca << 16, 0, data);
            
            /* update card state */
            card_info.current_state = GET_FIELD(data->resp[0], SDHC_STS_CURRENT_STATE);
            
            PRINTD("card current state=%d\n", card_info.current_state);
        }
        
        switch (card_info.current_state)
        {
            case SDHC_STATE_PRG:
                /* delay 10ms */
                udelay(10000);
                card_info.current_state = SDHC_STATE_TRAN;
                break;
            case SDHC_STATE_TRAN:
                break;
            default:
                printf("card state error %d\n", card_info.current_state);
                return CARD_ERROR_PROTOCOL;
        }
    }
    
    /* set block size */
    mmc_send_cmd(SDHC_CMD_SET_BLOCKLEN, 512, 0, data);
    
    /* set as multi block */
    SET_BIT(SDHC_XFR_MODE, MULTI_BLK_SEL_P);
    
    /* set data transfer direction to write */
    CLEAR_BIT(SDHC_XFR_MODE, DATA_XFR_DIR_SEL_P);
    
    /* enable auto CMD12 */
    SET_BIT(SDHC_XFR_MODE, AUTO_CMD12_EN_P);
    
    /* enable block count */
    SET_BIT(SDHC_XFR_MODE, BLK_CNT_EN_P);
    
    /* enable DMA */
    SET_BIT(SDHC_XFR_MODE, BLK_DMA_EN_P);
    
    while(1)
    {
        /* set block count register, block count 65535 */
        if (blocks > 0xFFFF)
        {
            PRINTD("blocks=%lx > 0xFFFF\n", blocks);
            
            /* keep handling blocks information */
            data->blocks = 0xFFFF;
            
            /* set block count register */
            SDHC_BLK_CNT = 0xFFFF;
            
            /* set system address register */
            SDHC_DMA_ADD = (UINT32) data->buffer;
            
            /* send write multi-block command */
            ret = mmc_send_cmd(SDHC_CMD_WRITE_MULTIPLE_BLOCK, offset, 0, data);
            
            if (ret)
                return ret;
            
            /* minus the handled blocks */
            blocks = blocks - 0xFFFF;
            
            /* adjust offset, block_count*block_size = 65535*512 = 33553920 */
            offset = offset + 33553920;
            
            /* adust buffer pointer */
            data->buffer = buffer + 33553920;
            
            /* change card state */
            card_info.current_state = SDHC_STATE_PRG;
        }
        else
        {
            PRINTD("blocks=%lx\n", blocks);
            
            /* keep handling blocks information */
            data->blocks = blocks;
            
            /* set block count register */
            SDHC_BLK_CNT = blocks;
            
            /* set system address register */
            SDHC_DMA_ADD = (UINT32) data->buffer;
            
            /* send write multi-block command */
            ret = mmc_send_cmd(SDHC_CMD_WRITE_MULTIPLE_BLOCK, offset, 0, data);
            
            if (ret)
                return ret;
            
            /* change card state */
            card_info.current_state = SDHC_STATE_PRG;
            
            break;
        }
    }
    
#if 0
    for (i = 0; i < blocks; i++)
    {
        mmc_send_cmd(SDHC_CMD_WRITE_SINGLE_BLOCK, offset, 0, data);
        offset = offset + 512;
        data->buffer = data->buffer + 512;
        
        /* print # every 64 Kbytes */
        if (hold_idx >> 7)
        {
            if (print_idx >> 6)
            {
                printf("\n         ");
                print_idx = 0;
            }
            else
            {
                printf("#");
                print_idx++;
            }
            
            hold_idx = 0;
        }
        else
        {
            hold_idx++;
        }
    }
#endif
    
    return 0;
}


/* block erase */
unsigned long mmc_berase(int dev, 
                         unsigned long start,   /* start block */
                         lbaint_t blkcnt)
{
    struct mmc_data data;
    UINT32 offset;
    UINT32 retry;
    int ret;
    
    PRINTD("mmc_berase\n");
    
    if (blkcnt == 0)
        return 0;
    
    /* check if card/host is initiated */
    if (card_info.current_state == SDHC_STATE_IDLE)
    {
        printf("*** card is not initiated ***\n");
        return 0;
    }
    
    /* check if card is in transfer state */
    if (card_info.current_state != SDHC_STATE_TRAN)
    {
        /* select the card if card is in standby state */
        if (card_info.current_state == SDHC_STATE_STBY)
        {
            /* select the card */
            mmc_send_cmd(SDHC_CMD_SELECT_DESELECT_CARD, card_info.rca << 16, 0, &data);
            
            /* change card state */
            card_info.current_state = SDHC_STATE_TRAN;
        }
        else
        {
            for (retry = 0; retry < 500; retry++)
            {
                /* read card status register, CMD13 */
                mmc_send_cmd(SDHC_CMD_SEND_STATUS, card_info.rca << 16, 0, &data);
                
                /* update card state */
                card_info.current_state = GET_FIELD(data.resp[0], SDHC_STS_CURRENT_STATE);
                
                PRINTD("card current state=%d\n", card_info.current_state);
                
                switch (card_info.current_state)
                {
                    case SDHC_STATE_PRG:
                        /* delay 10ms */
                        udelay(10000);
                        card_info.current_state = SDHC_STATE_TRAN;
                        
                        PRINTD("detect SDHC_STATE_PRG\n");
                        break;
                    case SDHC_STATE_TRAN:
                        retry = 500;
                        break;
                    default:
                        printf("card state error %d\n", card_info.current_state);
                        return CARD_ERROR_PROTOCOL;
                }
            }
        }
    }
    
    /* setup start address */
    offset = start << 9;
    
    PRINTD("start address=%lx\n", offset);
    
    /* send erase start address command */
    mmc_send_cmd(SDHC_CMD_ERASE_WR_BLK_START, offset, 0, &data);
    
    /* setup end address */
    offset = (start + blkcnt - 1) << 9;
    
    PRINTD("end address=%lx\n", offset);
    
    /* send erase end address command */
    mmc_send_cmd(SDHC_CMD_ERASE_WR_BLK_END, offset, 0, &data);
    
    /* send erase command */
    ret = mmc_send_cmd(SDHC_CMD_ERASE, 0, 0, &data);
    
    if (ret)
        return 0;
    
    /* change card state */
    card_info.current_state = SDHC_STATE_PRG;
    
    return blkcnt;
}


/* block read, mandatory function to register as a block device */
unsigned long mmc_bread(int dev, 
                        unsigned long start,     /* start block */
                        lbaint_t blkcnt,
                        unsigned long *buffer)
{
    struct mmc_data data;
    
    PRINTD("mmc_bread\n");
    
#ifdef MMC_DEBUG
    printf("dev=%d, start=%lx, blkcnt=%lx\n", dev, start, blkcnt);
#endif
    
    if (blkcnt == 0)
        return 0;
    
    /* check if card/host is initiated */
    if (card_info.current_state == SDHC_STATE_IDLE)
    {
        printf("*** card is not initiated ***\n");
        return 0;
    }
    
    /* read block by block */
    if (mmc_block_read(&data, (start * 512), (UINT8 *) buffer, blkcnt, 0))
    {
        printf("*** fail to read ***\n");
        return 0;
    }
    
    return blkcnt;
}


/* block write */
unsigned long mmc_bwrite(int dev, 
                         unsigned long start,    /* start block */
                         lbaint_t blkcnt,
                         unsigned long *buffer)
{
    struct mmc_data data;
    
    PRINTD("mmc_bwrite\n");
    
#ifdef MMC_DEBUG    
    printf("dev=%d, start=%lx, blkcnt=%lx\n", dev, start, blkcnt);
#endif
    
    if (blkcnt == 0)
        return 0;
    
    /* check if card/host is initiated */
    if (card_info.current_state == SDHC_STATE_IDLE)
    {
        printf("*** card is not initiated ***\n");
        return 0;
    }
    
    /* write block by block */
    if (mmc_block_write(&data, (start * 512), (UINT8 *) buffer, blkcnt, 0))
    {
        printf("*** fail to write ***\n");
        return 0;
    }
    
    return blkcnt;
}


static int mmc_block_dev_init(int dev)
{
    /* PRINTD("mmc_block_dev_init\n"); */
    
    mmc_dev.if_type = IF_TYPE_MMC;
    mmc_dev.part_type = PART_TYPE_DOS;
    mmc_dev.dev = 0;
    mmc_dev.lun = 0;
    mmc_dev.type = 0;           /* device type, 0: DEV_TYPE_HARDDISK */
    mmc_dev.blksz = 512;        /* block size */
    mmc_dev.lba = 0x200000;      /* number of blocks, 1024*1024*1024/512 */
    sprintf((char*)mmc_dev.vendor, "Unknown vendor");
    sprintf((char*)mmc_dev.product, "Unknown product");
    sprintf((char*)mmc_dev.revision, "N/A");
    mmc_dev.removable = 1;      /* 1: removable */
    mmc_dev.block_read = mmc_bread;
    
    /* fat_register_device(&mmc_dev, 1); */
    
    return 0;
}


/* freq unit is KHz */
static int mmc_set_clock(UINT32 freq)
{
    UINT32 base_clock;
    UINT32 selected_clock;
    UINT8 clock_divider = 0;
    int i = 0;
    
    PRINTD("mmc_set_clock\n");
    
    /* disable SD clock */
    CLEAR_BIT(SDHC_CLK_CTRL, SD_CLK_EN_P);
    
    if (freq != 0)
    {
        /* disable SD clock */
        CLEAR_BIT(SDHC_CLK_CTRL, SD_CLK_EN_P);
        
        /* change the SDSD bit accroding to the card type */
        if (card_info.card_type == SD_CARD)
        {
            /* enable SDIO interface sampling delay */
            SET_BIT(INTCR2, INTCR2_SDSD_BIT);
        }
        else if (card_info.card_type == MMC_CARD)
        {
            /* disable SDIO interface sampling delay */
            CLEAR_BIT(INTCR2, INTCR2_SDSD_BIT);
        }
        else
        {
            /* disable SDIO interface sampling delay */
            CLEAR_BIT(INTCR2, INTCR2_SDSD_BIT);
        }
        
        base_clock = GET_FIELD(SDHC_CAPABILITIES, SDHC_CLOCK_BASE) * 1000;
        selected_clock = base_clock;
        
        while(selected_clock > freq)
        {
            if (i >= 8)
            {
                /* choose the lowest frequency */
                i = 8;
                break;
            }
            
            /* clock divided by 2 */
            selected_clock = selected_clock >> 1;
            
            i++;
        }
        
        if (i == 0)
        {
            clock_divider = 0;
        }
        else
        {
            clock_divider = 1 << (i-1);
        }
        
        /* set SDCLK frequency select */
        SET_FIELD(SDHC_CLK_CTRL, SDCLK_FREQ_SEL, clock_divider);    /* 48MHz / 256 */
        
        /* delay 1 ms */
        udelay(1000);
        
        /* wait internal clcok stable */
        while(!READ_BIT(SDHC_CLK_CTRL, INTR_CLK_STABLE_P));
        
        /* enable SD clock */
        SET_BIT(SDHC_CLK_CTRL, SD_CLK_EN_P);
        
        /* keep clock frequency information */
        card_info.selected_clock = selected_clock;
        
        PRINTD("base_clock=%ld, want=%ld, final=%ld, clock_divider=%lx, i=%d\n", base_clock, freq, selected_clock, clock_divider, i);
    }
    
    return 0;
}


int mmc_cal_card_size(UINT32 *size)
{
    UINT8 version;
    
    if (CARD_IS_MMC(card_info.card_type))
    {
        /* determine MMC spec version */
        version = CSD_MMC_SPEC_VERS;
    }
    else
    {
        /* determine SD spec version */
        version = SCR_SD_SPEC;
    }
    
#if IS_ENABLED(LUN_SD)    
    /* In SD cards, field structures of the CSD register are different depend 
    on the Physical Specification Version and Card Capacity */
    if (CARD_IS_SD(card_info.card_type))
    {
#ifdef SUPPORT_SD_2_0
        /*The CSD_STRUCTURE field in the CSD register indicates its structure version */
        if (CSD_STRUCTURE == SD_CSD_VER_2)
        {
            /* In case of SD ver 2.00 or later High Capacity:
            Calculate Total number of sectors: 
            TotalSectorCount = MEM_CAPACITY / SDHC_SECTOR_SIZE
            where
            MEM_CAPACITY = BLOCKNR * BLOCK_LEN
            BLOCKNR = (C_SIZE+1)
            BLOCK_LEN = 512 */
            *size = ((CSD_SD_HCS_C_SIZE + 1) 
                     * SDHC_HCS_BLOCK_LENGTH 
                     / SDHC_SECTOR_SIZE);
            
            PRINTD("SDHC card size=%lx\n", *size);
            
            PRINTD("CSD_SD_HCS_C_SIZE=%lx, SDHC_HCS_BLOCK_LENGTH=%lx, SECTOR_SIZE=%lx\n",
                   CSD_SD_HCS_C_SIZE,
                   SDHC_HCS_BLOCK_LENGTH,
                   SDHC_SECTOR_SIZE);
            
            return 0;
        }
#endif
    }
#endif
    
    /* In case of MMC/SD Standard Capacity:
    Calculate Total number of sectors: 
    TotalSectorCount = MEM_CAPACITY / SDHC_SECTOR_SIZE
    where
    MEM_CAPACITY = BLOCKNR * BLOCK_LEN
    BLOCKNR = (C_SIZE+1) * MULT
    MULT = 2^(C_SIZE_MULT+2)     (C_SIZE_MULT < 8)
    BLOCK_LEN = 2^READ_BL_LEN    (9 <= READ_BL_LEN =< 11) */
    *size = ((CSD_C_SIZE + 1) 
             * ((UINT32) 1 << (CSD_C_SIZE_MULT + 2)) 
             * ((UINT32) 1 << (CSD_READ_BL_LEN))
             / SDHC_SECTOR_SIZE);
    
    PRINTD("SD/MMC card size=%lx\n", *size);
    PRINTD("CSD_C_SIZE=%lx, CSD_C_SIZE_MULT=%lx, CSD_READ_BL_LEN=%lx, SECTOR_SIZE=%lx\n",
           CSD_C_SIZE,
           CSD_C_SIZE_MULT,
           CSD_READ_BL_LEN,
           SDHC_SECTOR_SIZE);
    
    return 0;
}


static int mmc_card_detect(int dev)
{
    UINT8 buffer[512] __attribute__ ((aligned(32)));
    struct mmc_data data;
    int ret;
    int retry;
    UINT32 card_freq;
    
    PRINTD("mmc_card_detect\n");
    
    /* set buffer */
    data.buffer = &buffer[0];
    
    /* wait card state stable */
    while(!READ_BIT(SDHC_PRSNT_STATE, CARD_STABLE_P));
    
    /* check card inserted */
    switch(dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
            if (READ_BIT(SDHC_PRSNT_STATE, CARD_INSERT_P))
            {
                card_info.card_inserted = 1;
            }
            else
            {
                card_info.card_inserted = 0;
            }
            
            break;
            
        case FWU_DEV_MASER:
            /* 0 -> present */
            if (READ_BIT(GP6DIN, GPI111))
            {
                card_info.card_inserted = 0;
            }
            else
            {
                card_info.card_inserted = 1;
            }
            
            break;
            
        default:
            break;
    }
    
    /* check write protect switch pin */
    switch(dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
#endif
        case FWU_DEV_AMEA:
        case FWU_DEV_AMEA_NO_MUX:
            if (READ_BIT(SDHC_PRSNT_STATE, WP_PIN_LVL_P))
            {
                card_info.write_protected = 1;
            }
            else
            {
                card_info.write_protected = 0;
            }
            
            break;
            
        case FWU_DEV_MASER:
            card_info.write_protected = 0;
            
            break;
            
        default:
            break;
    }
    
    /* The RCA to be used in Idle_state shall be the card default RCA = 0x0000 */
    card_info.rca = 0x0000;
    card_info.ocr = OCR_DEFAULT;
    card_info.card_type = SD_CARD;
    
    PRINTD("dev=%d, card_inserted=%d, write_protected=%d\n", 
           dev,
           card_info.card_inserted, 
           card_info.write_protected);
    
    if (card_info.card_inserted)
    {
        PRINTD("card is inserted\n");
        
        /* reset card, CMD0 */
        ret = mmc_send_cmd(SDHC_CMD_GO_IDLE_STATE, 0, 0, &data);
        
        if (ret)
        {
            return ret;
        }
        
        /* change card state */
        card_info.current_state = SDHC_STATE_IDLE;
        
        /* determine the card type */
        
        PRINTD("is it a SD card\n");
        
        /* send SD command to get OCR, ACMD41 */
        ret = mmc_send_cmd(SDHC_CMD_APP_CMD, card_info.rca << 16, 10, &data);
        
        /* it is a SD card */
        if (ret == 0)
        {
            PRINTD("it is a SD card\n");
            
            mmc_send_cmd(SDHC_ACMD_SD_SEND_OP_COND, card_info.ocr, 10, &data);
            
            /* reset retry count */
            retry = 0;
            
            while(1)
            {
                /* card power up status bit */
                if (data.resp[0] & OCR_BUSY)
                {
                    card_info.ocr = data.resp[0];
                    break;
                }
                else
                {
                    /* delay 5 ms to retry */
                    udelay(5000);
                }
                
                if (retry > 200)
                {
                    printf("*** get OCR timeout ***\n");
                    return CARD_ERROR_TIMEOUT;
                }
                
                retry ++;
                
                /* get OCR, ACMD41 */
                mmc_send_cmd(SDHC_CMD_APP_CMD, card_info.rca << 16, 10, &data);
                mmc_send_cmd(SDHC_ACMD_SD_SEND_OP_COND, card_info.ocr, 10, &data);
                
                PRINTD("retry=%d\n", retry);
            }
        }
        
        /* it might be a MMC card */
        else
        {
            PRINTD("is it a MMC card\n");
            
            card_info.card_type = MMC_CARD;
            
            /* change the clock setting accroding the type of card */
            mmc_set_clock(400);
            
            /* Assign to this card a Relative Card Address (RCA)
               The MMC Spec says:
               "Send CMD3 with a chosen RCA, with value greater than 1" */
            card_info.rca = 0x2;
            
            /* get OCR, CMD1 */
            ret = mmc_send_cmd(SDHC_CMD_SEND_OP_COND, card_info.ocr, 10, &data);
            
            /* it is a MMC card */
            if (ret == 0)
            {
                PRINTD("it is a MMC card\n");
                
                /* reset retry count */
                retry = 0;
                
                while(1)
                {
                    /* card power up status bit */
                    if (data.resp[0] & OCR_BUSY)
                    {
                        card_info.ocr = data.resp[0];
                        break;
                    }
                    else
                    {
                        /* delay 5 ms to retry */
                        udelay(5000);
                    }
                    
                    if (retry > 200)
                    {
                        printf("*** get OCR timeout ***\n");
                        return CARD_ERROR_TIMEOUT;
                    }
                    
                    retry ++;
                    
                    /* get OCR, CMD1 */
                    mmc_send_cmd(SDHC_CMD_SEND_OP_COND, card_info.ocr, 10, &data);
                    
                    PRINTD("retry=%d\n", retry);
                }
            }
            else
            {
                card_info.card_type = UNKNOWN_CARD;
                
                printf("*** unknown card type ***\n");
                return CARD_ERROR_PROTOCOL;
            }
        }
        
        /* get CID, CMD2 */
        ret = mmc_send_cmd(SDHC_CMD_ALL_SEND_CID, 0, 0, &data);
        
        if (ret)
        {
            printf("*** get CID error ***\n");
            return CARD_ERROR_PROTOCOL;
        }
        
        *((UINT32 *) &card_info.cid[0]) = data.resp[0];
        *((UINT32 *) &card_info.cid[4]) = data.resp[1];
        *((UINT32 *) &card_info.cid[8]) = data.resp[2];
        *((UINT32 *) &card_info.cid[12]) = data.resp[3];
        
        /* change card state */
        card_info.current_state = SDHC_STATE_IDENT;
        
        if (card_info.card_type == SD_CARD)
        {
            /* get RCA, CMD3 */
            ret = mmc_send_cmd(SDHC_CMD_SEND_RELATIVE_ADDR, 0, 0, &data);
            
            if (ret)
            {
                printf("*** get RCA error ***\n");
                return CARD_ERROR_PROTOCOL;
            }
            
            card_info.rca = data.resp[0] >> 16;
        }
        else
        {
            /* assign relative address RCA, CMD3 */
            ret = mmc_send_cmd(SDHC_CMD_SEND_RELATIVE_ADDR, 
                               card_info.rca << 16, 0, &data);
            
            if (ret)
            {
                printf("*** set RCA error ***\n");
                return CARD_ERROR_PROTOCOL;
            }
        }
        
        /* change card state */
        card_info.current_state = SDHC_STATE_STBY;
        
        /* get CSD, CMD9 */
        ret = mmc_send_cmd(SDHC_CMD_SEND_CSD, card_info.rca << 16, 0, &data);
        
        if (ret)
        {
            printf("*** get CSD error ***\n");
            return CARD_ERROR_PROTOCOL;
        }
        
        *((UINT32 *) &card_info.csd[0]) = data.resp[0];
        *((UINT32 *) &card_info.csd[4]) = data.resp[1];
        *((UINT32 *) &card_info.csd[8]) = data.resp[2];
        *((UINT32 *) &card_info.csd[12]) = data.resp[3];
        
        PRINTD("CSD_TRAN_SPEED=%lx, CSD_TRAN_SPEED_UNIT=%d, CSD_TRAN_SPEED_TIME_VALUE=%d\n",
               CSD_TRAN_SPEED,
               GET_FIELD(CSD_TRAN_SPEED, CSD_TRAN_SPEED_UNIT),
               GET_FIELD(CSD_TRAN_SPEED, CSD_TRAN_SPEED_TIME_VALUE));
        
        /* get card max. data transfer rate */
        switch(GET_FIELD(CSD_TRAN_SPEED, CSD_TRAN_SPEED_TIME_VALUE))
        {
            case 0x1:
                card_freq = 10;
                break;
            case 0x2:
                card_freq = 12;
                break;
            case 0x3:
                card_freq = 13;
                break;
            case 0x4:
                card_freq = 15;
                break;
            case 0x5:
                card_freq = 20;
                break;
            case 0x6:
                PRINTD("TIME_VALUE 6\n");
                card_freq = 25;
                break;
            case 0x7:
                card_freq = 30;
                break;
            case 0x8:
                card_freq = 35;
                break;
            case 0x9:
                card_freq = 40;
                break;
            case 0xA:
                card_freq = 45;
                break;
            case 0xB:
                card_freq = 50;
                break;
            case 0xC:
                card_freq = 55;
                break;
            case 0xD:
                card_freq = 60;
                break;
            case 0xE:
                card_freq = 70;
                break;
            case 0xF:
                card_freq = 80;
                break;
            default:
                card_freq = 10;
        }
        
        switch(GET_FIELD(CSD_TRAN_SPEED, CSD_TRAN_SPEED_UNIT))
        {
            case 0:
                PRINTD("UNIT 0\n");
                card_freq = card_freq * 10;
                break;
            case 1:
                PRINTD("UNIT 1\n");
                card_freq = card_freq * 100;
                break;
            case 2:
                PRINTD("UNIT 2\n");
                card_freq = card_freq * 1000;
                break;
            case 3:
                PRINTD("UNIT 3\n");
                card_freq = card_freq * 10000;
                break;
            default:
                PRINTD("UNIT default\n");
                card_freq = card_freq * 10;
        }
        
        /* limit to 12 MHz to conform the timing requirement of SD spec */
        if (card_info.card_type == SD_CARD)
        {
            if (card_freq > 12000)
            {
                card_freq = 12000;
            }
        }

        /* Limit the freq at 12MHz becuase WPCM450 does not support MMC freq higher thatn 20MH and the MMC freqs which WPCM450 supported are 6MHz, 12MHz, 24MHz and 48MHz */
        if (card_info.card_type == MMC_CARD)
        {
            if(card_freq > 12000)
            {
                card_freq = 12000;
            }
        }


        /* change to card max transfer rate */
        mmc_set_clock(card_freq);
        
        /* get card size from CSD register */
        mmc_cal_card_size(&card_info.size);
        
        /* select the card to move to transfer state */
        mmc_send_cmd(SDHC_CMD_SELECT_DESELECT_CARD, 
                     card_info.rca << 16, 
                     0, 
                     &data);
        
        /* change card state */
        card_info.current_state = SDHC_STATE_TRAN;
        
#if 0
        /* set to 4-bit mode if it is a sd card */
        if (card_info.card_type == SD_CARD)
        {
            /* Since we use a Card Detection Switch on the connector for Card
            Detection (in-order to be compatible with MMC cards),
            disconnect the 50KOhm pull-up resistor */
            
            /* clear card detect, ACMD42, bit[0] 1->connect 0->disconnect */
            mmc_send_cmd(SDHC_CMD_APP_CMD, card_info.rca << 16, 0, &data);
            ret = mmc_send_cmd(SDHC_ACMD_SET_CLR_CARD_DETECT, 0, 0, &data);
            
            if (ret)
            {
                printf("*** disconnect card detect error ***\n");
                return CARD_ERROR_PROTOCOL;
            }
            
            /* change data transfer direction to read */
            SDHC_XFR_MODE = 0;
            SET_BIT(SDHC_XFR_MODE, DATA_XFR_DIR_SEL_P);
            
            /* set block size register, 8 bytes */
            SET_FIELD(SDHC_BLK_SIZE, XFR_BLK_SIZE, 8);
            
            /* get SCR, ACMD51 */
            mmc_send_cmd(SDHC_CMD_APP_CMD, card_info.rca << 16, 0, &data);
            ret = mmc_send_adtc_cmd(SDHC_ACMD_SEND_SCR, 0, 8, 0, &data);
            
            if (ret)
            {
                printf("*** get SCR error ***\n");
                return CARD_ERROR_PROTOCOL;
            }
            
            *((UINT32 *) &card_info.scr[0]) = *((UINT32 *) data.buffer);
            *((UINT32 *) &card_info.scr[4]) = *(((UINT32 *) data.buffer) + 1);
            
            PRINTD("scr=%02x %02x %02x %02x %02x %02x %02x %02x\n",
                   data.buffer[0],
                   data.buffer[1],
                   data.buffer[2],
                   data.buffer[3],
                   data.buffer[4],
                   data.buffer[5],
                   data.buffer[6],
                   data.buffer[7]
                  );
            
            /* change data transfer direction to write */
            SDHC_XFR_MODE = 0;
            
            /* set block size register, 512 bytes */
            SET_FIELD(SDHC_BLK_SIZE, XFR_BLK_SIZE, 0x200);
            
            /* check if sd card supports 4-bit mode */
            if (SCR_SD_BUS_WIDTHS & 0x04)
            {
                PRINTD("SD card supports 4-bit mode\n");
                
                /* set to 4-bit mode, ACMD6 */
                mmc_send_cmd(SDHC_CMD_APP_CMD, card_info.rca << 16, 0, &data);
                
                ret = mmc_send_cmd(SDHC_ACMD_SET_BUS_WIDTH, _4_BIT_MODE, 0, &data);
                
                if (ret)
                {
                    printf("*** get SCR error ***\n");
                    return CARD_ERROR_PROTOCOL;
                }
                else
                {
                    PRINTD("set host controller to 4-bit mode\n");
                    
                    /* set data transfer width to 4-bit mode */
                    SET_FIELD(SDHC_HOST_CTRL, DATA_XFR_WIDTH, _4_BIT_MODE);
                }
            }
            else
            {
                PRINTD("SD card doesn't support 4-bit mode\n");
            }
        }
#endif
    }
    else
    {
        /* fail to detect card type */
        return -1;
    }
    
    PRINTD("rca=%04x, cid=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
           card_info.rca,
           card_info.cid[0],
           card_info.cid[1],
           card_info.cid[2],
           card_info.cid[3],
           card_info.cid[4],
           card_info.cid[5],
           card_info.cid[6],
           card_info.cid[7],
           card_info.cid[8],
           card_info.cid[9],
           card_info.cid[10],
           card_info.cid[11],
           card_info.cid[12],
           card_info.cid[13],
           card_info.cid[14],
           card_info.cid[15]
          );
    
    PRINTD("csd=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
           card_info.csd[0],
           card_info.csd[1],
           card_info.csd[2],
           card_info.csd[3],
           card_info.csd[4],
           card_info.csd[5],
           card_info.csd[6],
           card_info.csd[7],
           card_info.csd[8],
           card_info.csd[9],
           card_info.csd[10],
           card_info.csd[11],
           card_info.csd[12],
           card_info.csd[13],
           card_info.csd[14],
           card_info.csd[15]
          );
    
    PRINTD("scr=%02x %02x %02x %02x %02x %02x %02x %02x\n",
           card_info.scr[0],
           card_info.scr[1],
           card_info.scr[2],
           card_info.scr[3],
           card_info.scr[4],
           card_info.scr[5],
           card_info.scr[6],
           card_info.scr[7]
          );
    
    return 0;
}


/* initialize the hardware */
static int mmc_hw_init(int dev)
{
    PRINTD("mmc_hw_init\n");
    
    /* select SD card interface */
    SET_BIT(MFSEL1, MF_SDIOSEL);
    
    /* enable SD interface sampling delay for SD standard timing */
    SET_BIT(INTCR2, INTCR2_SDSD_BIT);
    
    /* enable SDIO clock, CLKEN/SDIO=1 */
    SET_BIT(CLK_EN, CLK_SDIO_BIT);
    
    /* initiate SD/MMC socket */
    switch(dev)
    {
#ifdef CONFIG_WPCM450_SVB
        case FWU_DEV_EVB:
            /* select GPIO63 */
            CLEAR_BIT(MFSEL1, MF_HSP1SEL_BIT);
            
            /* For EVB, GPIO63 = low ¡V selects SD socket */
            /* Switch clock MUX: set GPIO63 to 0 for SDCLK */
            CLEAR_BIT(GP3DOUT, GPIO63);
            
            /* output buffer enable */
            SET_BIT(GP3CFG0, GPIO63);
            
            break;
#endif
            
        case FWU_DEV_MASER:
            /* set GPIO38 to high to select AMEA SD */
            SET_BIT(GP2DOUT, GPIO38);
            
            /* output buffer enable */
            SET_BIT(GP2CFG0, GPIO38);
            
            
            /* set GPIO63 to low to block power to AMEA SD connector */
            CLEAR_BIT(GP3DOUT, GPIO63);
            
            /* set GPIO59 to low to apply power to MASER DC SD connector */
            /* CLEAR_BIT(GP3DOUT, GPIO59); */
            
            break;
            
        case FWU_DEV_AMEA:
            /* set GPIO38 to low to select MASER DC eMMC */
            CLEAR_BIT(GP2DOUT, GPIO38);
            
            /* output buffer enable */
            SET_BIT(GP2CFG0, GPIO38);
            
            /* set GPIO59 to high to block power to MASER DC SD connector */
            /* SET_BIT(GP3DOUT, GPIO59); */
            
            /* set GPIO63 to high to apply power to AMEA SD connector */
            SET_BIT(GP3DOUT, GPIO63);
            
            break;
            
        case FWU_DEV_AMEA_NO_MUX:
            
            /* set GPIO59 to high to block power to MASER DC SD connector */
            /* SET_BIT(GP3DOUT, GPIO59); */
            
            /* set GPIO63 to high to apply power to AMEA SD connector */
            SET_BIT(GP3DOUT, GPIO63);
            
            break;
            
        default:
            break;
    }
    
    PRINTD("GP2CFG0=%lx, GP3CFG0=%lx, GP7CFG0=%lx\n",
           GP2CFG0, GP3CFG0, GP7CFG0);
    
    PRINTD("GP2DOUT=%lx, GP3DOUT=%lx, GP7DOUT=%lx\n",
           GP2DOUT, GP3DOUT, GP7DOUT);
    
    PRINTD("GP2DIN=%lx, GP3DIN=%lx, GP6DIN=%lx, GP7DIN=%lx\n",
           GP2DIN, GP3DIN, GP6DIN, GP7DIN);
    
    /* SD Card Interface - Set/clear Software Reset Control Bit */
    SET_BIT(CLK_IPSRST, CLK_IPSRST_SDIO_BIT);
    udelay(100000);
    CLEAR_BIT(CLK_IPSRST, CLK_IPSRST_SDIO_BIT);
    
    /* software reset for all */
    SET_BIT(SDHC_SW_RST, SW_RST_ALL_P);
    
    /* wait until host controller is ready */
    while(SDHC_SW_RST);
    
    /* select SD bus voltage */
    SET_FIELD(SDHC_PWR_CTRL, VOLAGE_SEL, 0x07);
    
    /* enable SD bus power */
    SET_BIT(SDHC_PWR_CTRL, BUS_PWR_P);  /* power on SD bus power */
    
    /* enable internal clock */
    SET_BIT(SDHC_CLK_CTRL, INTR_CLK_EN_P);
    
    /* set data timeout counter value */
    SET_FIELD(SDHC_TO_CTRL, DATA_TO_CNT, 0x0E);    /* TMCLK x 2^27 */
    
    /* delay as least 74 SD clocks, maximum 1 ms */
    /* udelay(10000); */
    
    /* set clock */
    mmc_set_clock(400);
    
    /* delay 1 ms */
    udelay(1000);
    
    /* delay as least 74 SD clocks, maximum 1 ms */
    udelay(1000);
    
    /* enable normal interrupt status */
    SDHC_NRML_INT_STS_EN = 0x000B;
    /*
    SET_BIT(SDHC_NRML_INT_STS_EN, CMD_COMPLETE_STS_EN_P);
    SET_BIT(SDHC_NRML_INT_STS_EN, XFR_COMPLETE_STS_EN_P);
    SET_BIT(SDHC_NRML_INT_STS_EN, BUF_WR_READY_STS_EN_P);
    SET_BIT(SDHC_NRML_INT_STS_EN, BUF_RD_READY_STS_EN_P);
    SET_BIT(SDHC_NRML_INT_STS_EN, CARD_INSERT_STS_EN_P);
    SET_BIT(SDHC_NRML_INT_STS_EN, CARD_REMOVE_STS_EN_P);
    SET_BIT(SDHC_NRML_INT_STS_EN, XFR_COMPLETE_STS_EN_P);
    SET_BIT(SDHC_NRML_INT_STS_EN, CMD_COMPLETE_STS_EN_P);
    */
    
    /* enable erro interrupt status */
    SDHC_ERR_INT_STS_EN = 0x01FF;
    /*
    AUTO_CMD12_ERR_STS_EN_P
    CUR_LIM_ERR_STS_EN_P
    DATA_ENDBIT_ERR_STS_EN_P
    DATA_CRC_ERR_STS_EN_P
    DATA_TO_ERR_STS_EN_P
    CMD_IDX_ERR_STS_EN_P
    CMD_ENDBIT_ERR_STS_EN_P
    CMD_CRC_ERR_STS_EN_P
    CMD_TO_ERR_STS_EN_P
    */
    
    /* set block size register, 512 bytes */
    SET_FIELD(SDHC_BLK_SIZE, XFR_BLK_SIZE, 0x200);
    
    /* set host SDMA buffer boundary, 512K bytes */
    SET_FIELD(SDHC_BLK_SIZE, HOST_SDMA_BUF_BOUNDARY, 0x07);
    
    return 0;
}


static int mmc_dump_card_info(int dev)
{
    if (!card_info.card_inserted)
    {
        printf("*** card is not inserted ***\n");
        
        return 1;
    }
    
    switch (card_info.card_type)
    {
        case SD_CARD:
            printf("SD:    ");
            break;
        case MMC_CARD:
            printf("MMC:   ");
            break;
        default:
            printf("Unknown:");
    }
    
    if (card_info.selected_clock > 1000)
    {
        printf("%ld MHz", (card_info.selected_clock / 1000));
    }
    else
    {
        printf("%ld KHz", card_info.selected_clock);
    }
    
    PRINTD("RCA : %04x\n", card_info.rca);
    
    PRINTD("OCR : %08x\n", card_info.ocr);
    
    PRINTD("CID : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
           card_info.cid[0],
           card_info.cid[1],
           card_info.cid[2],
           card_info.cid[3],
           card_info.cid[4],
           card_info.cid[5],
           card_info.cid[6],
           card_info.cid[7],
           card_info.cid[8],
           card_info.cid[9],
           card_info.cid[10],
           card_info.cid[11],
           card_info.cid[12],
           card_info.cid[13],
           card_info.cid[14],
           card_info.cid[15]
          );
    
    PRINTD("CSD : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
           card_info.csd[0],
           card_info.csd[1],
           card_info.csd[2],
           card_info.csd[3],
           card_info.csd[4],
           card_info.csd[5],
           card_info.csd[6],
           card_info.csd[7],
           card_info.csd[8],
           card_info.csd[9],
           card_info.csd[10],
           card_info.csd[11],
           card_info.csd[12],
           card_info.csd[13],
           card_info.csd[14],
           card_info.csd[15]
          );
    
    PRINTD("SCR : %02x %02x %02x %02x %02x %02x %02x %02x\n",
           card_info.scr[0],
           card_info.scr[1],
           card_info.scr[2],
           card_info.scr[3],
           card_info.scr[4],
           card_info.scr[5],
           card_info.scr[6],
           card_info.scr[7]
          );
    
    /* print card size */
    if (card_info.size < (1024 / 512))
    {
        /* (size * 512) */
        printf("%6ld B\n", card_info.size << 9);
    }
    else if (card_info.size < (1024 * 1024 / 512))
    {
        /* (size * 512 / 1024) */
        printf("%6ld KB\n", (card_info.size >> 1));
    }
    else if (card_info.size < (1024 * 1024 * 1024 / 512))
    {
        /* (size * 512 / 1024 / 1024) */
        printf("%6ld MB\n", (card_info.size >> 1 >> 10));
    }
    else
    {
        /* (size * 512 / 1024 / 1024 / 1024) */
        printf("%6ld GB\n", (card_info.size >> 1 >> 10 >> 10));
    }
    
    return 0;   
}


int mmc_get_dev_size(int dev, 
                     UINT32 *size)
{
    /* card size in blocks */
    *size = card_info.size;
    
    return 0;
}


/* mandatory functions for u-boot mmc sub-system */

block_dev_desc_t * mmc_get_dev(int dev)
{
    PRINTD("mmc_get_dev\n");
    
    return (dev == 0) ? &mmc_dev : NULL;
}


int mmc_init(int verbose)
{
    PRINTD("mmc_init, dev=%d\n", verbose);
    
    int ret;
    
    /* check the version of chip */
    if (PDID >= PDID_Z2)
    {
        /* mmc hardware initiate */
        mmc_hw_init(verbose);
        
        /* card identification */
        ret = mmc_card_detect(verbose);
        
        if (ret)
            return ret;
        
        /* register as a block device */
        mmc_block_dev_init(verbose);
        
        /* dump card related information */
        mmc_dump_card_info(verbose);
        
        /* set flag to indicate this module is initiated as least once */
        mmc_init_state = 1;
    }
    else
    {
        printf("*** SD/MMC is not supported in this chip ***\n");
        return -1;
    }
    
    return 0;
}


int mmc_read(ulong src, uchar *dst, int size)
{
    struct mmc_data data;
    UINT32 offset;
    UINT32 blocks;
    
    PRINTD("mmc_read\n");
    
    PRINTD("src=%lx, dst=%p\n", src, dst);
    
    /* check if card/host is initiated */
    if (card_info.current_state == SDHC_STATE_IDLE)
    {
        printf("*** card is not initiated ***\n");
        return -1;
    }
    
    /* offset in SD/MMC */
    offset = src - CFG_MMC_BASE;
    
    /* src address should be dword aligned */
    if (((UINT32) src & 0x1FF) != 0)
    {
        printf("Source address is not at block boundary!\n");
        return -1;
    }
    
    /* dst address should be block aligned */
    if (((UINT32) dst & 0x1FF) != 0)
    {
        printf("Target address is not at block boundary!\n");
        return -1;
    }
    
    /* size should be block aligned */
    if ((size & 0x1FF) != 0)
    {
        printf("Count is not at block boundary!\n");
        return -1;
    }
    
    /* calculate blocks */
    if ((size & 0x1FF) != 0)
    {
        /* size is not block aligned */
        blocks = (size >> 9) + 1;
    }
    else
    {
        /* size is block aligned */
        blocks = size >> 9;
    }
    
    PRINTD("offset=%lx, size=%lx, blocks=%lx\n", offset, size, blocks);
    
    /* read block by block */
    mmc_block_read(&data, offset, (UINT8 *) dst, blocks, 1);
    
    return 0;
}


int mmc_write(uchar *src, ulong dst, int size)
{
    struct mmc_data data;
    UINT32 offset;
    UINT32 blocks;
    
    PRINTD("mmc_write\n");
    
    PRINTD("src=%p, dst=%lx\n", src, dst);
    
    /* check if card/host is initiated */
    if (card_info.current_state == SDHC_STATE_IDLE)
    {
        printf("*** card is not initiated ***\n");
        return -1;
    }
    
    /* offset in SD/MMC */
    offset = dst - CFG_MMC_BASE;
    
    /* src address should be block aligned */
    if (((UINT32) src & 0x1FF) != 0)
    {
        printf("Source address is not at block boundary!\n");
        return -1;
    }
    
    /* dst address should be dword aligned */
    if (((UINT32) dst & 0x1FF) != 0)
    {
        printf("Target address is not at block boundary!\n");
        return -1;
    }
    
    /* size should be block aligned */
    if ((size & 0x1FF) != 0)
    {
        printf("Count is not at block boundary!\n");
        return -1;
    }
    
    /* calculate blocks */
    if ((size & 0x1FF) != 0)
    {
        /* size is not block aligned */
        blocks = (size >> 9) + 1;
    }
    else
    {
        /* size is block aligned */
        blocks = size >> 9;
    }
    
    /* write block by block */
    mmc_block_write(&data, offset, (UINT8 *) src, blocks, 1);
    
    PRINTD("offset=%lx\n", offset);
    
    return 0;
}


int mmc2info(ulong addr)
{
    PRINTD("mmc2info\n");
    
    /* mapping to ram address */
    if (addr >= CFG_MMC_BASE && addr < CFG_MMC_BASE + CFG_MMC_MAPPING_SIZE) 
    {
        return 1;
    }
    
    return 0;
}


#endif /* CONFIG_MMC */
