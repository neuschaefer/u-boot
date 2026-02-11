#ifndef __BCM11140_TABLET_H
#define __BCM11140_TABLET_H

#include "bcm11140_core.h"

/*
 * If you need to override something, then you 
 * can #undef it here and #define it to the custom setting. 
 */
#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS EXTRA_CORE_SETTINGS EXTRA_NET_SETTINGS

#define VC4_OWNED_PINS \
  {004, 0x0, 0x0,"DPI_POWER_PIN"},	\
  {100, 0x0, 0x0,"DPI_BL_PWR_PIN"},	\
  {  7, 0x0, 0x0,"DPI_BL_EN_PIN"},	\
  {  5, 0x0, 0x0,"LCD_RST_B_PIN"},	\
  {  8, 0x0, 0x1, "CAM1_PWDN"},		\
  {  9, 0x0, 0x0, "CAM1_RST"},		\
  { 10, 0x0, 0x0, "CAM2_RST"},		\
  { 11, 0x0, 0x1, "CAM2_PWDN"},	   \
  {104, 0x0, 0x0, "CAM_FLASH_EN1"},  \
  {108, 0x0, 0x0, "CAM_FLASH_TRIG"}, \
  {176, 0x0, 0x0, "CAM_REG_ON"},	   \

/* Only the tablet boards enable the ethernet phy using a GPIO */
#define CONFIG_ETH_PHY_POWER_GPIO	99

#define KONA_KEYPAD_KEYMAP   \
    { 0x0, 0x0, "VOL_DOWN" }, { 0x0, 0x1, "VOL_UP" }, { 0x0, 0x2, "HOME" }, \
    { 0x1, 0x0, "SEARCH" },   { 0x1, 0x1, "BACK" },   { 0x1, 0x2, "MENU" }

#undef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING " - bcm11140_tablet"

#endif /* __BCM11140_TABLET_H */
