/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 OEM console driver.
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
#include "command.h"
#include "config.h"

#include "cdefs.h"
#include "com_defs.h"

#include "wpcm450_console.h"
#include "wpcm450_platform.h"
#include "wpcm450_gctrl_regs.h"


#if 0
#define PRINTD(fmt,args...) printf("CONSOLE: " fmt ,##args)
#else
#define PRINTD(fmt,args...)
#endif


/* console buffer */
extern char console_buffer[CFG_CBSIZE];

/* version string */
extern char version_string[];

/* switch serial port */
extern int serial_change_port(UINT8 port);
extern int serial_mux_restore(void);

/* manufacture mode */
extern int manuf_mode_read(void);

/* platform ID */
extern UINT8 platform_id;

/* check persistent storage */
extern int ps_check(void);


/* console information */
console_info_type console;


#define CONSOLE_XMK_STR(x)  #x
#define CONSOLE_MK_STR(x)   CONSOLE_XMK_STR(x)


#ifdef CONFIG_WPCM450_WHOVILLE
#define CONSOLE_PROMPT      "RECOVER> "
#endif

#ifndef CONSOLE_PROMPT
#define CONSOLE_PROMPT      "Please select an option ==> "
#endif


int console_oem_precheck(UINT8 option)
{
#ifdef CONFIG_WPCM450_WHOVILLE
    if ((platform_id != PF_ID_YERTLE)
        && (platform_id != PF_ID_CINDYLOU)
        && (platform_id != PF_ID_MCBEAN)
        && (platform_id != PF_ID_MAYZIE)
        && (platform_id != PF_ID_SAMIAM)
        && (platform_id != PF_ID_HORTON)
        && (platform_id != PF_ID_DIAMAS)
        && (platform_id != PF_ID_COASTER)
        && (platform_id != PF_ID_MACK))
    {
        /* check if MASER DC is present, 0 -> present */
        if (READ_BIT(GP6DIN, GPI111))
        {
            printf("\n*** Required hardware is missing causing iDRAC to not be bootable ***\n");
            return -1;
        }
        else
        {
            return 0;
        }
    }
#endif
    
    return 0;
}


int console_print_menu(int mode)
{
    PRINTD("console_print_menu\n");
    
    printf("\n\n");
    printf("====  Firmware Recovery Option  ====\n");
    printf("\n");
    
    if (mode == CONSOLE_DEBUG_MODE)
    {
        printf("%3d. Exit OEM Console ......... Debug Mode\n", CONSOLE_EXIT);
        printf("\n");
    }
    
    printf("%3d. Bootloader Version ....... %s\n", CONSOLE_VERSION,
                                                   CONFIG_AVCT_VERSION "." \
                                                   CONFIG_AVCT_PATCHLEVEL "." \
                                                   CONFIG_AVCT_SUBLEVEL);
    printf("%3d. Ethernet Adapter ......... %s\n", CONSOLE_NAME, 
                                                   getenv("ethact"));
    printf("%3d. MAC Address .............. %s", CONSOLE_MAC_ADDRESS, 
                                                 getenv("ethaddr"));
    
#if 0
    /* mark the MAC address as read only if the MAC address is not the same as
       code default */
    if (strcmp(getenv("ethaddr"), 
        CONSOLE_MK_STR(CONFIG_ETHADDR)"\0") == 0)
    {
        printf("\n");
    }
    else
    {
        printf(" [Read Only]\n");
    }
#else
    printf("\n");
#endif
    
    printf("\n");
    
    printf("%3d. IP Address ............... %s\n", CONSOLE_SYSTEM_IP, 
                                                   getenv("ipaddr"));
    printf("%3d. Subnet Mask .............. %s\n", CONSOLE_SYSTEM_NETMASK, 
                                                   getenv("netmask"));
    printf("%3d. Gateway IP Address ....... %s\n", CONSOLE_SYSTEM_GATEWAYIP, 
                                                   getenv("gatewayip"));
    
    printf("\n");
    printf("%3d. TFTP Server IP Address ... %s\n", CONSOLE_TFTP_SERVER_IP, 
                                                   getenv("serverip"));
    printf("%3d. Image File Name .......... %s\n", CONSOLE_IMAGE_FILE_NAME, 
                                                   getenv("fwuimage"));
    printf("\n");
    printf("%3d. Enable DHCP Client\n", CONSOLE_DHCP_CLIENT);
#if 0
    printf("%3d. Create Default Partition Table\n", CONSOLE_FDISK);
#endif
    printf("%3d. Execute Firmware Upgrade\n", CONSOLE_FIRMWARE_UPGRADE);
    printf("%3d. Reset\n", CONSOLE_RESET);
    printf("\n");
    
    printf("%3d. Bypass Firmware Recovery\n", CONSOLE_BYPASS);
#ifdef CONFIG_WPCM450_WHOVILLE
    printf("     - This option reverts serial device 2 to be connected\n" \
           "       to the external serial connector.\n");
#endif
    printf("\n");
    
    return 0;
}


