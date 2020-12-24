/*
 * Board specific setup info
 *
 * (C) Copyright 2003, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
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

#include <config.h>
#include <version.h>

.globl lowlevel_init
lowlevel_init:

init_dram:
    mov r4, lr		/* save lr */

    /*Set Scratch register Bit 7 before initialize*/
    /* *(scu1+ 0) = 0xa8 */
    mov r1, #0xa8
    ldr r0, = 0x1e6e0100
    str r1, [r0]

    ldr r0, =0x1e6e0140
    ldr r1, [r0]
    orr r1, r1, #0x80
    str r1, [r0]

    /*Check Stratch Register Bit 6*/
    ldr r0, =0x1e6e0140
    ldr r1, [r0]
    bic r1, r1, #0xFFFFFFBF
    mov r2, r1, lsr #6
    cmp r2, #0x01
    beq platform_exit

    /* Check hardware Trapping */
    /* Bit 24~30 reserved for aspeed,  if Bit 31 is 1 we don't initialize ddr */
    ldr r0, =0x1e6e0170
    ldr r1, [r0]
    ldr r2, [r0]

    bic r2, r2, #0x7FFFFFFF
    mov r3, r2,lsr #31
    cmp r3, #0x01
    beq platform_exit

    bic r1, r1, #0xF8FFFFFF
    mov r2, r1,lsr #24

    /* Initialize giob dir */
    mov r1, #0x0f
    ldr r0, =0x1e8a000c
    str r1, [r0]

    /* Light LED */
    ldr r0, = 0x1e8a0008
    str r2, [r0]

    /* DRAM Timing Select */
    cmp r2, #0x00
    bleq dram_set_0  	/* 100 Mhz setting 1 */
    
    cmp r2, #0x01
    bleq dram_set_1  	/* 100 Mhz setting 2 */

    cmp r2, #0x02
    bleq dram_set_2  	/* 133 Mhz */

    cmp r2, #0x03
    bleq dram_set_3  	/* 166 Mhz */

    cmp r2, #0x04    	/* 184 Mhz */
    bleq dram_set_4

    cmp r2, #0x07    	/* 200 Mhz */
    bleq dram_set_5

    /*Set Scratch register Bit 6 after ddr initial finished */
    ldr r0, =0x1e6e0140
    ldr r1, [r0]
    orr r1, r1, #0x40
    str r1, [r0]

platform_exit:
    mov lr, r4		/* restore lr */
 
    /* back to arch calling code */    
    mov pc, lr

/*
 *************************************************************************
 *
 * DRAM Setting Modules
 *
 *
 *************************************************************************
 */
dram_set_0:
    /* *(scu1+ 0) = 0xa8 */
    mov r1, #0xa8  
    ldr r0, = 0x1e6e0100 
    str r1, [r0] 

    /* *(scu1+ 8) = 0x0000cc41 */
    ldr r1, =0x0000cc41 
    ldr r0, =0x1e6e0120 
    str r1, [r0] 

    /*Delay about 10ms */
    ldr r2, =0x00010000 
