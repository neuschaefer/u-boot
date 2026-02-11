#include <linux/types.h>
#include <malloc.h>
#include <asm/errno.h>
#include <asm/arch/platform.h>
#include <asm/io.h>
#include "bcm2708_smi.h"
#include "bcm2708_dma.h"
#include "bcm2708_smi_regs.h"

#define USE_DMA         1
#define USE_FIFO        0
#define USE_DIRECT      0

// FIFO setup defines
#define FIFO_NON_DMA 0
#define FIFO_DMA     1

#define FIFO_READ    0
#define FIFO_WRITE   1

struct bcm2708_smi_info {
        void *smi_base;
        void *dma_base;
};

static struct bcm2708_smi_info *smi_info = NULL;

static int32_t smi_check_transfer_complete( void )
{
        int32_t success = -1; //fail by default
        int timeout = 0;
        int transfer_completed = 0;

        do
        {
                // check the transfer is done
                if ( readl((smi_info->smi_base + SMICS)) & (1 << SMICS_DONE))
                {
                        //check the peripheral is no longer active
                        if ( !(readl((smi_info->smi_base + SMICS)) & (1 << SMICS_ACTIVE)))
                        {
                                transfer_completed = 1;
                        }
                }
        }
        while ( !transfer_completed && (timeout++ < 100000) );

        // did we time out?
        if (transfer_completed)
        {
                //ensure that the fifo is empty
                if ( !(readl((smi_info->smi_base + SMIFD)) & (0x1F << SMIFD_FCNT) /*fifo level*/ ))
                {
                        //check all fifo flags are cleared
                        if ( (readl((smi_info->smi_base + SMICS)) >> 24)
                                        == (( (1<<SMICS_TXE) | (1<<SMICS_TXD) | (1<<SMICS_TXW)) >> 24) )
                        {
                                //success!
                                success = 0;
                        }
                }
        }

        if (success)
                printf("[%s]: smi fifo error\n", __func__);

        return success;
}

static void smi_setup_fifo(const uint32_t slot,
                const uint32_t address,
                const uint32_t data_size_in_bytes,
                const uint32_t dma,
                const uint32_t write)
{
        uint32_t smics, smil, smia, smidc;

        /* disable SMI */
        smics = 0;
        writel(smics, (smi_info->smi_base + SMICS));

        // we do 8bit transfers so just length
        smil = data_size_in_bytes;
        writel(smil, (smi_info->smi_base + SMIL));

        smia = (address & 0x3F)        // address
                | (slot << SMIA_DEVICE);  // which settings to use.
        writel(smia, (smi_info->smi_base + SMIA));

        // set dma mode if required
        smidc = readl(smi_info->smi_base + SMIDC);
        if( dma ) {
                smidc |= (1 << SMIDC_DMAEN);
                writel(smidc, (smi_info->smi_base + SMIDC));
        } else {
                smidc &= ~(1 << SMIDC_DMAEN);
                writel(smidc, (smi_info->smi_base + SMIDC));
        }

        // once the other registers are set up we can enable the peripheral
        smics = (1 << SMICS_ENABLE);
        writel(smics, (smi_info->smi_base + SMICS));

        // we use SMI_PIXEL_FORMAT_NAND, see nand_common.c
        smics |= (1 << SMICS_PXLDAT);
        writel(smics, (smi_info->smi_base + SMICS));

        smics |= (write << SMICS_WRITE) // set the write flag if needed
                |  (1 << SMICS_CLEARFIFO); // clear the FIFO
        writel(smics, (smi_info->smi_base + SMICS));
        smics |= (1 << SMICS_START);    // start the transfer
        writel(smics, (smi_info->smi_base + SMICS));

}

int32_t smi_wait_transfer_complete(void)
{
        uint32_t smics;

        do {
                smics = readl((smi_info->smi_base + SMICS));
        }while( !(smics & (1 << SMICS_DONE)) );

        return 0;
}