int console_print_info(int mode)
{
    PRINTD("console_print_info\n");
    
    /* print console menu */
    console_print_menu(mode);
    
    return 0;
}


int console_update_env(char *message, 
                       char *env_variable, 
                       int min_len, 
                       int max_len)
{
    char command[CFG_CBSIZE] = {0, };
    int len;
    
    PRINTD("console_update_env\n");
    
    printf("\n");
    printf("Please enter %s ==> ", message);
    
    /* wait for input */
    len = readline("");
    
    if ((len >= min_len) && (len <= max_len))
    {
        /* copy command from console buffer */
        strcpy(command, console_buffer);
        
        setenv(env_variable, command);
    }
    else
    {
        return -1;
    }
    
    return len;
}


int console_loop(int mode)
{
    int ret;
    char command[CFG_CBSIZE] = {0, };
    int len;
    UINT8 option;
    UINT8 i;
    UINT8 abort;
    UINT32 delay;
    
    PRINTD("console_loop\n");
    
    while (1)
    {
        /* wait for input */
        /* len = readline(CFG_PROMPT); */
        len = readline(CONSOLE_PROMPT);
        
        /* copy command from console buffer */
        strcpy(command, console_buffer);
        
        PRINTD("received command, len=%d\n", len);
        
        if ((len > 0) && (len < 3))
        {
            /* convert string to unsigned char */
            option = (UINT8) simple_strtoul(command, NULL, 10);
            
            /* OEM specific checking */
            if (console_oem_precheck(option))
            {
                continue;
            }
            
            switch (option)
            {
                case CONSOLE_EXIT:
                    if (mode == CONSOLE_DEBUG_MODE)
                    {
                        PRINTD("exit OEM console\n");
                        return 0;
                    }
                    
                    break;
                    
                case CONSOLE_DHCP_CLIENT:
                    ret = run_command("dhcp", 0);
                    
                    PRINTD("ret=%d\n", ret);
                    
                    if (ret < 0)
                    {
                        printf("*** fail to execute DHCP ***\n");
                    }
                    
                    break;
                    
                case CONSOLE_SYSTEM_IP:
                    console_update_env("IP Address", "ipaddr", 7, 15);
                    break;
                    
                case CONSOLE_SYSTEM_NETMASK:
                    console_update_env("Subnet Mask", "netmask", 7, 15);
                    break;
                    
                case CONSOLE_SYSTEM_GATEWAYIP:
                    console_update_env("Gateway IP Address", "gatewayip", 7, 15);
                    break;
                    
                case CONSOLE_MAC_ADDRESS:
#if 0
                    if (strcmp(getenv("ethaddr"), 
                               CONSOLE_MK_STR(CONFIG_ETHADDR)"\0") == 0)
                    {
                        ret = console_update_env("MAC Address", "ethaddr", 11, 17);
                        
                        if (ret <= 0)
                        {
                            /* if there is no input data */
                            break;
                        }
                        
                        /* reset command buffer */
                        command[0] = '\0';
                        
                        strcpy(command, "ps set 1 ");
                        strcat(command, getenv("ethaddr"));
                        
                        ret = run_command(command, 0);
                        
                        PRINTD("command[]=%s\n", command); 
                        PRINTD("ret=%d\n", ret);
                        
                        if (ret < 0)
                        {
                            printf("*** fail to set MAC address ***\n");
                        }
                    }
#else
                    /* MAC address can only be changed in manufacture mode */
                    if (manuf_mode_read() == 0)
                    {
                        break;
                    }
                    
                    ret = console_update_env("MAC Address", "ethaddr", 11, 17);
                    
                    if (ret <= 0)
                    {
                        /* if there is no valid input data */
                        break;
                    }
                    
                    /* reset command buffer */
                    command[0] = '\0';
                    
                    strcpy(command, "ps set 1 ");
                    strcat(command, getenv("ethaddr"));
                    
                    ret = run_command(command, 0);
                    
                    PRINTD("command[]=%s\n", command); 
                    PRINTD("ret=%d\n", ret);
                    
                    if (ret < 0)
                    {
                        printf("*** fail to set MAC address ***\n");
                    }
#endif
                    
                    break;
                    
                case CONSOLE_TFTP_SERVER_IP:
                    console_update_env("TFTP Server IP Address", "serverip", 7, 15);
                    break;
                    
                case CONSOLE_IMAGE_FILE_NAME:
                    console_update_env("Image File Name", "fwuimage", 1, 12);
                    break;
                    
#if 0
                case CONSOLE_FDISK:
                    ret = run_command("fwu fdisk default", 0);
                    
                    PRINTD("ret=%d\n", ret);
                    
                    if (ret < 0)
                    {
                        printf("*** fail to create default partition ***\n");
                    }
                    
                    break;
#endif
                    
                case CONSOLE_FIRMWARE_UPGRADE:
                    
                    printf("\n\nDownloading image from TFTP server ...\n\n");
                    
                    sprintf(command, 
                            "tftp %s %s%s", 
                            getenv("offset"), 
                            getenv("rootpath"), 
                            getenv("fwuimage"));
                    
                    ret = run_command(command, 0);
                    
                    PRINTD("ret=%d\n", ret);
                    
                    if (ret < 0)
                    {
                        printf("*** check network setting ***\n");
                        break;
                    }
                    
                    /* flash N-1 image */
                    printf("\n\nChecking and flashing N-1 image ...\n\n");
                    
                    printf("Checking partition of N-1 image ... ");
                    sprintf(command, "fwu act %d", 3);
                    
                    ret = run_command(command, 0);
                    
                    if (ret < 0)
                    {
                        printf("\n*** fail to swap to N-1 image ***\n");
                        break;
                    }
                    
                    printf("\n");
                    
                    sprintf(command, "fwu update %s", getenv("offset"));
                    
                    ret = run_command(command, 0);
                    
                    PRINTD("ret=%d\n", ret);
                    
                    if (ret < 0)
                    {
                        printf("*** fail to execute firmware upgrade ***\n");
                        break;
                    }
                    
                    /* sync up persistent storage */
                    printf("\n\nSyncing up persistent storage ...\n");
                    ps_check();
                    
                    /* flash N image */
                    printf("\n\nChecking and flashing N image ...\n\n");
                    
                    printf("Checking partition of N image ... ");
                    sprintf(command, "fwu act %d", 1);
                    
                    ret = run_command(command, 0);
                    
                    if (ret < 0)
                    {
                        printf("\n*** fail to swap to N image ***\n");
                        break;
                    }
                    
                    printf("\n");
                    
                    sprintf(command, "fwu update %s", getenv("offset"));
                    
                    ret = run_command(command, 0);
                    
                    PRINTD("ret=%d\n", ret);
                    
                    if (ret < 0)
                    {
                        printf("*** fail to execute firmware upgrade ***\n");
                        break;
                    }
                    
                    printf("\n\nErasing private storage ...\n\n");
                    
                    ret = run_command("run erase_ps", 0);
                    
                    if (ret < 0)
                    {
                        printf("*** fail to erase private storage ***\n");
                    }
                    
                    printf("\n\nErasing bootloader environment ...\n\n");
                    
                    ret = run_command("run erase_env", 0);
                    
                    if (ret < 0)
                    {
                        printf("*** fail to erase bootloader environment ***\n");
                    }
                    
                    /* delay 3 seconds before resetting */
                    delay = 3000;
                    abort = 0;
                    
                    printf("\n\nHit any key to stop reset:   ");
                    
                    while ((delay > 0) && !abort)
                    {
                        printf("\b\b%2d", (delay / 1000));
                        
                        for (i = 0; i < 100; i++)
                        {
                            /* test if we got a key press */
                            if (tstc())
                            {
                                /* consume input	*/
                                (void) getc();
                                abort = 1;
                                break;
                            }
                            
                            udelay(10000);
                            delay -= 10;
                        }
                    }
                    
                    printf("\b\b 0\n\n");
                    
                    if (!abort)
                    {
                        /* restore previous mux position */
                        serial_mux_restore();
                        
                        ret = run_command("reset", 0);
                        
                        PRINTD("ret=%d\n", ret);
                        
                        if (ret < 0)
                        {
                            printf("*** fail to reset system ***\n");
                        }
                    }
                    
                    break;
                    
                case CONSOLE_RESET:
                    
                    /* restore previous mux position */
                    serial_mux_restore();
                    
                    ret = run_command("reset", 0);
                    
                    PRINTD("ret=%d\n", ret);
                    
                    if (ret < 0)
                    {
                        printf("*** fail to reset system ***\n");
                    }
                    
                    break;
                    
                case CONSOLE_BYPASS:
                    
                    printf("\n*** bypass firmware recovery ***\n");
                    
                    /* restore previous mux position */
                    serial_mux_restore();
                    
                    break;
                    
                default:
                    
                    break;
            }
        }
        else if (strcmp(command, "printubootversion") == 0)
        {
            PRINTD("print U-Boot version\n");
            
            printf("\n%s\n", version_string);
        }
        else if (strcmp(command, version_string) == 0)
        {
            PRINTD("exit OEM console\n");
            
            return 0;
        }
        else
        {
            PRINTD("no match\n");
        }
        
        /* reflash menu */
        console_print_menu(mode);
    }
    
    return 0;
}


