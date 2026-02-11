#ifndef _BCMCAPRI_NAND_H
#define _BCMCAPRI_NAND_H

// #define BCMCAPRI_NAND_DEBUG

#ifdef BCMCAPRI_NAND_DEBUG
#include <exports.h>
#define dbgprint(fmt, ...) \
	printf(fmt, ##__VA_ARGS__)
#else
#define dbgprint(fmt, ...)
#endif

#endif				/* _BCMCAPRI_NAND_H */
