
#ifndef _MC_REGS_WPCS410_H
#define _MC_REGS_WPCS410_H

/************************************************************************/
/* WPCS410 registers definition                                         */
/************************************************************************/


#define MC_BA             0xB0001000             /* Memory control registers base address*/
#define MC_CFG0           VPlong(MC_BA+0x0)      /* Memory control configuration register 0 (defaul value = 0x20000000)*/
#define MC_CFG1           VPlong(MC_BA+0x4)      /* Memory control configuration register 1 (defaul value = 0x000E2033)*/
#define MC_CFG2           VPlong(MC_BA+0x8)      /* Memory control configuration register 2 (defaul value = 0x40000033)*/
#define MC_CFG3           VPlong(MC_BA+0xC)      /* Memory control configuration register 3 (defaul value = 0x00000000)*/
#define MC_CFG4           VPlong(MC_BA+0x10)     /* Memory control configuration register 4 (defaul value = 0x00000000)*/
#define MC_CFG5           VPlong(MC_BA+0x14)     /* Memory control configuration register 5 (defaul value = 0x00000000)*/
#define MC_CFG6           VPlong(MC_BA+0x18)     /* Memory control configuration register 5 (defaul value = 0x00000000)*/
#define MC_CFG7           VPlong(MC_BA+0x1C)     /* Memory control configuration register 7 (defaul value = 0x00000000)*/
#define MC_P1_ARBT        VPlong(MC_BA+0x24)     /* Port 1 arbitration timer value (defaul value = 0x00000000)*/
#define MC_P1_CNT         VPlong(MC_BA+0x20)     /* Port 1 arbitration control register (defaul value = 0x00000000)*/
#define MC_P2_ARBT        VPlong(MC_BA+0x2C)     /* Port 2 arbitration timer value (defaul value = 0x00000000)*/
#define MC_P2_CNT         VPlong(MC_BA+0x28)     /* Port 2 arbitration control register (defaul value = 0x00000000)*/
#define MC_P3_ARBT        VPlong(MC_BA+0x34)     /* Port 3 arbitration timer value (defaul value = 0x00000000)*/
#define MC_P3_CNT         VPlong(MC_BA+0x30)     /* Port 3 arbitration control register (defaul value = 0x00000000)*/
#define MC_P4_ARBT        VPlong(MC_BA+0x3C)     /* Port 4 arbitration timer value (defaul value = 0x00000000)*/
#define MC_P4_CNT         VPlong(MC_BA+0x38)     /* Port 4 arbitration control register (defaul value = 0x00000000)*/
#define MC_P5_ARBT        VPlong(MC_BA+0x44)     /* Port 4 arbitration timer value (defaul value = 0x00000000)*/
#define MC_P5_CNT         VPlong(MC_BA+0x40)     /* Port 4 arbitration control register (defaul value = 0x00000000)*/
#define MC_P6_ARBT        VPlong(MC_BA+0x4C)     /* Port 4 arbitration timer value (defaul value = 0x00000000)*/
#define MC_P6_CNT         VPlong(MC_BA+0x48)     /* Port 4 arbitration control register (defaul value = 0x00000000)*/
#define MC_P1_INCRS       VPlong(MC_BA+0x50)     /* Port 1 INCR size control (defaul value = 0x00000000)*/
#define MC_P2_INCRS       VPlong(MC_BA+0x54)     /* Port 2 INCR size control (defaul value = 0x00000000)*/
#define MC_P3_INCRS       VPlong(MC_BA+0x58)     /* Port 3 INCR size control (defaul value = 0x00000000)*/
#define MC_P4_INCRS       VPlong(MC_BA+0x5C)     /* Port 4 INCR size control (defaul value = 0x00000000) */
#define MC_DLL_0          VPlong(MC_BA+0x0060)		 /* DLL and ODT Configuration Register 0 */
#define MC_DLL_1          VPlong(MC_BA+0x0064)		 /* PDLL and ODT Configuration Register 1 */


/* Memory Control Register 0 fields */



#define MC_A2A_TIME_P        28
#define MC_A2A_TIME_S         4
#define MC_MIN_A2P_TIME_P    23
#define MC_MIN_A2P_TIME_S     5
#define MC_P2A_CMD_TIME_P    19
#define MC_P2A_CMD_TIME_S     4
#define MC_P2A_CMD_DELAY_P   13 
#define MC_P2A_CMD_DELAY_S    6
#define MC_REFREASH_INT_P     0
#define MC_REFREASH_INT_S    13


/* Memory Control Register 1 fields */



#define MC_W2R_DELAY_P        28
#define MC_W2R_DELAY_S         4
#define MC_R2P_DELAY_TERM_P   24
#define MC_R2P_DELAY_TERM_S    4
#define MC_SDRAM_SIZE_P       18
#define MC_SDRAM_SIZE_S        3
#define MC_SDRAM_WIDTH_P      16
#define MC_SDRAM_WIDTH_S       2
#define MC_EXT_BANK_P         14
#define MC_EXT_BANK_S          2
#define MC_WRT_RECOV_TIME_P    8
#define MC_WRT_RECOV_TIME_S    4
#define MC_SET_ACTIVE_P        4
#define MC_SET_ACTIVE_S        4
#define MC_RAS2CAS_DELAY_P     0
#define MC_RAS2CAS_DELAY_S     4


/* Memory Control Register 2 fields */
#define MC_REGE_BIT            31
#define MC_DDR_MODE_BIT        30
#define MC_POWER_DOWN_EXIT_BIT 12
#define MC_WRITE_RECOVERY_P     9
#define MC_WRITE_RECOVERY_S     3
#define MC_DLLRESET_BIT         8
#define MC_TESTMODE_BIT         7
#define MC_CAS_LATENCY_P        4
#define MC_CAS_LATENCY_S        3
#define MC_WRAP_TYPE_BIT        3
#define MC_BURST_LENGTH_P       0
#define MC_BURST_LENGTH_S       3

/* Arbitration Timer Registers fields */
#define MC_TIMER_P              0
#define MC_TIMER_S             24

/* Arbitration Control Registers fields */
#define MC_SCHEME_BIT           2
#define MC_PRI_BIT              1
#define MC_AUTO_BIT             0

/* INCR Size Control Registers */
#define MC_INCR_SIZE_P          0
#define MC_INCR_SIZE_S          3

/* MC_DLL_0 & MC_DLL_ Registers */
#define MC_IODT_S               2
#define MC_IODT_P               8
#define MC_DLL_DELAY_S          8
#define MC_DLL_DELAY_P          0


typedef enum {
  MC_ONE_EXT_BANK  = 0x0,
  MC_TWO_EXT_BANK  = 0x1,
  MC_FOUR_EXT_BANK = 0x2
}McExtBank_t;

typedef enum {
  MC_SDRAM_4BIT_WIDE  = 0x0,
  MC_SDRAM_8BIT_WIDE  = 0x1,
  MC_SDRAM_16BIT_WIDE = 0x2,
  MC_SDRAM_32BIT_WIDE = 0x3
}McSdramWidth_t;


/* RAM size options */

typedef enum {
  MC_SDRAM_64MBIT  = 0x1,
  MC_SDRAM_128MBIT = 0x2,
  MC_SDRAM_256MBIT = 0x3,
  MC_SDRAM_512MBIT = 0x4,
  MC_SDRAM_1GBIT   = 0x5,
  MC_SDRAM_2GBIT   = 0x6
}McSdramSize_t;

#endif /* _MC_REGS_WPCS410_H */
