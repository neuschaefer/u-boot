/******************************************************************************
 *
 * Copyright (c) 2003 Windond Electronics Corp.
 * All rights reserved.
 *
 * $Workfile: cdefs.h $
 *
 * $Author$
 ******************************************************************************/
/*
 * $History: cdefs.h $
 * 
 * *****************  Version 1  *****************
 * User: Yachen       Date: 06/01/04   Time: 2:28p
 * Created in $/W90P710/Module Test Programs/FIRMWARE_710/WBLv1_1/Inc
 * Module test bootloader, removed decompress function in order to save
 * memory space for LCD control
 * 
 * *****************  Version 2  *****************
 * User: Wschang0     Date: 03/08/28   Time: 5:38p
 * Updated in $/W90P710/FIRMWARE/WBLv1_1/Inc
 * Add VSS header
 */
#ifndef CDEFS_H
#define CDEFS_H
//------------------------------------------------------------------------------
#define OK    0
#define ERROR 1

#ifndef NULL
#define NULL	0
#endif

//------------------------------------------------------------------------------
//typedef int INT;
typedef int SIGNED;
typedef unsigned int UNSIGNED;
typedef unsigned int UINT;
typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UCHAR;
typedef int INT32;
typedef char CHAR;
typedef int INT;
typedef void VOID;
typedef unsigned long ULONG;
typedef unsigned char BOOL;



/****************************************************************************************************
*                                                               
* I/O routines  
*
****************************************************************************************************/
#define VPlong(x)   			(*(volatile unsigned int *)(x))
#define VPshort(x) 			(*(volatile unsigned short *)(x))
#define VPchar(x)  			(*(volatile unsigned char *)(x))

#define inpw(port)			VPlong(port)
#define outpw(port,x)		VPlong(port)=(x)
#define inph(port)			VPshort(port)
#define outph(port,x)		VPshort(port)=(x)
#define inpb(port)			VPchar(port)
#define outpb(port,x)		VPchar(port)=(x)

//------------------------------------------------------------------------------
#endif
