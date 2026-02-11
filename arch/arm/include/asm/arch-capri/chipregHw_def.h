/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/


#ifndef CHIPREG_DEF_H
#define CHIPREG_DEF_H

/* ---- Include Files ----------------------------------------------------- */

#include <asm/arch/chal_types.h>
#include <asm/arch/reg.h>
#include <asm/arch/chipregHw_reg.h>
//#define _Bool uint8_t

/* ---- Public Constants and Types ---------------------------------------- */

typedef enum {
  chipregHw_STRAP_ASIC_MODE_BIGISLAND    = 0x0,
  chipregHw_STRAP_ASIC_MODE_HANA         = 0x1,
  chipregHw_STRAP_ASIC_MODE_LITTLEISLAND = 0x2
 } chipregHw_STRAP_ASIC_MODE_e;

typedef enum {
  chipregHw_STRAP_XTAL_TYPE_13MHZ    = 0x0,
  chipregHw_STRAP_XTAL_TYPE_26MHZ    = 0x1,
  chipregHw_STRAP_XTAL_TYPE_19p2MHZ  = 0x2,
  chipregHw_STRAP_XTAL_TYPE_38p4MHZ  = 0x3
 } chipregHw_STRAP_XTAL_TYPE_e;

typedef enum {
  chipregHw_STRAP_BOOT_OPTIONS_NOR_FLASH    = 0x0,
  chipregHw_STRAP_BOOT_OPTIONS_SD_MMC       = 0x1,
  chipregHw_STRAP_BOOT_OPTIONS_NAND_FLASH   = 0x2,
  chipregHw_STRAP_BOOT_OPTIONS_SERIAL_FLASH = 0x3,
  chipregHw_STRAP_BOOT_OPTIONS_UART         = 0x4,
  chipregHw_STRAP_BOOT_OPTIONS_USB_OTG      = 0x5
 } chipregHw_STRAP_BOOT_OPTIONS_e;

/* Pin function - refer to pinmux spreadsheet */
typedef enum {
  chipregHw_PIN_FUNCTION_ALT01 = chipregHw_REG_PIN_MUX_ALT01,
  chipregHw_PIN_FUNCTION_ALT02 = chipregHw_REG_PIN_MUX_ALT02,
  chipregHw_PIN_FUNCTION_ALT03 = chipregHw_REG_PIN_MUX_ALT03,
  chipregHw_PIN_FUNCTION_ALT04 = chipregHw_REG_PIN_MUX_ALT04,
  chipregHw_PIN_FUNCTION_ALT05 = chipregHw_REG_PIN_MUX_ALT05,
  chipregHw_PIN_FUNCTION_ALT06 = chipregHw_REG_PIN_MUX_ALT06,
} chipregHw_PIN_FUNCTION_e;

/* PIN Output slew rate */
typedef enum {
  chipregHw_PIN_SLEW_RATE_HIGH          =    chipregHw_REG_SLEW_RATE_HIGH,
  chipregHw_PIN_SLEW_RATE_LOW           =    chipregHw_REG_SLEW_RATE_LOW
} chipregHw_PIN_SLEW_RATE_e;

/* PIN Current drive strength */
typedef enum {
  chipregHw_PIN_CURRENT_STRENGTH_02mA   =    chipregHw_REG_CURRENT_STRENGTH_02mA,
  chipregHw_PIN_CURRENT_STRENGTH_04mA   =    chipregHw_REG_CURRENT_STRENGTH_04mA,
  chipregHw_PIN_CURRENT_STRENGTH_06mA   =    chipregHw_REG_CURRENT_STRENGTH_06mA,
  chipregHw_PIN_CURRENT_STRENGTH_08mA   =    chipregHw_REG_CURRENT_STRENGTH_08mA,
  chipregHw_PIN_CURRENT_STRENGTH_10mA   =    chipregHw_REG_CURRENT_STRENGTH_10mA,
  chipregHw_PIN_CURRENT_STRENGTH_12mA   =    chipregHw_REG_CURRENT_STRENGTH_12mA,
  chipregHw_PIN_CURRENT_STRENGTH_14mA   =    chipregHw_REG_CURRENT_STRENGTH_14mA,
  chipregHw_PIN_CURRENT_STRENGTH_16mA   =    chipregHw_REG_CURRENT_STRENGTH_16mA
} chipregHw_PIN_CURRENT_STRENGTH_e;


