#include <exports.h>
#include <common.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/vcio.h>

#if defined (CONFIG_VCEB)
#include "vceb/host/vceb.h"
#include "vceb/common/vceb_common.h"
#include "rle_decoder.h"
#include "audio_sample.h"
#endif



#ifdef CONFIG_VCEB
static int

arec_generate16( void *audio , uint32_t samples)
{
    int i;
    int16_t *wptr = (int16_t*) audio;

    for (i=0; i<samples; i++) {
        // the tables are 8-bit unsigned mono, 8khz data
        int pos = (i/6) % (sizeof(left_8bit) + sizeof(right_8bit));
        if ( i > 6 && pos == 0 ) break;
        if ( pos < sizeof(left_8bit) ) {
            int16_t left = left_8bit[ pos ];
            *wptr++ = (left - 0x80) << 8;
            *wptr++ = 0;
        } else {
            int16_t right = right_8bit[ pos - sizeof(left_8bit) ];
            *wptr++ = 0;
            *wptr++ = (right - 0x80) << 8;
        }
    }
    return i;
}

extern void *_binary_boot_rle_start;
extern void *_binary_boot_rle_end;

static int vceb_init(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SHOW_BOOT_PROGRESS
void show_boot_progress(int status)
{
    printf("Boot progress: %d\n", status);
}
#endif

/*****************************************
 * board_init -early hardware init
 *****************************************/
int board_init (void)
{
    gd->bd->bi_arch_number = CONFIG_MACH_TYPE; // MACH_TYPE_BCM2835; /* board id for linux */
    gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR; /* adress of boot parameters */

    return 0;
}

/*****************************************************************
 * misc_init_r - miscellaneous platform dependent initializations
 ******************************************************************/
int arch_misc_init (void)
{
    int ret = 0;
    //printf("initializing mailbox driver\n");
#ifdef CONFIG_BCM2835_MAILBOX
    ret = bcm_mailbox_init();
#endif
    return ret;
}

/*****************************************************************
 * misc_init_r - miscellaneous platform dependent initializations
 ******************************************************************/
int misc_init_r (void)
{

#ifdef CONFIG_VCEB
    vceb_init();
#endif
    return 0;
}


#ifdef CONFIG_VCEB
/*****************************************************************
 * vceb_init - Initialize video core and display splash screen 
 ******************************************************************/
int vceb_init (void)
{
    //printf("Starting VCEB\n");
    // buf size = 800 width * 480 height * 16 depth = 32 * 192000
    unsigned int *data = malloc(sizeof(unsigned int) * 250000);
    unsigned int samples;
    if (!data) {
        printf("Error: Out of memory.");
    }
    int audio_size = 48000 * 5 * (16 >> 2);

    vceb_initialise(1);
    vceb_framebuffer_overlay_enable(1);

    printf("Attempting to test VCEB_AUDIO\n");

    vceb_audio_open(8, 2, 48000, VCEB_AUDIO_FORMAT_PCM, 0);

    samples = arec_generate16(data, audio_size / (16>>2));
    while (samples < audio_size / (16 >> 2)) {
        samples += arec_generate16((uint8_t *)data + (samples * (16 >> 2)), (audio_size/(16>>2) - samples));
    }

    printf("adata: %08x sz: %d\n", data, audio_size);
    vceb_audio_start();
    vceb_audio_write(audio_size, data);
    vceb_audio_pause();
    udelay(10000000);
    vceb_audio_start();
    vceb_audio_write(audio_size, data);
    vceb_audio_volume(100);
    vceb_audio_close();

    printf("Audio Close...\n");
    vceb_audio_write(audio_size, data);
    printf("Audio Write...\n");
    vceb_audio_open(8, 2, 48000, VCEB_AUDIO_FORMAT_PCM, 0);
    printf("Audio Open...\n");
    vceb_audio_write(audio_size, data);
    printf("Audio Start...\n");
    vceb_audio_start();
    vceb_audio_write(audio_size, data);
    printf("Audio Write 2\n");
    vceb_audio_close();

    decode_rle_image(
        (void *)&_binary_boot_rle_start, 
        data,
        4 * (uint32_t)(&_binary_boot_rle_end - &_binary_boot_rle_start),
        480 * 800
    );

    vceb_framebuffer_overlay_set((void *) data, VCEB_RBG_FORMAT_RGB565,
                    0x0, 480, 800, VCEB_ALIGN_CENTRE);
    return 0;
}
#endif

/**********************************************
 * dram_init - sets uboots idea of sdram size
 **********************************************/
int dram_init (void)
{
    gd->ram_size = PHYS_SDRAM_1_SIZE; //get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
    return 0;
}

void dram_init_banksize (void)
{
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}
