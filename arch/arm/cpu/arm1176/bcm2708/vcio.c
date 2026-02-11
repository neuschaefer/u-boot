/*
 *  linux/arch/arm/mach-bcm2708/vcio.c
 *
 *  Copyright (C) 2010 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This device provides a shared mechanism for writing to the mailboxes,
 * semaphores, doorbells etc. that are shared between the ARM and the VideoCore
 * processor
 */
#include <common.h>
#include <malloc.h>
#include <asm/arch/vcio.h>
#include <asm/io.h>
#include <linux/err.h>

/* ----------------------------------------------------------------------
 *      Mailbox
 * -------------------------------------------------------------------- */

// offsets from a mail box base address
#define MAIL_WRT  0x00 // write - and next 4 words
#define MAIL_RD   0x00 // read - and next 4 words
#define MAIL_POL  0x10 // read without popping the fifo
#define MAIL_SND  0x14 // sender ID (bottom two bits)
#define MAIL_STA  0x18 // status
#define MAIL_CNF  0x1C // configuration

#define MBOX_MSG(chan, data24) (((data24) & ~0xff) | ((chan) & 0xff))

#define MBOX_MAGIC 0xd0d0c0de

typedef struct mailbox_s
{
   void *status;
   void *write;
   uint32_t magic;
} MAILBOX_T;

static MAILBOX_T *mbox_dev;

static void mbox_init(MAILBOX_T *mbox_out,
                      uint32_t addr_mbox)
{
        mbox_out->status = (void *)(addr_mbox + MAIL_STA);
        mbox_out->write  = (void *)(addr_mbox + MAIL_WRT);
        mbox_out->magic  = MBOX_MAGIC;
}


static int mbox_write(MAILBOX_T *mbox, unsigned chan, uint32_t data25)
{
        int rc;

        if (mbox->magic != MBOX_MAGIC)
                rc = -EINVAL;
        else
        {
                // wait for the mailbox FIFO to have some space in it
                while (0 != (readl(mbox->status) & (1<<31)))
                        continue;

                writel(MBOX_MSG(chan, data25), mbox->write);
                rc = 0;
        }
        return rc;
}



/* ----------------------------------------------------------------------
 *      Mailbox Methods
 * -------e------------------------------------------------------------- */

static int dev_mbox_write(unsigned chan, uint32_t data25)
{
        int rc;

        if (mbox_dev == NULL)
                rc = -ENODEV;
        else
        {
                MAILBOX_T *mailbox = mbox_dev;
                rc = mbox_write(mailbox, chan, data25);
        }
        return rc;
}

extern int bcm_mailbox_write(unsigned chan, uint32_t data25)
{
        return dev_mbox_write(chan, data25);
}


extern int bcm_mailbox_init(void)
{
	int ret = 0;
	MAILBOX_T *mailbox;

	mailbox = valloc(sizeof(*mailbox));
    if (NULL == mailbox) {
        printf("failed to allocate mailbox memory\n");
        ret = -ENOMEM;
    } else {
        memset(mailbox, 0, sizeof(*mailbox));

        // should be based on the registers from res really
        mbox_init(mailbox, ARM_0_MAIL1_WRT);

        if (NULL != mbox_dev) {
            printf("Warning, mailbox device is not null!");
        }

        mbox_dev = mailbox;
        printf("mailbox at %x\n", (ARM_0_MAIL1_WRT));
    }
    return ret;
}

extern int bcm_mailbox_remove(void)
{
    MAILBOX_T *mailbox = mbox_dev;
   if (mbox_dev != NULL) {
       free(mailbox);
       mailbox = NULL;
   }

   return 0;
}

