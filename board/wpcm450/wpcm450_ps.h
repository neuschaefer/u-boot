/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 persistent storage driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


#define PS_REC_MAC0             0
#define PS_REC_MAC1             1
#define PS_REC_VERSION          2
#define PS_REC_PLATFORM         3
#define PS_REC_FAILSAFE         4
#define PS_REC_MAX              4

#define PS_ERASE_ENV            0
#define PS_ERASE_REC            1

#define PS_CREATE_NEW           0
#define PS_CREATE_UPDATE        1

#define PS_PLATFORM_UNKNOWN     0xFFFFFFFF

#define PS_CURRENT_VERSION      3

/* LSB is jumper state indicator used by FW */
#define PS_FAILSAFE_LOCKOUT_MASK    0xFE
#define PS_FAILSAFE_LOCKOUT         0x5A


#pragma pack(1)

/* structure of persistent storage */
typedef struct ps_info
{
    UINT8 magic_number[4];      /* 0x1B8, 0x41 0x56 0x43 0x54 (AVCT) */
    UINT8 ps_ver;               /* version, 0x01 */
    UINT8 reserved[3];          /* 0x00, 0x00 0x00 */
    UINT8 mac0[8];              /* mac address for emc 0 */   
    UINT8 mac1[8];              /* mac address for emc 1 */
    UINT8 uboot_ver[8];         /* 0-2: major 4-6: minor, in BCD */
    UINT8 platform_id[4];       /* platfrom id, 0xFFFFFFFF -> unspecified */
    UINT8 u8FailsafeFlag;       /* Failsafe lockout flag  */
} ps_info_type;

#pragma pack()
