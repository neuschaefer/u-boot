/*
 *  linux/drivers/mmc/host/bcm2708_mci.c - Broadcom BCM2708 MCI driver
 *
 *  Copyright (C) 2010 Broadcom, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <common.h>
#include <malloc.h>
#include <part.h>
#include <mmc.h>

#include <linux/err.h>

#include <asm/io.h>
#include <asm/sizes.h>

#include "bcm2708_mci.h"

#define DRIVER_NAME "bcm2708_mci"

#define USE_MMCPLUS
#define TIMEOUT_RETRY_COUNT 0

//#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT(a,...)
#endif

struct bcm2708_mci_host {
    void *reg_base;
    struct mmc *mmc;
} BCM2708_MCI_HOST_T;

static void do_command(void *base, u32 cmd, u32 arg)
{
    DEBUG_PRINT("Do command : %08x\n", cmd);
    writel(arg, base + BCM2708_MCI_ARGUMENT);
    writel(cmd | BCM2708_MCI_ENABLE, base + BCM2708_MCI_COMMAND);

    while (readl(base + BCM2708_MCI_COMMAND) & BCM2708_MCI_ENABLE);
}

static int bcm2708_mci_read_response(struct mmc *mmc, struct mmc_cmd *cmd)
{
    struct bcm2708_mci_host *host = (struct bcm2708_mci_host *)mmc->priv;
    void *mmc_base = host->reg_base;
    int status;

    status = readl(mmc_base + BCM2708_MCI_STATUS);
    if (cmd->resp_type & MMC_RSP_BUSY) {
        DEBUG_PRINT("Waiting for busy %08x...", status);
        while (!(status & BCM2708_MCI_BUSYCLR) && !(status & !BCM2708_MCI_BUSYCLR)) {
            status = readl(mmc_base + BCM2708_MCI_STATUS);
        }
        writel(BCM2708_MCI_BUSYCLR, mmc_base + BCM2708_MCI_STATUS);
        DEBUG_PRINT("Busy cleared! %08x\n", status);
        status = readl(mmc_base + BCM2708_MCI_STATUS);
    }


    if (status & BCM2708_MCI_CMDTIMEOUT) {
        // Timeout is expected for some commands.
        DEBUG_PRINT("ERROR: mmc driver saw timeout with opcode = %d, timeout = %d\n",
                cmd->cmdidx, readl(mmc_base + BCM2708_MCI_TIMEOUT));
        return 1;
    } else if (status & BCM2708_MCI_FIFOERR){
        printf("ERROR reading response, fifo data error %08x\n", status);
        return -1;
    } else if ((status & BCM2708_MCI_CRC7ERR) && (cmd->resp_type & MMC_RSP_CRC)) {
        printf("ERROR reading response, crc7 error %08x\n", status);
        cmd->response[0] = readl(mmc_base + BCM2708_MCI_RESPONSE0);
        printf("%08x %08x\n", cmd->response[0], status);
        return -1;
    } else if (status) {
        printf("Possible error in response %08x\n", status);
    }

    if (cmd->resp_type & MMC_RSP_136) {
        cmd->response[3] = readl(mmc_base + BCM2708_MCI_RESPONSE0);
        cmd->response[2] = readl(mmc_base + BCM2708_MCI_RESPONSE1);
        cmd->response[1] = readl(mmc_base + BCM2708_MCI_RESPONSE2);
        cmd->response[0] = readl(mmc_base + BCM2708_MCI_RESPONSE3);
        DEBUG_PRINT("%08x:%08x:%08x:%08x %08x\n",
                cmd->response[3], cmd->response[2], cmd->response[1], cmd->response[0], status);
    } else {
        cmd->response[0] = readl(mmc_base + BCM2708_MCI_RESPONSE0);
        DEBUG_PRINT("%08x %08x\n", cmd->response[0], status);
    }
    return 0;
}



static void bcm2708_mci_transfer_data(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
    struct bcm2708_mci_host *host = (struct bcm2708_mci_host *)mmc->priv;
    void *mmc_base = host->reg_base;
    unsigned long length;
    uint32_t *ptr;
    uint32_t status;

    length = data->blocks * data->blocksize;
    DEBUG_PRINT("transfer data %lu\n", length);

    if (data->flags & MMC_DATA_READ) {
        ptr = (uint32_t *) data->dest;
    } else {
        ptr = (uint32_t *) data->src;
    }

    while (length > 3) {
        while (!(readl(mmc_base + BCM2708_MCI_STATUS) & BCM2708_MCI_DATAFLAG));

        if (data->flags & MMC_DATA_READ) {
            *ptr++ = readl(mmc_base + BCM2708_MCI_DATA);
        } else {
            writel(*ptr++, mmc_base + BCM2708_MCI_DATA);
        }
        length -= 4;
    }

   status = readl(mmc_base + BCM2708_MCI_STATUS);
   while (status & BCM2708_MCI_DATAFLAG) {
       uint32_t data;
       printf("ERROR Data flag still set! %08x", status);
       data = readl(mmc_base + BCM2708_MCI_DATA);
       printf("data: %08x\n", data);
       status = readl(mmc_base + BCM2708_MCI_STATUS);
   }
}

static int
bcm2708_mci_start_command(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
    struct bcm2708_mci_host *host = (struct bcm2708_mci_host *)mmc->priv;
    void *mmc_base = host->reg_base;
    uint32_t opcode;
    int redo = 0;
    uint32_t temp;

    DEBUG_PRINT("op %u arg %08x flags %08x resp_type %08x data %08x\n",
            cmd->cmdidx, cmd->cmdarg, cmd->flags, cmd->resp_type, data);

    do {
        int status;
        /*
         * clear the controller status register
         */
        writel(-1, mmc_base + BCM2708_MCI_STATUS);
        temp = readl(mmc_base + BCM2708_MCI_STATUS);
        if (temp != 0) {
            printf("Error: status register not cleared after reset!\n");
        }

        /*
         * build the command register write, incorporating no
         * response, long response, busy, read and write flags
         */

        opcode = cmd->cmdidx;

        if (cmd->resp_type & MMC_RSP_PRESENT) {
            if (cmd->resp_type & MMC_RSP_136) {
                opcode |= BCM2708_MCI_LONGRESP;
            }
        } else {
            opcode |= BCM2708_MCI_NORESP;
        }

        if (cmd->resp_type & MMC_RSP_BUSY) {
            opcode |= BCM2708_MCI_BUSY;
        }

        if (data) {
            if (data->flags & MMC_DATA_READ) {
                opcode |= BCM2708_MCI_READ;
            } else {
                opcode |= BCM2708_MCI_WRITE;
            }

            if (data->blocksize) {
                DEBUG_PRINT("BYTECOUT %d BLOCKCOUNT %d .. ",readl(mmc_base + BCM2708_MCI_HBCT), readl(mmc_base + BCM2708_MCI_HBLC));
                DEBUG_PRINT("set blocksize to %d\n", data->blocksize);
                writel(data->blocksize, mmc_base + BCM2708_MCI_HBCT);
            }
        }

        /*
         * run the command and wait for it to complete
         */

        do_command(mmc_base, opcode, cmd->cmdarg);

        /*
         * retrieve the response and error (if any)
         * if it times out, retry
         */
        status = bcm2708_mci_read_response(mmc, cmd);

        // No error, don't need to retry
        if (status == 0) {
            break;
        } else if (status > 0) { // Time out, retry
            redo += status;
        } else {
            printf("COMM_ERROR, unrecoverable.\n");
            return COMM_ERR; // unrecoverable error
        }
    } while (redo < TIMEOUT_RETRY_COUNT);

    if (redo <= TIMEOUT_RETRY_COUNT) {
        if (data) {
            bcm2708_mci_transfer_data(mmc, cmd, data);
        }
    } else {
        DEBUG_PRINT("command timed out\n");
        return TIMEOUT;
    }
    return 0;
}

