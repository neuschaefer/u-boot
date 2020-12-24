/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 platform definitions.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


/*
0000   = Bluefish
0001   = Thidwick
0010   = Sneetch

0011   = Mack

0011   = Onceler blade      (bottom 2 bits hard coded
0100   = Barbaloot blade    (bottom 2 bits hard coded)

0101   = Zooks 
0110   = Zax                (top 3 bits hard coded)
0111   = McCave             (top 3 bits hard coded)

1000   = Yertle             
1001   = Cindy Lou
1010   = McBean             (no Super I/O)
1011   = Mayzie             (no Super I/O and no up-sell)
1100   = SamIAm             (no Super I/O)
1101   = Horton             (no Super I/O)

10101  = Diamas
10110  = Coaser
XXXX   = Redfish
XXXX   = Twiceler
*/


/*platform ID definitions */
#ifdef CONFIG_WPCM450_WHOVILLE
typedef enum
{
    PF_ID_BLUEFISH,
    PF_ID_THIDWICK,
    PF_ID_SNEETCH,
    
    PF_ID_MACK,
    
    /* blade */
    PF_ID_BARBALOOT,    /* blade (bottom 2 bits hard coded) */
    
    PF_ID_ZOOKS, 
    PF_ID_ZAX,          /* top 3 bits hard coded */
    PF_ID_MCCAVE,       /* top 3 bits hard coded */
    
    /* MASER Lite */
    PF_ID_YERTLE,     
    PF_ID_CINDYLOU,   
    PF_ID_MCBEAN,       /* no Super I/O */
    PF_ID_MAYZIE,       /* no Super I/O and no up-sell) */
    PF_ID_SAMIAM,       /* no Super I/O */
    PF_ID_HORTON,       /* no Super I/O */
    
    PF_UNUSED_0xE,      /* PF_ID_REDFISH, */
    PF_UNUSED_0xF,      /* PF_ID_TWICELER, */

    PF_ID_BRUTUS,       /* AMD platform G34*/
    PF_ID_CLOVER,       /* AMD platform G34*/

    PF_UNUSED_0x12,

    PF_ID_SKIPPER,      /* AMD platform C32*/
    PF_ID_SLINKY,       /* AMD platform C32*/
   	PF_ID_DIAMAS,		/* no Super I/O */
	PF_ID_COASTER,		/* no Super I/O */
	 
    /* default platform ID */
    PF_ID_UNKNOWN = 0xFF,
} platform_id_type;
#else
typedef enum
{
    /* default platform ID */
    PF_ID_UNKNOWN = 0xFF,
} platform_id_type;
#endif
