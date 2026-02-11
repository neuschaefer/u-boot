#ifndef _BCM2708_NAND_H
#define _BCM2708_NAND_H

#define BCM2708_NAND_DEBUG

#ifdef BCM2708_NAND_DEBUG
#include <exports.h>
#define bcm2708_nand_print(fmt, ...) \
	printf(fmt, ##__VA_ARGS__)
#else
#define bcm2708_nand_print(fmt, ...)
#endif

#endif /* _BCM2708_NAND_H */