int32_t smi_wait_direct_complete(void)
{
        uint32_t smidcs;

        do {
                smidcs = readl((smi_info->smi_base + SMIDCS));
        }while( !(smidcs & (1 << SMIDCS_DONE)) );

        return 0;
}

int32_t smi_read_direct (const uint32_t slot, const uint32_t address,
                const uint32_t data_size_in_bytes, const uint32_t blocking,
                const void *data)
{
        int i = 0;
        int ret = -1;
        uint32_t smidcs, smida;
        uint32_t read_data;
        uint8_t *buf = (uint8_t *)data;

        //	bcm2708_smi_print("slot=%d, address=0x%x, size=%d, blocking=%d, data=%p\n",
        //		slot, address, data_size_in_bytes, blocking, data);
        for(i = 0; i < data_size_in_bytes; i++) {

                smida = (address & 0x3F) | (slot << SMIDA_DEVICE);
                writel(smida, (smi_info->smi_base + SMIDA));

                //start the transfer
                smidcs = (1 << SMIDCS_ENABLE);
                writel(smidcs, (smi_info->smi_base + SMIDCS));

                smidcs |= (1 << SMIDCS_START);
                writel(smidcs, (smi_info->smi_base + SMIDCS));

                /* wait until data is transferred */
                smi_wait_direct_complete();
                //TODO
                read_data = readl((smi_info->smi_base + SMIDD));

                *buf = (uint8_t) read_data;

                smidcs = 1 << SMIDCS_DONE;
                writel(smidcs, (smi_info->smi_base + SMIDCS));
                buf++;
        }

        return 0;

}

