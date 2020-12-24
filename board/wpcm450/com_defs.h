/*------------------------------------------------------------------------------
 * Copyright (c) 2004-2005 by Winbond Electronics Corporation
 * All rights reserved.
 *<<<---------------------------------------------------------------------------
 * File Contents: 
 *     com_defs.h - Common definitions to be used with all C files
 *
 * Project:
 *     Driver Set for WPC876xL Notebook Embedded Controller Peripherals
 *------------------------------------------------------------------------->>>*/

#ifndef _COM_DEFS_H
#define _COM_DEFS_H

#include "cdefs.h"
/*------------------------------------------------------------------------------
 * Includes
 *----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 * Defines
 *----------------------------------------------------------------------------*/

#define    TRUE            1
#define    FALSE           0

#ifndef    NULL
#define    NULL            0
#endif     // NULL


#define    BIT(X)          (1 << (X))



/*------------------------------------------------------------------------------
 * BIT operation macros
 *----------------------------------------------------------------------------*/

#define SET_BIT(reg, bit)       ((reg)|=(0x1<<(bit)))
#define READ_BIT(reg, bit)      ((reg)&(0x1<<(bit)))
#define CLEAR_BIT(reg, bit)     ((reg)&=(~(0x1<<(bit))))
#define IS_BIT_SET(reg, bit)    (((reg)&(0x1<<(bit)))!=0)

/*------------------------------------------------------------------------------
 * FIELD operation macros
 *----------------------------------------------------------------------------*/

// Get contents of "field" from "reg"
#define GET_FIELD(reg, field)   \
    GET_FIELD_SP(field##_S,field##_P,reg)
    
// Set contents of "field" in "reg" to value"
#define SET_FIELD(reg, field, value)    \
    SET_FIELD_SP(field##_S,field##_P,reg, value)
    
// Auxiliary macro: Get contents of field using size and position
#define GET_FIELD_SP(size, position, reg)    ((reg>>position)&((1<<size)-1))

// Auxiliary macro: Set contents of field using fields size and position
#define SET_FIELD_SP(size, position, reg ,value)    \
    (reg = (reg &(~((((UINT32)1<<size)-1)<<position)))|((UINT32)value<<position))

/*------------------------------------------------------------------------------
 * MASK operation macros
 *----------------------------------------------------------------------------*/

#define SET_MASK(reg, bit_mask)       ((reg)|=(bit_mask))
#define READ_MASK(reg, bit_mask)      ((reg)&(bit_mask))
#define CLEAR_MASK(reg, bit_mask)     ((reg)&=(~(bit_mask)))
#define IS_MASK_SET(reg, bit_mask)    (((reg)&(bit_mask))!=0)

#define GET_MSB(x)  ((x&0xFFFF)>>8)
#define GET_LSB(x)  ((x&0x00FF))

/*------------------------------------------------------------------------------
 * Bits Masks
 *----------------------------------------------------------------------------*/ 
#define BITS_7_0        0xFF
#define BITS_15_8       0xFF00
#define BITS_23_16      0xFF0000

#endif /* _COM_DEFS_H */


