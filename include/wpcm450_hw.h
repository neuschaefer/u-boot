#ifndef _WPCM450_HW_H_
#define _WPCM450_HW_H_

#include <config.h>

/* U-Boot does not use MMU. So no mapping */
#define IO_ADDRESS(x)	(x)
#define MEM_ADDRESS(x)  (x)

#include <wpcm450/hwmap.h>
#include <wpcm450/hwreg.h>
#include <wpcm450/hwdef.h>
#include <wpcm450/serreg.h>
#include <wpcm450/macreg.h>
#endif