int32_t smi_read (const uint32_t slot, const uint32_t address,
                const uint32_t data_size_in_bytes, const uint32_t blocking,
                const void *data)
{
#if USE_DIRECT
        int i = 0;
        int ret = -1;
        uint32_t smidcs, smida;
        uint32_t read_data;
        uint8_t *buf = (uint8_t *)data;

        //	bcm2708_smi_print("slot=%d, address=0x%x, size=%d, blocking=%d, data=%p\n",
        //		slot, address, data_size_in_bytes, blocking, data);
        for(i = 0; i < data_size_in_bytes; i++) {

                smida = (address & 0x3F) | (slot << SMIDA_DEVICE);
                writel(smida, (smi_info->smi_base + SMIDA));

                //start the transfer
                smidcs = (1 << SMIDCS_ENABLE);
                writel(smidcs, (smi_info->smi_base + SMIDCS));

                smidcs |= (1 << SMIDCS_START);
                writel(smidcs, (smi_info->smi_base + SMIDCS));

                /* wait until data is transferred */
                smi_wait_direct_complete();
                //TODO
                read_data = readl((smi_info->smi_base + SMIDD));

                *buf = (uint8_t) read_data;

                smidcs = 1 << SMIDCS_DONE;
                writel(smidcs, (smi_info->smi_base + SMIDCS));
                buf++;
        }

        return 0;
exit_err:
        return ret;

#elif (USE_FIFO)
        smi_setup_fifo(slot, address, data_size_in_bytes, FIFO_NON_DMA, FIFO_READ);

        int32_t success = -1;
        uint32_t timeout = 0;

        uint32_t dummy;
        uint8_t * ptr = (uint8_t *)data;
        uint32_t data_size = data_size_in_bytes;

        // do the read
        while(data_size) 
        {
                timeout = 0;

                while ( !( readl((smi_info->smi_base + SMICS)) & (1 << SMICS_RXR)) && (timeout++ < 10000) ); //wait until there is data in the read FIFO

                if( timeout > 10000) {
                        printf("[%s]: timeout while waiting for data\n", __func__);
                        success = -1;
                        goto exit_err;
                }

                if (data_size / 4) {
                        *((uint32_t *)ptr) = readl( (smi_info->smi_base + SMID) );
                        ptr += 4;
                        data_size -= 4;
                }
                else {
                        //read data from the FIFO
                        dummy =  readl((smi_info->smi_base + SMID));
                        // handle alignment issues by using memcpy
                        memcpy(ptr, &dummy, data_size);
                        ptr += data_size;
                        data_size -= data_size;
                }
        }
        // verify that the transfer has completed
        success += smi_check_transfer_complete();
exit_err:
        return success;
#elif (USE_DMA)

		int ret = -1;

//		bcm2708_smi_print("slot=%d, address=0x%x, size=%d, blocking=%d, data=%p\n",
//				slot, address, data_size_in_bytes, blocking, data);
		
		/* setup the smi for DMA */
        smi_setup_fifo( slot, address, data_size_in_bytes, FIFO_DMA, FIFO_READ);

        void *buf = malloc(sizeof(bcm2708_dma_cb_t) + DMA_BYTE_ALIGN);
		if( !buf ) {
			printf("[%s]: error in allocating dma structure \n");
			ret = -ENOMEM;
			goto exit_error;
		}

        bcm2708_dma_cb_t *cb = (bcm2708_dma_cb_t *) ALIGN_UP(buf, DMA_BYTE_ALIGN);

        cb->ti = (SMI_PERIPH_DREQ << DMA_TI_PERMAP) 
			| ( 1 << DMA_TI_DEST_INC) 
			| ( 1 << DMA_TI_SRC_DREQ)
			| ( 1 << DMA_TI_DEST_WIDTH);
        cb->source_ad = (void *) (SMI_PHY_BASE + SMID);
        cb->dest_ad = (void *)  ALIAS_DIRECT(data);
        cb->txfr_len = data_size_in_bytes;
        cb->nextconbk = 0x0;
        cb->res1 = 0x0;
        cb->res2 = 0x0;
//		bcm2708_smi_print("cb=0x%x ti=0x%x source_ad=0x%x dest_ad=0x%x len=0x%x stride=0x%x nextcon=0x%x\n",
//				cb, cb->ti, cb->source_ad, cb->dest_ad, cb->txfr_len, cb->stride, cb->nextconbk);

        //reset dma 
        writel( 1 << DMA_CS_RESET, (smi_info->dma_base + DMA0_CS) );
        // write cb block addr
        writel( ALIAS_DIRECT(cb), (smi_info->dma_base + DMA0_CONBLOCK_AD));
        // activate dma
        writel( 1 << DMA_CS_ACTIVE, (smi_info->dma_base + DMA0_CS) );

        //wait for dma to finish
        while( !( readl(smi_info->dma_base + DMA0_CS) & (1 << DMA_CS_END) ) );
        free(buf);
        return 0;
exit_error:
		return ret;

#endif
}



int32_t smi_write_direct (const uint32_t slot, const uint32_t address,
                const uint32_t data_size_in_bytes, const uint32_t blocking,
                const void *data)
{
        int i = 0;
        int ret = -1;
        uint32_t smidcs, smida;
        uint32_t write_data = 0;
        uint8_t *buf = (uint8_t *)data;

        //bcm2708_smi_print("slot=%d, address=0x%x, size=%d, blocking=%d, data=%p\n",
        //		slot, address, data_size_in_bytes, blocking, data);

        for(i = 0; i < data_size_in_bytes; i++) {
                smida = (address & 0x3F) | (slot << SMIDA_DEVICE);
                writel(smida, (smi_info->smi_base + SMIDA));

                write_data = *buf;

                writel(write_data, (smi_info->smi_base + SMIDD));

                //start the transfer
                smidcs = (1 << SMIDCS_ENABLE | 1 << SMIDCS_WRITE);
                writel(smidcs, (smi_info->smi_base + SMIDCS));

                smidcs |= (1 << SMIDCS_START);
                writel(smidcs, (smi_info->smi_base + SMIDCS));

                //wait for data complete
                smi_wait_direct_complete();

                smidcs = (1 << SMIDCS_DONE);
                writel(smidcs, (smi_info->smi_base + SMIDCS));
                buf++;
        }
        return 0;
}

