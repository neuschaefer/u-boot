#ifndef __BCM11130_RAY_H
#define __BCM11130_RAY_H

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
  { 91, 0x0, 0x0,"LCD_SPI0_CS"},	\
  { 92, 0x0, 0x0,"LCD_SPI0_CLK"},	\
  { 94, 0x0, 0x0,"LCD_SPI0_MOSI"},	\
  {  8, 0x0, 0x1, "CAM1_PWDN"},		\
  {  9, 0x0, 0x0, "CAM1_RST"},		\
  { 10, 0x0, 0x0, "CAM2_RST"},		\
  { 11, 0x0, 0x1, "CAM2_PWDN"},	        \
  {176, 0x0, 0x0, "CAM2_REG_ON"},	\



#define KONA_KEYPAD_KEYMAP   \
    { 0x0, 0x0, "VOL_UP" },   { 0x0, 0x1, "HOME" }, { 0x0, 0x2, "UP" }, \
    { 0x1, 0x0, "VOL_DOWN" }, { 0x1, 0x1, "BACK" }, { 0x1, 0x2, "DOWN" }

#undef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING " - bcm11130_ray"

#endif /* __BCM11130_RAY_H */
