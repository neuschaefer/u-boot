/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 misc initiations.
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
#include "cdefs.h"
#include "com_defs.h"
#include "wpcm450_gctrl_regs.h"
#include "ram_init.h"
#include "wpcm450_aic.h"
#include "wpcm450_fwupgrade.h"
#include "wpcm450_platform.h"
#include "wpcm450_mc_regs.h"


/* extern int mmc_init(int verbose); */
extern int fwu_init(void);
extern int ps_check(void);

/* platform ID */
UINT8 platform_id = PF_ID_UNKNOWN;


DECLARE_GLOBAL_DATA_PTR;


/* fan control */
int fan_control(void)
{
    /* (0, 0) = PWM0 \
       (0, 1) = PWM1 \
       (0, 2) = PWM2 \
       (0, 3) = PWM3 \
       (1, 0) = PWM4 \
       (1, 1) = PWM5 \
       (1, 2) = PWM6 \
       (1, 3) = PWM7 */
    
    SET_BIT(MFSEL2, MF_PWM7SEL_BIT);
    SET_BIT(MFSEL2, MF_PWM6SEL_BIT);
    
    /* SET_BIT(MFSEL2, MF_PWM5SEL_BIT); */
    
    SET_BIT(MFSEL2, MF_PWM4SEL_BIT);
    
    /* SET_BIT(MFSEL2, MF_PWM3SEL_BIT); */
    SET_BIT(MFSEL2, MF_PWM2SEL_BIT);
    SET_BIT(MFSEL2, MF_PWM1SEL_BIT);
    SET_BIT(MFSEL2, MF_PWM0SEL_BIT);
    
    /* set up the clock selector */
    CSR(0) = 0;
    CSR(1) = 0;
    
    /* set up the prescaler */
    PPP(0) = 0x0101;
    PPP(1) = 0x0101;
    
    /* set up: invert on/off, toggle mode/one-shot mode and PWM timer off */
    /* 3: one-shot mode/toggle mode(1) \
       2: invert-off/invert-on(1) \
       1: reserved \
       0: disabled/enabled(1) */
    
    PCR(0) = 0x00008808;    /* xxx3-21x0 */
    PCR(1) = 0x00088008;    /* xxx7-65x4 */
    
    /* set up the comparator register */
    CMR(0, 0) = 0x66;
    CMR(0, 1) = 0x66;
    CMR(0, 2) = 0x66;
    /* CMR(0, 3) = 0x66; */
    CMR(1, 0) = 0x66;
    /* CMR(1, 1) = 0x66; */
    CMR(1, 2) = 0x66;
    CMR(1, 3) = 0x66;
    
    /* set up the counter register */
    CNR(0, 0) = 0xEE;
    CNR(0, 1) = 0xEE;
    CNR(0, 2) = 0xEE;
    /* CNR(0, 3) = 0xEE; */
    CNR(1, 0) = 0xEE;
    /* CNR(1, 1) = 0xEE; */
    CNR(1, 2) = 0xEE;
    CNR(1, 3) = 0xEE;
    
    /* set up the interrupt enable register */
    PIER(0) = 0;
    PIER(1) = 0;
    
    /* enable the PWM timer */
    /* 3: one-shot mode/toggle mode(1) \
       2: invert-off/invert-on(1) \
       1: reserved \
       0: disabled/enabled(1) */
    PCR(0) = 0x00009909;    /* xxx3-21x0 */
    PCR(1) = 0x00099009;    /* xxx7-65x4 */
    
    return 0;
}