delay0:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay0	 
	
    /* *(ddr_setting+ 0) = 0x01;*/
    mov r1, #0x01
    ldr r0, =0x1e6e0000
    str r1, [r0]

    /* *(ddr_setting+ 1) = 0x89;*/
    mov r0, #0x89
    ldr r1, =0x1e6e0004
    str r0,[r1]

    /*  *(ddr_setting+ 2) = 0x43441454; */
    ldr r0, =0x43441454
    ldr r1, =0x1e6e0008
    str r0, [r1]

    /* *(ddr_setting+ 3) = 0x110a0310; */
    ldr r0, =0x110a0310
    ldr r1, =0x1e6e000c
    str r0, [r1]
	
    /* *(ddr_setting+ 4) = 0x33337777; */
    ldr r0, =0x33337777
    ldr r1, =0x1e6e0010
    str r0,[r1]

    /* *(ddr_setting+ 8) = 0x00140354; */
    ldr r0, =0x00140354
    ldr r1, =0x1e6e0020
    str r0,[r1]

    /* *(ddr_setting+ 9) = 0x000000001; */
    ldr r0, =0x00000001
    ldr r1, =0x1e6e0024
    str r0,[r1]

    /* *(ddr_setting+ 10) = 0x0d063cf0; */
    ldr r0, =0x0d063cf0
    ldr r1, =0x1e6e0028
    str r0,[r1]
    
    /* *(ddr_setting+ 7) = 0x00; */
    mov r0, #0x00
    ldr r1, =0x1e6e001c
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x03;*/
    mov r0, #0x03
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /*  *(ddr_setting+ 6) = 0x131; */
    ldr r0, =0x131
    ldr r1, =0x1e6e0018
    str r0,[r1]
    
    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /* *(ddr_setting+ 6) = 0x31; */
    mov r0, #0x31
    ldr r1, =0x1e6e0018
    str r0,[r1]
                                                                                
    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]
                                                                                                                                               
    /* *(ddr_setting+ 9) = 0x03; */
    mov r0, #0x03
    ldr r1, =0x1e6e0024
    str r0, [r1]

    /* *(ddr_setting+ 10) = 0xd063cf1;*/
    ldr r0, =0x0d063cf1
    ldr r1, =0x1e6e0028
    str r0, [r1]

    /* *(ddr_setting+ 12) = 0xfffffe01;*/
    ldr r0, =0xfffffe01
    ldr r1, =0x1e6e0030
    str r0, [r1]

    mov pc, lr      /* back to my caller */

dram_set_1:
    /* *(scu1+ 0) = 0xa8 */
    mov r1, #0xa8  
    ldr r0, = 0x1e6e0100 
    str r1, [r0] 

    /* *(scu1+ 8) = 0x0000cc41 */
    ldr r1, =0x0000cc41 
    ldr r0, =0x1e6e0120 
    str r1, [r0] 

    /*Delay about 10ms */
    ldr r2, =0x00010000
delay1:
    nop
    nop
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay1
	 	
    /* *(ddr_setting+ 0) = 0x01;*/
    mov r1, #0x01
    ldr r0, =0x1e6e0000
    str r1, [r0]

    /* *(ddr_setting+ 1) = 0x89;*/
    mov r0, #0x89
    ldr r1, =0x1e6e0004
    str r0,[r1]

    /*  *(ddr_setting+ 2) = 0x11221343; */
    ldr r0, =0x11221343
    ldr r1, =0x1e6e0008
    str r0, [r1]

    /* *(ddr_setting+ 3) = 0x8040007; */
    ldr r0, =0x8040007
    ldr r1, =0x1e6e000c
    str r0, [r1]
	
    /* *(ddr_setting+ 4) = 0x33333333; */
    ldr r0, =0x33333333
    ldr r1, =0x1e6e0010
    str r0,[r1]

    /* *(ddr_setting+ 8) = 0x00130350; */
    ldr r0, =0x00130350
    ldr r1, =0x1e6e0020
    str r0,[r1]

    /* *(ddr_setting+ 9) = 0x000000001; */
    ldr r0, =0x00000001
    ldr r1, =0x1e6e0024
    str r0,[r1]
                                                                                                    
    /* *(ddr_setting+ 10) = 0xd0435f0; */
    ldr r0, =0xd0435f0
    ldr r1, =0x1e6e0028
    str r0,[r1]

    /* *(ddr_setting+ 7) = 0x00; */
    mov r0, #0x00
    ldr r1, =0x1e6e001c
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x03;*/
    mov r0, #0x03
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /*  *(ddr_setting+ 6) = 0x131; */
    ldr r0, =0x131
    ldr r1, =0x1e6e0018
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /* *(ddr_setting+ 6) = 0x31; */
    mov r0, #0x31
    ldr r1, =0x1e6e0018
    str r0,[r1]
                                                                               
    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]
                                                                                                                                               
    /* *(ddr_setting+ 9) = 0x03; */
    mov r0, #0x03
    ldr r1, =0x1e6e0024
    str r0, [r1]

    /* *(ddr_setting+ 10) = 0xd0435f1;*/
    ldr r0, =0x0d0435f1
    ldr r1, =0x1e6e0028
    str r0, [r1]
    
    /* *(ddr_setting+ 12) = 0xfffffe01;*/
    ldr r0, =0xfffffe01
    ldr r1, =0x1e6e0030
    str r0, [r1]

    mov pc, lr      /* back to my caller */

