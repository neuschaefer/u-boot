#ifndef __BCM11351_RAY_H
#define __BCM11351_RAY_H

#include "bcm11140_core.h"

/*
 * If you need to override something, then you 
 * can #undef it here and #define it to the custom setting. 
 */
#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS EXTRA_CORE_SETTINGS EXTRA_NET_SETTINGS
#define CONFIG_CMD_A9FREQ

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
    { 0x0, 0x1, "1" }, { 0x1, 0x1, "2" }, { 0x2, 0x1, "3" }, { 0x3, 0x1, "a" }, { 0x4, 0x1, "(" }, \
    { 0x0, 0x2, "4" }, { 0x1, 0x2, "5" }, { 0x2, 0x2, "6" }, { 0x3, 0x2, "b" }, { 0x4, 0x2, ">" }, \
    { 0x0, 0x3, "7" }, { 0x1, 0x3, "8" }, { 0x2, 0x3, "9" }, { 0x3, 0x3, "c" }, { 0x4, 0x3, "<" }, \
    { 0x0, 0x4, "*" }, { 0x1, 0x4, "0" }, { 0x2, 0x4, "#" }, { 0x3, 0x4, "d" }, { 0x4, 0x4, "S" }, \

#undef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING " - bcm11351_ray"

#endif /* __BCM11351_RAY_H */