/* fan control */
int skipper_slinky_fan_control(void)
{
    /* (0, 0) = PWM0 \
       (0, 1) = PWM1 \
       (0, 2) = PWM2 \
       (0, 3) = PWM3 \
       (1, 0) = PWM4 \
       (1, 1) = PWM5 \
       (1, 2) = PWM6 \
       (1, 3) = PWM7 */
    
    SET_BIT(MFSEL2, MF_PWM7SEL_BIT);
    SET_BIT(MFSEL2, MF_PWM6SEL_BIT);
    /* SET_BIT(MFSEL2, MF_PWM5SEL_BIT); */
    /* SET_BIT(MFSEL2, MF_PWM4SEL_BIT); */
    
    SET_BIT(MFSEL2, MF_PWM3SEL_BIT);
    SET_BIT(MFSEL2, MF_PWM2SEL_BIT);
    SET_BIT(MFSEL2, MF_PWM1SEL_BIT);
    SET_BIT(MFSEL2, MF_PWM0SEL_BIT);
    
    /* set up the clock selector */
    CSR(0) = 0;
    CSR(1) = 0;
    
    /* set up the prescaler */
    PPP(0) = 0x0101;
    PPP(1) = 0x0101;
    
    /* set up: invert on/off, toggle mode/one-shot mode and PWM timer off */
    /* 3: one-shot mode/toggle mode(1) \
       2: invert-off/invert-on(1) \
       1: reserved \
       0: disabled/enabled(1) */
    
    PCR(0) = 0x00008808;    /* xxx3-21x0 */
    PCR(1) = 0x00088008;    /* xxx7-65x4 */
    
    /* set up the comparator register */
    CMR(0, 0) = 0x66;
    CMR(0, 1) = 0x66;
    CMR(0, 2) = 0x66;
    CMR(0, 3) = 0x66;
    /* CMR(1, 0) = 0x66; */
    /* CMR(1, 1) = 0x66; */
    CMR(1, 2) = 0x66;
    CMR(1, 3) = 0x66;
    
    /* set up the counter register */
    CNR(0, 0) = 0xEE;
    CNR(0, 1) = 0xEE;
    CNR(0, 2) = 0xEE;
    CNR(0, 3) = 0xEE;
    /* CNR(1, 0) = 0xEE; */
    /* CNR(1, 1) = 0xEE; */
    CNR(1, 2) = 0xEE;
    CNR(1, 3) = 0xEE;
    
    /* set up the interrupt enable register */
    PIER(0) = 0;
    PIER(1) = 0;
    
    /* enable the PWM timer */
    /* 3: one-shot mode/toggle mode(1) \
       2: invert-off/invert-on(1) \
       1: reserved \
       0: disabled/enabled(1) */
    PCR(0) = 0x00009909;    /* xxx3-21x0 */
    PCR(1) = 0x00099009;    /* xxx7-65x4 */
    
    return 0;
}



int platform_id_init(void)
{
#ifdef CONFIG_WPCM450_WHOVILLE
    /* platform ID, 0xC4000003 bit 0-3, CPLD base 0xC4000000 */
    platform_id = *((UINT8 *) 0xC4000003) & 0x0F;
    /* extend platform ID to 5 bits. 0xC4000022 bit2 is the bit4 of platform ID */
    platform_id |= (*((UINT8 *) 0xC4000022) & 0x4) << 2;
#endif
    
    return 0;
}


int manuf_mode_init(void)
{
#ifdef CONFIG_WPCM450_WHOVILLE
    /* select GPIO58 and set as input */
    CLEAR_BIT(MFSEL1, MF_R1MDSEL_BIT);
    CLEAR_BIT(GP3CFG0, GPIO58);
#endif
    
    return 0;
}


/* retrun 1: yes, 0: no */
int manuf_mode_read(void)
{
#ifdef CONFIG_WPCM450_WHOVILLE
    if (READ_BIT(GP3DIN, GPIO58))
    {
        return 0;
    }
#endif
    
    /* manufacture mode on */
    return 1;
}


int board_init (void)
{
    /* arch number */
    gd->bd->bi_arch_number = MACH_TYPE_WPCM450;
    
	/* location of boot parameters */
	gd->bd->bi_boot_params = 0x100;
	
    /* read platform ID information */
    platform_id_init();
	
    return 0;
}


/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    
    switch(GET_FIELD(PWRON, PWRON_RAM_SIZE))
    {
        /* Skip all initializations and jump to SPI flash directly with no check */
        case MEM_SKIP_INIT_MODE:
            
#ifdef CONFIG_WPCM450_WHOVILLE

#ifdef CONFIG_WPCM450_WHOVILLE_X00
            gd->bd->bi_dram[0].size = 0x4000000; /* default ram size is 64 MB */
#else
            gd->bd->bi_dram[0].size = 0x8000000; /* default ram size is 128 MB */
#endif

#else
            gd->bd->bi_dram[0].size = 0;
#endif
            break;
            
        case MEM_128MB_MODE:    /* 128 MB. */
            gd->bd->bi_dram[0].size = 0x8000000;
            break;
            
        case MEM_64MB_MODE:     /* 64 MB.  */
            gd->bd->bi_dram[0].size = 0x4000000;
            break;
            
        case MEM_32MB_MODE:     /* 32 MB.  */
            gd->bd->bi_dram[0].size = 0x2000000;
            break;
            
        default:
            gd->bd->bi_dram[0].size = 0;
            break;
    }
    
    return 0;
}