dram_set_2:
    /* *(scu1+ 0) = 0xa8 */
    mov r1, #0xa8  
    ldr r0, = 0x1e6e0100 
    str r1, [r0] 

    /* *(scu1+ 8) = 0x00004801 */
    ldr r1, =0x00004801 
    ldr r0, =0x1e6e0120 
    str r1, [r0] 

    /*Delay about 10ms */
    ldr r2, =0x00010000
delay2:
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay2
	 	
    /* *(ddr_setting+ 0) = 0x01;*/
    mov r1, #0x01
    ldr r0, =0x1e6e0000
    str r1, [r0]

    /* *(ddr_setting+ 1) = 0x89;*/
    mov r0, #0x89
    ldr r1, =0x1e6e0004
    str r0,[r1]

    /*  *(ddr_setting+ 2) = 0x12221343; */
    ldr r0, =0x12221343
    ldr r1, =0x1e6e0008
    str r0, [r1]

    /* *(ddr_setting+ 3) = 0xa060009; */
    ldr r0, =0xa060009
    ldr r1, =0x1e6e000c
    str r0, [r1]
	
    /* *(ddr_setting+ 4) = 0x11112222; */
    ldr r0, =0x11112222
    ldr r1, =0x1e6e0010
    str r0,[r1]

    /* *(ddr_setting+ 8) = 0x00140350; */
    ldr r0, =0x00100350
    ldr r1, =0x1e6e0020
    str r0,[r1]

    /* *(ddr_setting+ 9) = 0x000000001; */
    ldr r0, =0x00000001
    ldr r1, =0x1e6e0024
    str r0,[r1]

    /* *(ddr_setting+ 10) = 0x15042df0; */
    ldr r0, =0x15042df0
    ldr r1, =0x1e6e0028
    str r0,[r1]

    /* *(ddr_setting+ 7) = 0x00; */
    mov r0, #0x00
    ldr r1, =0x1e6e001c
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x03;*/
    mov r0, #0x03
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /*  *(ddr_setting+ 6) = 0x131; */
    ldr r0, =0x131
    ldr r1, =0x1e6e0018
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /* *(ddr_setting+ 6) = 0x31; */
    mov r0, #0x31
    ldr r1, =0x1e6e0018
    str r0,[r1]                                                                                

    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]
                                                                                                                                               
    /* *(ddr_setting+ 9) = 0x03; */
    mov r0, #0x03
    ldr r1, =0x1e6e0024
    str r0, [r1]

    /* *(ddr_setting+ 10) = 0x15042df1;*/
    ldr r0, =0x15042df1
    ldr r1, =0x1e6e0028
    str r0, [r1]

    /* *(ddr_setting+ 12) = 0xfffffe01;*/
    ldr r0, =0xfffffe01
    ldr r1, =0x1e6e0030
    str r0, [r1]

    mov pc, lr      /* back to my caller */

dram_set_3:
    /* *(scu1+ 0) = 0xa8 */
    mov r1, #0xa8  
    ldr r0, = 0x1e6e0100 
    str r1, [r0] 

    /* *(scu1+ 8) = 0x00004a21 */
    ldr r1, =0x00004a21 
    ldr r0, =0x1e6e0120 
    str r1, [r0] 

    /*Delay about 10ms */
    ldr r2, =0x00010000