/* PIN Pull up register settings */
typedef enum {
  chipregHw_PIN_PULL_NONE               =    chipregHw_REG_PULL_NONE,
  chipregHw_PIN_PULL_UP                 =    chipregHw_REG_PULL_UP,
  chipregHw_PIN_PULL_DOWN               =    chipregHw_REG_PULL_DOWN
} chipregHw_PIN_PULL_e;


/* PIN input type settings */
typedef enum {
  chipregHw_PIN_INPUTTYPE_TTL           =    chipregHw_REG_INPUTTYPE_TTL,
  chipregHw_PIN_INPUTTYPE_ST            =    chipregHw_REG_INPUTTYPE_ST
} chipregHw_PIN_INPUTTYPE_e;

/* The following defines are used for HANA only */
/* PIN Pull up register settings */
typedef enum {
  chipregHw_I2C_PIN_PULL_NONE           =    chipregHw_REG_I2C_PULL_NONE,
  chipregHw_I2C_PIN_PULL_UP             =    chipregHw_REG_I2C_PULL_UP
} chipregHw_I2C_PIN_PULL_e;

/* PIN Output source rate */
typedef enum {
  chipregHw_I2C_PIN_SRC_RATE_HIGH       =    chipregHw_REG_I2C_SRC_RATE_HIGHSPEED,
  chipregHw_I2C_PIN_SRC_RATE_LOW        =    chipregHw_REG_I2C_SRC_RATE_SLOWSPEED
} chipregHw_I2C_PIN_SRC_RATE_e;

typedef enum {
  chipregHw_I2C_PIN_MODE_NONHS          =    chipregHw_REG_I2C_MODE_NONHIGHSPEED,
  chipregHw_I2C_PIN_MODE_HS             =    chipregHw_REG_I2C_MODE_HIGHSPEED
} chipregHw_I2C_PIN_MODE_e;

typedef enum {
  chipregHw_I2C_PIN_INPUT_CTRL_ENABLE   =    chipregHw_REG_I2C_INPUT_CTRL_ENABLE,
  chipregHw_I2C_PIN_INPUT_CTRL_DISABLE  =    chipregHw_REG_I2C_INPUT_CTRL_DISABLE
} chipregHw_I2C_PIN_INPUT_CTRL_e;

/* ---- Public Variable Externs ------------------------------------------ */
/* ---- Public Function Prototypes --------------------------------------- */

/****************************************************************************/
/**
*  @brief   Get Numeric Chip ID
*
*  This function returns Chip ID
*
*  @return  Complete numeric Chip ID
*
*/
/****************************************************************************/
static inline uint32_t chipregHw_getChipId(void);

/****************************************************************************/
/**
*  @brief   Get Island ID
*
*  This function returns Island ID
*
*  @return  Complete Island ID
*
*/
/****************************************************************************/
static inline uint32_t chipregHw_getIslandId(void);

/****************************************************************************/
/**
*  @brief   Get Hardware Straps
*
*  This function returns Hardware Straps
*
*  @return  Hardware Straps
*
*/
/****************************************************************************/
static inline uint32_t chipregHw_getHardwareStraps(void);

/****************************************************************************/
/**
*  @brief   Get Software Straps
*
*  This function returns Software Straps
*
*  @return  Software Straps
*
*/
/****************************************************************************/
static inline uint32_t chipregHw_getSoftwareStraps(void);

/****************************************************************************/
/**
*  @brief   Set Software Straps
*
*  This function sets Software Straps
*
*/
/****************************************************************************/
static inline void chipregHw_setSoftwareStraps
(
   uint32_t strapOptions        /* Strap Settings */
);

/****************************************************************************/
/**  @brief Checks if software strap is enabled
 *
 *   @return 1 : When enable
        0 : When disable
 */
/****************************************************************************/
static inline _Bool chipregHw_isSoftwareStrapsEnable(void);

/****************************************************************************/
/**  @brief Enable software strap
 */
/****************************************************************************/
static inline void chipregHw_softwareStrapsEnable(void);

/****************************************************************************/
/**  @brief Disable software strap
 */
/****************************************************************************/
static inline void chipregHw_softwareStrapsDisable(void);

/****************************************************************************/
/**  @brief Get Crytal Oscillator Bypass strap
 */
/****************************************************************************/
static inline _Bool chipregHw_getStrapXtalOscBypass(void);

/****************************************************************************/
/**  @brief Get ASIC mode strap
 */