/*
 * Check Board Identity
 */
int checkboard(void)
{
    char *s = getenv("serial#");
    
    puts("Board: WPCM450");
    
    /* attach platform ID */
    if (platform_id != PF_ID_UNKNOWN)
    {
        printf("_%02x", platform_id);
    }
    
    if (s != NULL) 
    {
        puts(", serial# ");
        puts(s);
    }
    
    putc('\n');
    
    return (0);
}
// This change is made only for the platforms Brutus, Clover, Zax and Zooks to
// eliminate noise seen on the signls from WPCM450 to DRAM
int Brutus_Clover_DRAM_Init(void)
{
        MC_CFG3 = 0x06; //Bit 1 is set to 1 for DRAM Drive strength 60% //Change DRAM Drive Strength to 60% -> register address 0xB000100Ch, set bit 1= "1"
        MC_DLL_0 = 0x0001040B;  // Bits 9,8 are set to 0 //Change ODT setting to OFF -> Both registers address 0XB0001060h / 0xB0001064h, set bit 9~8= "00"
        MC_DLL_1 = 0x0000000B;  // DLL Delay change
        return 0;
}

/* miscellaneous platform dependent initialisations */
int misc_init_r (void)
{
    /* enable VGS core to decode IO addresses */
    SET_BIT(INTCR, INTCR_VGAIOEN_BIT);
    
    /* set to normal LPC access, the bit is set to clear */
    SET_BIT(SMC_CTL, SMC_CTL_HOSTWAIT);
    
    /* define LPC address when BADDR straps 6-5 set as 1-0.*/
    HIFCR = 0xCAE;
    
    /* check persistent stograge records */
    ps_check();
    
#ifdef CONFIG_WPCM450_WHOVILLE
    
    /* deselect PWM pins to avoid BMC driving */
    CLEAR_BIT(MFSEL2, MF_PWM7SEL_BIT);
    CLEAR_BIT(MFSEL2, MF_PWM6SEL_BIT);
    
    CLEAR_BIT(MFSEL2, MF_PWM5SEL_BIT);
    CLEAR_BIT(MFSEL2, MF_HG1SEL_BIT);
    
    CLEAR_BIT(MFSEL2, MF_PWM4SEL_BIT);
    CLEAR_BIT(MFSEL2, MF_HG0SEL_BIT);
    
    CLEAR_BIT(MFSEL2, MF_PWM3SEL_BIT);
    CLEAR_BIT(MFSEL2, MF_PWM2SEL_BIT);
    CLEAR_BIT(MFSEL2, MF_PWM1SEL_BIT);
    CLEAR_BIT(MFSEL2, MF_PWM0SEL_BIT);
    
    /* initiate fans */
    if ((platform_id == PF_ID_THIDWICK) || (platform_id == PF_ID_DIAMAS) || (platform_id == PF_ID_COASTER))
    {
        fan_control();
    }
    else if((platform_id == PF_ID_SKIPPER) || (platform_id == PF_ID_SLINKY))
    {
        skipper_slinky_fan_control();
    }
    else if((platform_id == PF_ID_BRUTUS) || (platform_id == PF_ID_CLOVER) || (platform_id == PF_ID_ZAX) || (platform_id == PF_ID_ZOOKS))
    {
        Brutus_Clover_DRAM_Init();
    }
    
    /* setup CLPD X-Bus timing */
    XBCR = 0;
    
    /* initiate MASER DC presence pin, GPI111 */
    /* select GPI 113-108, GPIO 127-120, GPIO 40-37 */
    SET_FIELD(MFSEL1, MF_DVOSEL, 0x00);
    
    /* read GPI 111 status */
    /* READ_BIT(GP6DIN, GPI111); */
    
    
    /* setup the pin pull-up/down enable register */
    CLEAR_BIT(MISCPE, MISCPE_NSPICS);
    CLEAR_BIT(GPIOP5PE, GPIO80);
    CLEAR_BIT(GPIOP5PE, GPIO81);
    CLEAR_BIT(GPIOP5PE, GPIO82);
    CLEAR_BIT(GPIOP5PE, GPIO83);
    CLEAR_BIT(GPIOP7PE, GPIO121);
    CLEAR_BIT(GPIOP7PE, GPIO123);
    SET_BIT(GP7CFG2, GPIO123);
    
    
    /* pull high GPIO125 (BMC_SMI_N) and GPIO127 (BMC_RDY_N) */
    SET_BIT(GP7DOUT, GPIO125);
    SET_BIT(GP7CFG0, GPIO125);
    SET_BIT(GP7DOUT, GPIO127);
    SET_BIT(GP7CFG0, GPIO127);
    
    
    /* enable input debounce function for GPIOE6 */
    CLEAR_BIT(GP0CFG0, GPIOE6);
    SET_BIT(GPEVDBNC, GPIOE6);
    
    
    /* set GPIO124 to low to enable mux output */
    CLEAR_BIT(GP7DOUT, GPIO124);
    
    /* output buffer enable */
    SET_BIT(GP7CFG0, GPIO124);
    
    
    /* initiate MASER DC eMMC power control, GPIO59*/
    CLEAR_BIT(MFSEL2, MF_HG6SEL_BIT);
    
    /* check if MASER DC is present, 0 -> present */
    if (READ_BIT(GP6DIN, GPI111))
    {
        /* set GPIO59 to high to block power to MASER DC SD connector */
        SET_BIT(GP3DOUT, GPIO59);
    }
    else
    {
        /* set GPIO59 to low to apply power to MASER DC SD connector */
        CLEAR_BIT(GP3DOUT, GPIO59);
    }
    
    /* output buffer enable */
    SET_BIT(GP3CFG0, GPIO59);
    
    
    /* initiate AMEA SD power control, select GPIO63 */
    CLEAR_BIT(MFSEL1, MF_HSP1SEL_BIT);
    
    /* set GPIO63 to low to block power to AMEA SD connector */
    CLEAR_BIT(GP3DOUT, GPIO63);
    
    /* set GPIO63 to high to apply power to AMEA SD connector */
    /* SET_BIT(GP3DOUT, GPIO63); */
    
    /* output buffer enable */
    SET_BIT(GP3CFG0, GPIO63);
#endif
    
    /* initiate SD/MMC host controller and detect card type and frequence */
    /* mmc_init(0); */
    
    /* detect/initiate firmware upgrade module */
    fwu_init();
    
    /* initate manufacture mode pin */
    manuf_mode_init();
    
    /* check manufacture mode status */
    if (manuf_mode_read() == 0)
    {
        /* force to disable command prompt if manufacture mode is off */
        setenv("bootdelay", "0");
    }
    
    return (0);
}


