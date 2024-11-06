/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/lancom_nwapp2.h
 * Copyright (C) 2024 J. Neuschäfer
 */

#ifndef __LANCOM_NWAPP2_H
#define __LANCOM_NWAPP2_H

#define CFG_SYS_INIT_RAM_ADDR	0x01000000  /* at the 16 MiB mark */
#define CFG_SYS_INIT_RAM_SIZE	0x00100000  /* size:   1 MiB      */

#define CFG_SYS_FLASH_BASE	0xe0000000
#define CFG_SYS_FLASH_SIZE	0x00000010  /* ??? */

#endif	/* __LANCOM_NWAPP2_H */
