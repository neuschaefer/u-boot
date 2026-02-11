/*
 * (C) Copyright 2011
 * Broadcom Inc
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>

#include <asm/arch/ccu_inline.h>

int do_a9freqset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   uint32_t freq;
   uint32_t freq_id;
   uint32_t freq_mhz;
   uint32_t div;
   
   freq = simple_strtoul( argv[1], NULL, 0 );
        
   /* switch to policy 5 first */
   ccu_set_kproc_policy_freq( ccu_policy_3, ccu_kproc_policy_freq_312_156_104_1V0 );

   /* change PLL to get 1.2GHz */
   ccu_set_kproc_pll(92, 322638, 1, 2);
      
   switch( freq )
   {
      case 26:
         ccu_set_kproc_policy_freq( ccu_policy_3, ccu_kproc_policy_freq_xtal );
         break;
      case 52:
         ccu_set_kproc_policy_freq( ccu_policy_3, ccu_kproc_policy_freq_52_52_52 );
         break;        
      case 156:
         ccu_set_kproc_policy_freq( ccu_policy_3, ccu_kproc_policy_freq_156_156_78_0V9 );
         break;
      case 312:
         ccu_set_kproc_policy_freq( ccu_policy_3, ccu_kproc_policy_freq_312_156_104 );
         break;
      case 600:
         ccu_set_kproc_mdiv( ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3, 4 );
         ccu_set_kproc_policy_freq( ccu_policy_3, ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3 );
         break;
      case 800:
         ccu_set_kproc_mdiv( ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3, 3 );
         ccu_set_kproc_policy_freq( ccu_policy_3, ccu_kproc_policy_freq_aclk_aclkdiv2_aclkdiv3 );
         break;
      case 1200:
         ccu_set_kproc_mdiv( ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3, 2 );
         ccu_set_kproc_policy_freq( ccu_policy_3, ccu_kproc_policy_freq_aclkh_aclkhdiv2_aclkhdiv3 );
         break;
      default:
         printf( "Frequency not supported.  Supported frequencies [26|52|156|312|600|800|1200]\n" );
         return;
         break;
   }

   freq_id  = ccu_get_kproc_policy_freq(ccu_policy_3);
   freq_mhz = ccu_get_kproc_policy_freq_a9_hz(freq_id) / 1000000;
   div      = ccu_get_kproc_axi_div(freq_id)+1;

   printf("\nCPU Info:\n");
   printf("    Freq ID  = %4d\n", freq_id);
   printf("    Cpu freq = %4d MHz\n", freq_mhz);
   printf("    AXI freq = %4d MHz (Cpu freq divided by %d)\n", freq_mhz/div, div);
   
   return( 1 );
}

U_BOOT_CMD(
   a9freq, 2, 0, do_a9freqset,
   "a9freq <freq>\n",
   "[26|52|156|312|600|800|1200]\n"
);


   