#if 0
int interrupt_init (void)
{
    wpcm450_aic_initialize();
}
#endif


/* common function */
int wpcm450_progress_chart(char *prefix, UINT8 type, UINT8 percentage)
{
    UINT8 i;
    UINT8 finished;
    
    if (percentage > 100)
    {
        percentage = 100;
    }
    
    finished = percentage / 5;
    
    /* print prefix string */
    printf("\r%s |", prefix);
    
    /* handle finished portion */
    for (i = 0; i < 20; i++)
    {
        if (i < finished)
        {
            printf("#");
        }
        else
        {
            printf("-");
        }
    }
    
    printf("|%4d\%", percentage);
    
    return 0;
}


int wpcm450_print_size(char* prefix, UINT32 size)
{
    if (size < (1024))
    {
        /* (size) */
        printf("\r%s %4ld B ", prefix, size);
    }
    else if (size < (1024 * 1024))
    {
        /* (size / 1024) */
        printf("\r%s %4ld KB", prefix, (size >> 10));
    }
    else if (size < (1024 * 1024 * 1024))
    {
        /* (size / 1024 / 1024) */
        printf("\r%s %4ld MB", prefix, (size >> 10 >> 10));
    }
    else
    {
        /* (size / 1024 / 1024 / 1024) */
        printf("\r%s %4ld GB", prefix, (size >> 10 >> 10 >> 10));
    }
    
    return 0;
}