delay3:
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay3
	 	
    /* *(ddr_setting+ 0) = 0x01;*/
    mov r1, #0x01
    ldr r0, =0x1e6e0000
    str r1, [r0]

    /* *(ddr_setting+ 1) = 0x89;*/
    mov r0, #0x89
    ldr r1, =0x1e6e0004
    str r0,[r1]

    /*  *(ddr_setting+ 2) = 0x22331353; */
    ldr r0, =0x22331353
    ldr r1, =0x1e6e0008
    str r0, [r1]

    /* *(ddr_setting+ 3) = 0x0d07000b; */
    ldr r0, =0x0d07000b
    ldr r1, =0x1e6e000c
    str r0, [r1]
	
    /* *(ddr_setting+ 4) = 0x11113333; */
    ldr r0, =0x11113333
    ldr r1, =0x1e6e0010
    str r0,[r1]

    /* *(ddr_setting+ 8) = 0x00110350; */
    ldr r0, =0x00110350
    ldr r1, =0x1e6e0020
    str r0,[r1]

    /* *(ddr_setting+ 9) = 0x000000001; */
    ldr r0, =0x00000001
    ldr r1, =0x1e6e0024
    str r0,[r1]

    /* *(ddr_setting+ 10) = 0x1e0828f0; */
    ldr r0, =0x1e0828f0
    ldr r1, =0x1e6e0028
    str r0,[r1]

    /* *(ddr_setting+ 7) = 0x00; */
    mov r0, #0x00
    ldr r1, =0x1e6e001c
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x03;*/
    mov r0, #0x03
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /*  *(ddr_setting+ 6) = 0x131; */
    ldr r0, =0x131
    ldr r1, =0x1e6e0018
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /* *(ddr_setting+ 6) = 0x31; */
    mov r0, #0x31
    ldr r1, =0x1e6e0018
    str r0,[r1]                                                                                

    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]                                                                                
                                                               
    /* *(ddr_setting+ 9) = 0x03; */
    mov r0, #0x03
    ldr r1, =0x1e6e0024
    str r0, [r1]

    /* *(ddr_setting+ 10) = 0x1e0828f1;*/
    ldr r0, =0x1e0828f1
    ldr r1, =0x1e6e0028
    str r0, [r1]

    /* *(ddr_setting+ 12) = 0xfffffe01;*/
    ldr r0, =0xfffffe01
    ldr r1, =0x1e6e0030
    str r0, [r1]

    mov pc, lr      /* back to my caller */

dram_set_4:
    /* *(scu1+ 0) = 0xa8 */
    mov r1, #0xa8  
    ldr r0, = 0x1e6e0100 
    str r1, [r0] 

    /* *(scu1+ 8) = 0x00004b41 */
    ldr r1, =0x00004b41 
    ldr r0, =0x1e6e0120 
    str r1, [r0] 

    /*Delay about 10ms */
    ldr r2, =0x00010000 
delay4:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay4
	 	
    /* *(ddr_setting+ 0) = 0x01;*/
    mov r1, #0x01
    ldr r0, =0x1e6e0000
    str r1, [r0]

    /* *(ddr_setting+ 1) = 0x89;*/
    mov r0, #0x89
    ldr r1, =0x1e6e0004
    str r0,[r1]

    /*  *(ddr_setting+ 2) = 0x32441353; */
    ldr r0, =0x32441353
    ldr r1, =0x1e6e0008
    str r0, [r1]

    /* *(ddr_setting+ 3) = 0x0e08000d; */
    ldr r0, =0x0e08000d
    ldr r1, =0x1e6e000c
    str r0, [r1]	

    /* *(ddr_setting+ 4) = 0x11113333; */
    ldr r0, =0x11113333
    ldr r1, =0x1e6e0010
    str r0,[r1]

    /* *(ddr_setting+ 8) = 0x00110350; */
    ldr r0, =0x00110350
    ldr r1, =0x1e6e0020
    str r0,[r1]

    /* *(ddr_setting+ 9) = 0x000000001; */
    ldr r0, =0x00000001
    ldr r1, =0x1e6e0024
    str r0,[r1]

    /* *(ddr_setting+ 10) = 0x1d082ef0; */
    ldr r0, =0x01d082f0
    ldr r1, =0x1e6e0028
    str r0,[r1]

    /* *(ddr_setting+ 7) = 0x00; */
    mov r0, #0x00
    ldr r1, =0x1e6e001c
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x03;*/
    mov r0, #0x03
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /*  *(ddr_setting+ 6) = 0x131; */
    ldr r0, =0x131
    ldr r1, =0x1e6e0018
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]
    
    /* *(ddr_setting+ 6) = 0x31; */
    mov r0, #0x31
    ldr r1, =0x1e6e0018
    str r0,[r1]
                                                                                
    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]                                                                                
                                                               
    /* *(ddr_setting+ 9) = 0x03; */
    mov r0, #0x03
    ldr r1, =0x1e6e0024
    str r0, [r1]

    /* *(ddr_setting+ 10) = 0x1d082ef1;*/
    ldr r0, =0x1d082ef1
    ldr r1, =0x1e6e0028
    str r0, [r1]

    /* *(ddr_setting+ 12) = 0xfffffe01;*/
    ldr r0, =0xfffffe01
    ldr r1, =0x1e6e0030
    str r0, [r1]

    mov pc, lr      /* back to my caller */