int console_init(int mode)
{
    int ret = 0;
    
    PRINTD("console_init\n");
    
    if (mode == CONSOLE_USER_MODE)
    {
        /* try to boot into firmware */
        ret = run_command("fwu boot", 0);
    }
    
    PRINTD("ret=%d\n", ret);
    
    if ((ret) 
        || (mode == CONSOLE_DEBUG_MODE) 
        || (mode == CONSOLE_SKIP_BOOT_MODE))
    {
        PRINTD("enter console loop\n");
        
        printf("*** swtich to OEM console ***\n");
        
        if (mode != CONSOLE_DEBUG_MODE)
        {
#ifdef CONFIG_WPCM450_WHOVILLE
            /* force to switch serial port if there is no firmware to boot */
            serial_change_port(1);
#endif
        }
        
        /* initiate default setting */
        console.mode = mode;
        /* console.dhcp_enable = 0; */
        
        /* print console menu */
        console_print_menu(mode);
        
        /* enter console loop */
        console_loop(mode);
    }
    
    return 0;
}


int do_console(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int ret;
    
    switch (argc) 
    {
        case 0:
        case 1:
            break;
            
        case 2:
        case 3:
            if (strcmp(argv[1], "info") == 0) 
            {
                ret = console_print_info(console.mode);
                
                if (ret) 
                {
                    printf("*** fail to print OEM console setting ***\n");
                    return ret;
                }
                
                return 0;
            }
            else if (strcmp(argv[1], "init") == 0) 
            {
                if ((argc > 2) && (strcmp(argv[2], "debug") == 0))
                {
                    ret = console_init(CONSOLE_DEBUG_MODE);
                }
                else
                {
                    ret = console_init(CONSOLE_USER_MODE);
                }
                
                if (ret) 
                {
                    printf("*** fail to enable OEM console ***\n");
                    return ret;
                }
                
                return 0;
            }
            else if (strcmp(argv[1], "switch") == 0) 
            {
                ret 
                = serial_change_port((UINT8) simple_strtoul(argv[2], NULL, 10));
                
                if (ret) 
                {
                    printf("*** fail to switch serial port ***\n");
                    return ret;
                }
                
                return 0;
            }
            
            break;
            
        default:
            break;
    }
    
    /* the command is not correct */
    printf("Usage:\n%s\n", cmdtp->usage);
    
    return 1;
}


U_BOOT_CMD(
    console, 3, 1, do_console,
    "console - OEM console, type 'help console' for details\n",
    "info\n"
    "           - show current OEM console setting\n"
    "console init [debug]\n"
    "           - enable OEM console\n"
    "console switch [port]\n"
    "           - switch to serial 'port'\n"
);
