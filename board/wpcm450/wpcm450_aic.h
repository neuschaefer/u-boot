#ifndef _WPCM450_AIC_H_
#define _WPCM450_AIC_H_

#include "asm/types.h"

typedef void (*int_func_t)(u32 param);

s32 wpcm450_aic_initialize (void);
s32 wpcm450_register_int(s32 int_num, int_func_t func, u32 param);
void wpcm450_enable_int( u32 int_num);
void wpcm450_disable_int(u32 int_num);

#endif /* _WPCM450_AIC_H_ */