int32_t smi_write (const uint32_t slot, const uint32_t address,
                const uint32_t data_size_in_bytes, const uint32_t blocking,
                const void *data)
{
#if USE_DIRECT
        int i = 0;
        int ret = -1;
        uint32_t smidcs, smida;
        uint32_t write_data = 0;
        uint8_t *buf = (uint8_t *)data;

        //bcm2708_smi_print("slot=%d, address=0x%x, size=%d, blocking=%d, data=%p\n",
        //		slot, address, data_size_in_bytes, blocking, data);

        for(i = 0; i < data_size_in_bytes; i++) {
                smida = (address & 0x3F) | (slot << SMIDA_DEVICE);
                writel(smida, (smi_info->smi_base + SMIDA));

                write_data = *buf;

                writel(write_data, (smi_info->smi_base + SMIDD));

                //start the transfer
                smidcs = (1 << SMIDCS_ENABLE | 1 << SMIDCS_WRITE);
                writel(smidcs, (smi_info->smi_base + SMIDCS));

                smidcs |= (1 << SMIDCS_START);
                writel(smidcs, (smi_info->smi_base + SMIDCS));

                //wait for data complete
                smi_wait_direct_complete();

                smidcs = (1 << SMIDCS_DONE);
                writel(smidcs, (smi_info->smi_base + SMIDCS));
                buf++;
        }
        return 0;
exit_err:
        return ret;
#elif (USE_FIFO)
        smi_setup_fifo(slot, address, data_size_in_bytes, FIFO_NON_DMA, FIFO_WRITE);

        int32_t success = -1;
        uint32_t timeout = 0;
        uint32_t dummy = 0;
        uint8_t * ptr = (uint8_t *)data;
        uint32_t data_size = data_size_in_bytes;

        // do the write
        //    for (i = 0; i < (data_size_in_bytes / sizeof( uint32_t) ); i++ )
        while(data_size)
        {
                timeout = 0;

                while ( !(readl((smi_info->smi_base + SMICS)) & (1 << SMICS_TXW)) && (timeout++ < 5000) ); //wait until there is space in the write FIFO.

                if( timeout > 5000) {
                        printf("[%s]: timeout while waiting for data\n", __func__ );
                        success = -1;
                        goto exit_err;
                }

                // handle alignment issues using memcpy
                if( data_size / 4 ) {
                        memcpy(&dummy, ptr, 4);
                        data_size -= 4;
                        ptr += 4;
                }
                else {
                        memcpy(&dummy, ptr, data_size);
                        data_size -= data_size;
                        ptr += data_size;
                }

                // write data to the FIFO
                writel(dummy, (smi_info->smi_base + SMID));
        }

        // verify that the transfer has completed
        success += smi_check_transfer_complete();

exit_err:
        return(success);
#elif (USE_DMA)

		int ret = -1;

		/* setup SMI for DMA */ 
		smi_setup_fifo( slot, address, data_size_in_bytes, FIFO_DMA, FIFO_WRITE);

		void *buf = malloc(sizeof(bcm2708_dma_cb_t) + DMA_BYTE_ALIGN);

		if( !buf ) {
			printf("[%s]: Error in allocating dma structure\n", __func__ );
			ret = -ENOMEM;
			goto exit_error;
		}
 
		bcm2708_dma_cb_t *cb = (bcm2708_dma_cb_t *) ALIGN_UP(buf, DMA_BYTE_ALIGN);

        cb->ti = (SMI_PERIPH_DREQ << DMA_TI_PERMAP)
			| ( 1 << DMA_TI_SRC_INC)
			| ( 1 << DMA_TI_DEST_DREQ)
			| ( 1 << DMA_TI_SRC_WIDTH);

        cb->source_ad = (void *) ALIAS_DIRECT(data);
        cb->dest_ad = (void *) (SMI_PHY_BASE + SMID);
        cb->txfr_len = data_size_in_bytes;
        cb->nextconbk = 0x0;
        cb->res1 = 0x0;
        cb->res2 = 0x0;
 
		//reset dma 
        writel( 1 << DMA_CS_RESET, (smi_info->dma_base + DMA0_CS) );
 
        writel( ALIAS_DIRECT(cb), (unsigned int)(smi_info->dma_base + DMA0_CONBLOCK_AD));
        // activate dma
        writel( 1 << DMA_CS_ACTIVE, (smi_info->dma_base + DMA0_CS) );

        //wait for dma to finish
        while( !( readl(smi_info->dma_base + DMA0_CS) & (1 << DMA_CS_END) ) );
        free(buf);
        return 0;
exit_error:
		return ret;
#endif

}