dram_set_5:
    /* *(scu1+ 0) = 0xa8 */
    mov r1, #0xa8
    ldr r0, = 0x1e6e0100
    str r1, [r0]
                                                                                                    
    /* *(scu1+ 8) = 0x00004c41 */
    ldr r1, =0x00004c41
    ldr r0, =0x1e6e0120
    str r1, [r0]
                                                                                                    
    /*Delay about 10ms */
    ldr r2, =0x00010000
delay5:
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay5

    /* *(ddr_setting+ 0) = 0x01;*/
    mov r1, #0x01
    ldr r0, =0x1e6e0000
    str r1, [r0]

    /* *(ddr_setting+ 1) = 0x89;*/
    mov r0, #0x89
    ldr r1, =0x1e6e0004
    str r0,[r1]

    /*  *(ddr_setting+ 2) = 0x32441353; */
    ldr r0, =0x32441353
    ldr r1, =0x1e6e0008
    str r0, [r1]

    /* *(ddr_setting+ 3) = 0x0f08000d; */
    ldr r0, =0x0f08000d
    ldr r1, =0x1e6e000c
    str r0, [r1]
	
    /* *(ddr_setting+ 4) = 0x11113333; */
    ldr r0, =0x11113333
    ldr r1, =0x1e6e0010
    str r0,[r1]

    /* *(ddr_setting+ 8) = 0x00100350; */
    ldr r0, =0x00100350
    ldr r1, =0x1e6e0020
    str r0,[r1]

    /* *(ddr_setting+ 9) = 0x000000001; */
    ldr r0, =0x00000001
    ldr r1, =0x1e6e0024
    str r0,[r1]

    /* *(ddr_setting+ 10) = 0x1c0834f0; */
    ldr r0, =0x1c0834f0
    ldr r1, =0x1e6e0028
    str r0,[r1]

    /* *(ddr_setting+ 7) = 0x00; */
    mov r0, #0x00
    ldr r1, =0x1e6e001c
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x03;*/
    mov r0, #0x03
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /*  *(ddr_setting+ 6) = 0x131; */
    ldr r0, =0x131
    ldr r1, =0x1e6e0018
    str r0,[r1]

    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]

    /* *(ddr_setting+ 6) = 0x31; */
    mov r0, #0x31
    ldr r1, =0x1e6e0018
    str r0,[r1]
                                                                                
    /* *(ddr_setting+ 5) = 0x01; */
    mov r0, #0x01
    ldr r1, =0x1e6e0014
    str r0,[r1]                                                                                
                                                               
    /* *(ddr_setting+ 9) = 0x03; */
    mov r0, #0x03
    ldr r1, =0x1e6e0024

    str r0, [r1]

    /* *(ddr_setting+ 10) = 0x1c0834f1;*/
    ldr r0, =0x1c0834f1
    ldr r1, =0x1e6e0028
    str r0, [r1]

    /* *(ddr_setting+ 12) = 0xfffffe01;*/
    ldr r0, =0xfffffe01
    ldr r1, =0x1e6e0030
    str r0, [r1]

    mov pc, lr      /* back to my caller */
