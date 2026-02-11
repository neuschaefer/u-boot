#ifndef _BCM2708_DMA_H
#define _BCM2708_DMA_H

#include <linux/types.h>

/* Align a pointer/integer by rounding up/down */
#define ALIGN_DOWN(p, n)   ((uint32_t)(p) - ( (uint32_t)(p) % (uint32_t)(n) ))
#define ALIGN_UP(p, n)     ALIGN_DOWN((uint32_t)(p) + (uint32_t)(n) - 1, (n))

/* The following structure should be 32byte align */
#define DMA_BYTE_ALIGN	32
#define ALIAS_DIRECT(x)               ((void*)(((unsigned)(x)&~0xc0000000)|0xc0000000)) // uncached

typedef struct bcm2708_dma_cb {
  uint32_t ti;
  void* source_ad;
  void* dest_ad;
  uint32_t txfr_len;
  uint32_t stride;
  struct bcm2708_dma_cb* nextconbk;
  void* res1;
  void* res2;
} bcm2708_dma_cb_t;

// DREQ peripheral number
#define SMI_PERIPH_DREQ      0x00000004

// dma reg address offset
#define DMA0_CS              0x00
#define DMA0_CONBLOCK_AD     0x04
#define DMA0_TI              0x08
#define DMA0_SOURCE_AD       0x0c
#define DMA0_DEST_AD         0x10
#define DMA0_TXFR_LEN        0x14
#define DMA0_STRIDE          0x18

#define DMA0_ENABLE          0x0ff0


// bit definitions

#define DMA_EN0                0

#define DMA_CS_RESET          31
#define DMA_CS_ABORT          30
#define DMA_CS_DISDEBUG       29
#define DMA_CS_WAIT_FOR_OUTSTANDING_WRITES 28
#define DMA_CS_PANIC_PRIORITY 20
#define DMA_CS_PRIORITY       16
#define DMA_CS_ERROR           8
#define DMA_CS_WAITING_FOR_OUTSTANDING_WRITES 6
#define DMA_CS_DREQ_STOPS_DMA  5
#define DMA_CS_PAUSED          4
#define DMA_CS_DREQ            3
#define DMA_CS_INT             2
#define DMA_CS_END             1
#define DMA_CS_ACTIVE          0

#define DMA_TI_NO_WIDE_BURSTS  26
#define DMA_TI_WAITS           21
#define DMA_TI_PERMAP          16
#define DMA_TI_BURST_LENGTH    12
#define DMA_TI_SRC_IGNORE      11
#define DMA_TI_SRC_DREQ        10
#define DMA_TI_SRC_WIDTH        9
#define DMA_TI_SRC_INC          8
#define DMA_TI_DEST_IGNORE      7
#define DMA_TI_DEST_DREQ        6
#define DMA_TI_DEST_WIDTH       5
#define DMA_TI_DEST_INC         4
#define DMA_TI_WAIT_RESP        3
#define DMA_TI_TDMODE           1
#define DMA_TI_INTEN            0

#endif
