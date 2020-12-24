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


#define CONSOLE_DEBUG_MODE              0
#define CONSOLE_USER_MODE               1
#define CONSOLE_SKIP_BOOT_MODE          2


typedef enum
{
    /* group 0 */
    CONSOLE_EXIT,
    
    /* group 1 */
    CONSOLE_VERSION,
    CONSOLE_NAME,
    CONSOLE_MAC_ADDRESS,
    CONSOLE_SYSTEM_IP,
    CONSOLE_SYSTEM_NETMASK,
    
    /* group 2 */
    CONSOLE_SYSTEM_GATEWAYIP,
    CONSOLE_TFTP_SERVER_IP,
    CONSOLE_IMAGE_FILE_NAME,
    
    /* group 3 */
    CONSOLE_DHCP_CLIENT,
#if 0
    CONSOLE_FDISK,
#endif
    CONSOLE_FIRMWARE_UPGRADE,
    CONSOLE_RESET,
    
    /* group 4 */
    CONSOLE_BYPASS,
} console_index_type;


/* structure of OEM console information */
typedef struct console_info
{
    UINT8 mode;
    /* UINT8 dhcp_enable; */
}console_info_type;