static int bcm2708_mci_request(struct mmc *mmc, struct mmc_cmd *cmd,
        struct mmc_data *data)
{
    // Check is block size of power of 2.
    if (data && (data->blocksize & (data->blocksize - 1))) {
        printf("Error: %s: Unsupported block size (%d bytes)\n",
                mmc->name, data->blocksize);
        return COMM_ERR;
    }

    return bcm2708_mci_start_command(mmc, cmd, data);
}

static void bcm2708_mci_set_ios(struct mmc *mmc)
{
    struct bcm2708_mci_host *host = (struct bcm2708_mci_host *)mmc->priv;
    void *mmc_base = host->reg_base;
    // Setup bus width and clock speed

    DEBUG_PRINT("Want to set clock: %d width: %d\n", mmc->clock, mmc->bus_width);
    if (mmc->clock == 25000000 || mmc->clock == 26000000) {
        DEBUG_PRINT("setting clock div to 10 (8+2)\n");
        writel(0x8, mmc_base + BCM2708_MCI_CDIV);
    } else if (mmc->clock == 50000000 || mmc->clock == 52000000) {
        DEBUG_PRINT("setting clock div to 5 (3+2)\n");
        writel(0x3, mmc_base + BCM2708_MCI_CDIV);
    } else {
        // On init or unknown clock, we set the clock really low
        DEBUG_PRINT("Setting clock div to 0x7ff\n");
        writel(0x7FF, mmc_base + BCM2708_MCI_CDIV);
    }

    if (mmc->bus_width) {
        uint32_t hcfg;
        hcfg = readl(mmc_base + BCM2708_MCI_HCFG);
        DEBUG_PRINT("setting bus width to %d\n", mmc->bus_width);

        hcfg &= BCM2708_MCI_HCFG_WIDEEXT_CLR;
        hcfg |= (mmc->bus_width == 4) ? BCM2708_MCI_HCFG_WIDEEXT_4BIT : 0;

        writel(hcfg, mmc_base + BCM2708_MCI_HCFG);
    }
}

