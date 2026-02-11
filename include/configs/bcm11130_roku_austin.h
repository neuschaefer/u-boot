#ifndef __BCM11130_ROKU_AUSTIN_H
#define __BCM11130_ROKU_AUSTIN_H

/* We're basically an 11130 RAY JFFS2 with a few changes. */
#include "bcm11130_ray_jffs2.h"

#undef VC4_OWNED_PINS 
#define VC4_OWNED_PINS \
  {  8, 0x0, 0x1, "CAM1_PWDN"},

#undef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING " - bcm11130_roku_austin"

#undef CONFIG_MHZ_A9_DEFAULT
#define CONFIG_MHZ_A9_DEFAULT	900 /* Best running frequency for all */
#define CONFIG_MHZ_A9_DEFAULT_A2	1001 /* Frequency for A2 silicon */

#undef CONFIG_KONA_KEYPAD

#endif // __BCM11130_ROKU_AUSTIN_H
