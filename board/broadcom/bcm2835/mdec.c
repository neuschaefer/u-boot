#include <common.h>
#include <asm/arch/vcio.h>
#include <asm/arch/ipc.h>
#include <asm/io.h>

#include "mdec_codecs.h"
#include <asm/arch/media_dec_regs.h>
#include "mdec.h"

/*******************************************************************/
/* bootvc - boot vc image from image in memory */
/*******************************************************************/
static uint32_t mdec_id = 0;
static uint32_t mdec_base = 0;

union fourcc_union
{
	uint32_t fourcc_num;
	char fourcc_char[4];
};

int mdec_init(void) {
    // Check VC Media Dec IPC.
     union fourcc_union fourcc;

    if (mdec_base == 0) {
        memcpy(fourcc.fourcc_char, "MEDD", 4);
        mdec_base = ipc_get_service_addr(fourcc.fourcc_num, &mdec_id);
        printf("Starting media dec, base = %x id=%d\n", mdec_base, mdec_id);
        if (!mdec_base) {
            printf("Error: Failed to get mdec base addr\n");
            return -1;
        }
    }
    return 0;
}

void mdec_shutdown(void) {
    mdec_id = 0;
    mdec_base = 0;
}

void media_dec_set_state(const uint32_t play, uint32_t blocking)
{
    volatile uint32_t val;
    val = readl(mdec_base + MEDIA_DEC_CONTROL_OFFSET);

    if( play ) {
        val |= MEDIA_DEC_CONTROL_PLAY_BIT;
    } else {
        val &= (~MEDIA_DEC_CONTROL_PLAY_BIT);
    }

    writel(val, mdec_base + MEDIA_DEC_CONTROL_OFFSET);

    ipc_notify_vc_event(mdec_id);

    if (blocking) {
        val = readl(mdec_base + MEDIA_DEC_STATUS_OFFSET);
        while((val & MEDIA_DEC_CONTROL_PLAY_BIT) != (play ? MEDIA_DEC_CONTROL_PLAY_BIT : 0)) {
            if (val & MEDIA_DEC_CONTROL_ERROR_BIT) {
                printf("Error: set state %d failed\n", play);
                break;
            }
            val = readl(mdec_base + MEDIA_DEC_STATUS_OFFSET);
        }
    }
}


int32_t media_dec_setup_data_playback(uint32_t addr, uint32_t filled_size, uint32_t blocking)
{
   volatile uint32_t val;
   if (filled_size == 0)
   {
       printf("Error: Data size unset.\n");
       return -1;
   }
   writel(0, mdec_base + MEDIA_DEC_DEBUG_MASK );

   //test assert that the hardware is disabled
   if( 0 != readl(mdec_base + MEDIA_DEC_CONTROL_OFFSET)) {
       printf("Error: Media dec already running\n");
       return -1;
   }

   //setup the src width as 0xFFFFFFFF (ignore)
   writel(0xFFFFFFFF, mdec_base + MEDIA_DEC_SOURCE_X_OFFSET);
   writel(0xFFFFFFFF, mdec_base + MEDIA_DEC_SOURCE_Y_OFFSET);
   writel(0xFFFFFFFF, mdec_base + MEDIA_DEC_SOURCE_WIDTH_OFFSET);
   writel(0xFFFFFFFF, mdec_base + MEDIA_DEC_SOURCE_HEIGHT_OFFSET);

   writel(MEDIA_DEC_VIDEO_CodingAVC, mdec_base + MEDIA_DEC_VID_TYPE);
   writel(MEDIA_DEC_AUDIO_CodingUnused, mdec_base + MEDIA_DEC_AUD_TYPE);

   writel(__ipc_phys_to_bus(addr), mdec_base + MEDIA_DEC_DATA_ADDRESS_OFFSET);
   writel(filled_size, mdec_base + MEDIA_DEC_DATA_LENGTH_OFFSET);

   writel(MEDIA_DEC_CONTROL_ENABLE_BIT | MEDIA_DEC_CONTROL_LOCAL_DATAMODE_BIT,
           mdec_base + MEDIA_DEC_CONTROL_OFFSET);

   ipc_notify_vc_event(mdec_id);

   if (blocking) {
       val = readl(mdec_base + MEDIA_DEC_STATUS_OFFSET);
       while(!(val & MEDIA_DEC_CONTROL_ENABLE_BIT)) {
           if (val & MEDIA_DEC_CONTROL_ERROR_BIT) {
               printf("Error: mdec enable failed\n");
               break;
           }
           val = readl(mdec_base + MEDIA_DEC_STATUS_OFFSET);
       }
   } else {
       udelay(100);
   }

   //buffer size been allocated?
   if ( 0 == readl(mdec_base + MEDIA_DEC_MAX_BUFFER_SIZE)) {
       printf("Error, buffer not allocated\n");
       media_dec_tear_down( blocking);
       return -1;
   } else {
       //TODO check property to see if we should auto set state to play(1)
       media_dec_set_state(1, blocking);
   }
   return 0;
}


void media_dec_tear_down(uint32_t blocking)
{
    volatile uint32_t val;
    if (MEDIA_DEC_CONTROL_PLAY_BIT & readl(mdec_base + MEDIA_DEC_CONTROL_OFFSET)) {
        media_dec_set_state(0, blocking);
    }

    writel(0, mdec_base + MEDIA_DEC_CONTROL_OFFSET);

    ipc_notify_vc_event(mdec_id);

    if (blocking) {
        val = readl(mdec_base + MEDIA_DEC_STATUS_OFFSET);
        while(val & (MEDIA_DEC_CONTROL_ENABLE_BIT | MEDIA_DEC_CONTROL_PLAY_BIT)) {
            if (val & MEDIA_DEC_CONTROL_ERROR_BIT) {
                printf("Error: mdec enable failed\n");
                break;
            }
            val = readl(mdec_base + MEDIA_DEC_STATUS_OFFSET);
        }
    } else {
        udelay(100);
    }

    //check the ptr is invalid
    if (0 == readl(mdec_base + MEDIA_DEC_MAX_BUFFER_SIZE)) {
        printf("Error: mdec buffer still active after stop\n");
    }

   writel(0, mdec_base + MEDIA_DEC_DATA_ADDRESS_OFFSET);
   writel(0, mdec_base + MEDIA_DEC_DATA_LENGTH_OFFSET);
}

