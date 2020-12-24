/******************************************************************************
 *
 * Copyright (c) 2003 Windond Electronics Corp.
 * All rights reserved.
 *
 * $Workfile: ram_init.h $
 *
 * $Author$
 ******************************************************************************/

#ifndef _RAM_INIT_H_
#define _RAM_INIT_H_

typedef enum {
  MEM_SKIP_INIT_MODE = 0x0, /* Skip all initializations and jump to SPI flash directly with no check. */
  MEM_128MB_MODE     = 0x1, /* 128 MB. */
  MEM_64MB_MODE      = 0x2, /* 64 MB.  */
  MEM_32MB_MODE      = 0x3  /* 32 MB.  */
}PowerOnMemSizeTypes_t;

/************************************************************************/
/* FUNCTIONS DEFINITIONS                                                */
/************************************************************************/
void WBL_RamInit(void);

#endif /* _RAM_INIT_H_ */