/****************************************************************************/
static inline chipregHw_STRAP_ASIC_MODE_e chipregHw_getStrapAsicMode(void);

/****************************************************************************/
/**  @brief Get crystal type strap
 */
/****************************************************************************/
static inline chipregHw_STRAP_XTAL_TYPE_e chipregHw_getStrapXtalType(void);

/****************************************************************************/
/**  @brief Get boot mode strap
 */
/****************************************************************************/
static inline chipregHw_STRAP_BOOT_OPTIONS_e chipregHw_getStrapBootMode(void);

/****************************************************************************/
/**  @brief Get OTP Program and Status strap
 */
/****************************************************************************/
static inline _Bool chipregHw_getStrapOtpProgMode(void);

/****************************************************************************/
/**  @brief Get TMACT strap
 */
/****************************************************************************/
static inline _Bool chipregHw_getStrapTmact(void);

/****************************************************************************/
/**  @brief Get flashboot strap
 */
/****************************************************************************/
static inline _Bool chipregHw_getStrapFlashboot(void);


#if 0
/****************************************************************************/
/**
*  @brief   Get to know the configuration of pin
*
*/
/****************************************************************************/
static inline chipregHw_PIN_FUNCTION_e chipregHw_getPinFunction
(
   int pin                          /* Pin number */
);


/****************************************************************************/
/**
*  @brief   Configure pin function
*
*/
/****************************************************************************/
static inline void chipregHw_setPinFunction
(
   int pin,                           /* Pin number */
   chipregHw_PIN_FUNCTION_e func      /* Configuration function */
);

/****************************************************************************/
/**
*  @brief   Set Pin slew rate
*
*  This function sets the slew of individual pin
*
*/
/****************************************************************************/
static inline void chipregHw_setPinSlewRate
(
   int pin,                             /* Pin of type chipregHw_PIN_XXXXX */
   chipregHw_PIN_SLEW_RATE_e slewRate   /* Pin slew rate */
);


/****************************************************************************/
/**
*  @brief   Set Pin output drive current
*
*  This function sets output drive current of individual pin
*
*  Note: Avoid the use of the word 'current' since linux headers define this
*        to be the current task.
*/
/****************************************************************************/
static inline void chipregHw_setPinOutputCurrent
(
   int pin,                                 /* Pin of type chipregHw_PIN_XXXXX */
   chipregHw_PIN_CURRENT_STRENGTH_e curr    /* Pin current rating */
);


/****************************************************************************/
/**
*  @brief   Set Pin pullup register
*
*  This function sets pullup register of individual  pin
*
*/
/****************************************************************************/
static inline void chipregHw_setPinPullup
(
   int                     pin,       /* Pin of type chipregHw_PIN_XXXXX */
   chipregHw_PIN_PULL_e    pullup     /* Pullup register settings */
);


/****************************************************************************/
/**
*  @brief   Set Pin input type
*
*  This function sets input type of individual Pin
*
*/
/****************************************************************************/
static inline void chipregHw_setPinInputType
(
   int pin,                               /* Pin of type chipregHw_PIN_XXXXX */
   chipregHw_PIN_INPUTTYPE_e inputType    /* Pin input type */
);
#endif

/****************************************************************************/
/**
*  @brief   Set System Timer into 32b or 64b mode
*
*  
*
*
*  @note: mode = 0 -> 32bit mode; mode = 1 -> 64bit mode
*/
/****************************************************************************/
static inline void chipregHw_setSlvTimer64bitmode( int mode );

/****************************************************************************/
/**
*  @brief   Get System Timer mode
*
*  
*
*
*  @note: mode = 0 -> 32bit mode; mode = 1 -> 64bit mode
*/
/****************************************************************************/
static inline uint32_t chipregHw_getSlvTimer64bitmode( void );

/****************************************************************************/
/**
*  @brief   Set Hub Timer into 32b or 64b mode
*
*  
*
*
*  @note: mode = 0 -> 32bit mode; mode = 1 -> 64bit mode
*/
/****************************************************************************/
static inline void chipregHw_setHubTimer64bitmode( int mode );

/****************************************************************************/
/**
*  @brief   Get Hub Timer mode
*
*  
*
*
*  @note: mode = 0 -> 32bit mode; mode = 1 -> 64bit mode
*/
/****************************************************************************/
static inline uint32_t chipregHw_getHubTimer64bitmode( void );

/* ---- Private Constants and Types -------------------------------------- */

#endif /* CHIPREG_DEF_H */