static int bcm2708_mci_init(struct mmc *mmc)
{
    struct bcm2708_mci_host *host = (struct bcm2708_mci_host *)mmc->priv;
    void *mmc_base = host->reg_base;

    // pin muxing/gpios is done by vcloader

    DEBUG_PRINT("Resetting BCM2708 MCI Controller.\n");

    writel(0, mmc_base + BCM2708_MCI_COMMAND);
    writel(0, mmc_base + BCM2708_MCI_ARGUMENT);
    writel(0x00F00000, mmc_base + BCM2708_MCI_TIMEOUT);
    writel(0, mmc_base + BCM2708_MCI_CDIV);
    writel(0, mmc_base + BCM2708_MCI_STATUS);
    writel(0, mmc_base + BCM2708_MCI_VDD);
    writel(0, mmc_base + BCM2708_MCI_HCFG);
    writel(0, mmc_base + BCM2708_MCI_HBCT);
    writel(0, mmc_base + BCM2708_MCI_HBLC);

    writel( BCM2708_MCI_HCFG_SLOW_CARD | BCM2708_MCI_HCFG_BUSY_IRPT_EN |
            BCM2708_MCI_HCFG_BLOCK_IRPT_EN | BCM2708_MCI_HCFG_WIDE_INT_BUS,
            mmc_base + BCM2708_MCI_HCFG);

    // On A0 silicon it has been observed that the following must hold
    // WRITE_THRESHOLD<=5 and READ_THRESHOLD<=WRITE_THRESHOLD+1
    // with the chip running at 150MHz (with the interface running @ 150/22 = 6.8 MHz)
    // the second requirement suggests that the verilog does not properly separate the read / write FIFOs
    // On V3XDS Read=2 & Write=6

#define READ_THRESHOLD  3
#define WRITE_THRESHOLD 3
#if 1 // !!! This is still required, without it we get CRC16 errors in data.
    {
        uint32_t temp;
        temp = readl(mmc_base + BCM2708_MCI_EDM);
        temp &= ~((0x1F<<14) | (0x1F<<9));
        temp  |= (WRITE_THRESHOLD << 9) | (READ_THRESHOLD << 14);
        writel(temp, mmc_base + BCM2708_MCI_EDM);
    }
#endif

    // Power on delay
    udelay(10000);
    writel(BCM2708_MCI_VDD_ENABLE, mmc_base + BCM2708_MCI_VDD);
    udelay(10000);

    return 0;
}

int bcm2708_mmc_init(bd_t *bis)
{
    struct mmc *mmc = NULL;
    struct bcm2708_mci_host *host = NULL;

    mmc = malloc(sizeof(struct mmc));
    host = malloc(sizeof(struct bcm2708_mci_host));

    if (!mmc)
        return -ENOMEM;

    if (!host) {
        free(mmc);
        return -ENOMEM;
    }

    memset(mmc, 0, sizeof(struct mmc));
    memset(host, 0, sizeof(struct bcm2708_mci_host));

    host->reg_base = (void *)MMCI0_BASE;
    host->mmc = mmc;

    sprintf(mmc->name, DRIVER_NAME);
    mmc->send_cmd = bcm2708_mci_request;
    mmc->set_ios = bcm2708_mci_set_ios;
    mmc->init = bcm2708_mci_init;
    mmc->host_caps = MMC_MODE_4BIT;

#ifdef USE_MMCPLUS
    // MMC Plus adds support for 8-Bit MMC, 52 MHz
    // MODE_HS -- For high speed 50 MHz SD
    mmc->host_caps |= MMC_MODE_8BIT | MMC_MODE_HS | MMC_MODE_HS_52MHz;
#endif

    mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
    mmc->f_min = 100000;
    mmc->f_max = 52000000;
    mmc->block_dev.part_type = PART_TYPE_DOS;

    mmc->priv = (void *)host;

    mmc_register(mmc);

    return 0;
}