int32_t smi_setup_timing( const uint32_t slot, 
                const struct smi_periph_setup *periph_setup, const uint32_t smi_freq_hz )
{
        int ret = -1;
        uint32_t mode68, format32, pixel_swap;
        uint32_t write_width, read_width, clock_period_in_ns = 1000000000/smi_freq_hz;
        uint32_t read_strobe_time, read_pace_time, read_hold_time, read_setup_time;
        uint32_t write_strobe_time, write_pace_time, write_hold_time, write_setup_time;
        uint32_t val;
        uint32_t smidsr, smidsw, smics;


        bcm2708_smi_print("slot=%d periph_setup=%p smi_freq_hz=%d\n",
                        slot, periph_setup, smi_freq_hz);

        if(!smi_info) {
                bcm2708_smi_print("SMI driver not initialized\n");
                ret = -ENODEV;
                goto exit_err_init;
        }

        if(slot > 3) {
                bcm2708_smi_print("slot should be less than 3\n");
                ret = -ENOENT;
                goto exit_err_slot;
        }

        /* disable smi */
        val = readl(smi_info->smi_base + SMICS);
        val &= ~1;
        writel( (unsigned int)val, (unsigned int)(smi_info->smi_base + SMICS));

        switch(periph_setup->read_timings.transfer_width) 
        {
                case SMI_TRANSFER_WIDTH_8BIT:
                        bcm2708_smi_print("Read xfer width=8bit\n");
                        read_width = 0;
                        break;
                default:
                        bcm2708_smi_print("[%s]:Invalid read transfer width\n", __func__);
                        ret = -EINVAL;
                        goto exit_err_xfer;
                        break;
        }

        switch(periph_setup->write_timings.transfer_width)
        {
                case SMI_TRANSFER_WIDTH_8BIT:
                        bcm2708_smi_print("write xfer width=8bit\n");
                        write_width = 0;
                        break;
                default:
                        bcm2708_smi_print("[%s]:Invalid write transfer width\n", __func__);
                        ret = -EINVAL;
                        goto exit_err_xfer;
                        break;
        }

        read_strobe_time = (periph_setup->read_timings.strobe_time_in_ns + 
                        clock_period_in_ns - 1) / clock_period_in_ns;
        read_pace_time = (periph_setup->read_timings.pace_time_in_ns + 
                        clock_period_in_ns - 1) / clock_period_in_ns;
        read_hold_time = (periph_setup->read_timings.hold_time_in_ns + 
                        clock_period_in_ns - 1) / clock_period_in_ns;
        read_setup_time = (periph_setup->read_timings.setup_time_in_ns + 
                        clock_period_in_ns - 1) / clock_period_in_ns;

        read_strobe_time = clamp_t(uint32_t, read_strobe_time, 127, 0);
        read_pace_time = clamp_t(uint32_t, read_pace_time, 127, 0);
        read_hold_time = clamp_t(uint32_t, read_hold_time, 63, 0);
        read_setup_time = clamp_t(uint32_t, read_setup_time, 63, 0);

        mode68 = (periph_setup->mode == SMI_MODE_68);

        bcm2708_smi_print("Read: stobe_time=%d pace_time=%d hold_time=%d"
                        "setup_time=%d mode68=%d width=%d\n", read_strobe_time,
                        read_pace_time, read_hold_time, read_setup_time, mode68,
                        read_width);

        smidsr = (read_strobe_time << SMIDS_STROBE)
                | (read_pace_time << SMIDS_PACE)
                | (read_hold_time << SMIDS_HOLD)
                | (mode68  << SMIDS_MODE68)
                | (read_setup_time << SMIDS_SETUP)
                | (read_width << SMIDS_WIDTH);

        /* smidsw */
        write_strobe_time = (periph_setup->write_timings.strobe_time_in_ns + 
                        clock_period_in_ns - 1) / clock_period_in_ns;
        write_pace_time = (periph_setup->write_timings.pace_time_in_ns + 
                        clock_period_in_ns - 1) / clock_period_in_ns;
        write_hold_time = (periph_setup->write_timings.hold_time_in_ns + 
                        clock_period_in_ns - 1) / clock_period_in_ns;
        write_setup_time = (periph_setup->write_timings.setup_time_in_ns + 
                        clock_period_in_ns - 1) / clock_period_in_ns;

        write_strobe_time = clamp_t( uint32_t, write_strobe_time, 127, 0);
        write_pace_time = clamp_t(uint32_t, write_pace_time, 127, 0);
        write_hold_time = clamp_t(uint32_t, write_hold_time, 63, 0);
        write_setup_time = clamp_t(uint32_t, write_setup_time, 63, 0);

        pixel_swap = (periph_setup->pixel_bits_swapped && 
                        (periph_setup->pixel_format != SMI_PIXEL_FORMAT_DONT_CARE)) ? 1 : 0;

        format32 = (periph_setup->pixel_format == SMI_PIXEL_FORMAT_32BIT_RGB888);

        bcm2708_smi_print("write: stobe_time=%d pace_time=%d hold_time=%d"
                        "setup_time=%d pixel_swap=%d format32=%d width=%d\n", 
                        write_strobe_time,write_pace_time, write_hold_time,
                        write_setup_time, pixel_swap, format32, write_width);

        smidsw = (write_strobe_time << SMIDS_STROBE)
                | (write_pace_time << SMIDS_PACE)
                | (write_hold_time << SMIDS_HOLD)
                | (pixel_swap  << SMIDS_SWAP)
                | (format32 << SMIDS_FORMAT)
                | (write_setup_time << SMIDS_SETUP)
                | (write_width << SMIDS_WIDTH);


        /* sync settings */
        writel(smidsr, (smi_info->smi_base + SMIDSR0 + (slot * (SMIDSR1 - SMIDSR0))));
        writel(smidsw, (smi_info->smi_base + SMIDSW0 + (slot * (SMIDSW1 - SMIDSW0))));

        /*smics*/
        smics = readl(smi_info->smi_base + SMICS);
        if (periph_setup->tearing_effect)
                smics |= SMICS_TEEN;
        else
                smics &= ~SMICS_TEEN;

        if (periph_setup->hvs_input)
                smics |= SMICS_PVMODE;
        else
                smics &= ~SMICS_PVMODE;
        writel(smics, (smi_info->smi_base + SMICS));

        return 0;

exit_err_xfer:
exit_err_slot:
exit_err_init:
        return ret;
}


int bcm2708_smi_init(void)
{
        int ret = 0;

        struct bcm2708_smi_info *info;

        bcm2708_smi_print("probed 2708\n");	

        info = malloc(sizeof(struct bcm2708_smi_info));
        if (info == NULL) {
                bcm2708_smi_print( "no memory for smi info\n");
                ret = -ENOMEM;
                goto exit_mem_error;
        }

        /* get the smi  memory region */
        info->smi_base = (void*) SMI_BASE;
        info->dma_base = (void*) DMA_BASE;

        smi_info = info;  

        bcm2708_smi_print("SMI start=%p DMA start=%p\n", (void *)info->smi_base,
                        (void *)info->dma_base);


        return 0;

exit_mem_error:
        return ret;
}
