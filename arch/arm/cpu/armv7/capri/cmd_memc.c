/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/
#include <common.h>
#include <command.h>
#include <image.h>
#include <linux/ctype.h>

#include <asm/io.h>
#include <asm/arch/brcm_rdb_sysmap.h>
#include <asm/arch/brcm_rdb_csr.h>
#include <config.h>

// 312MHz counter for demesh clock

#define NS_TO_QOS_TIMEOUT(T) ( ((((unsigned)(T))*312u+500u)/1000u) << 12 )
#define QOS_TIMEOUT_TO_NS(t) (((unsigned)(t)*1000+156)/312)

#define CAM_TAG_PORT_PRI_AID( port, port_mask, priority, priority_mask, aid, aid_mask )   \
                ( ( ((aid_mask)&0xFF) | (((priority_mask) & 3) << 10) | (((port_mask) & 3) << 13) ) << 16 ) | \
                ( ((aid)&0xFF) | (((priority) & 3) << 10) | (((port) & 3) << 13) )

#define CAM_TAG_PORT_PRI(port, port_mask, priority, priority_mask ) CAM_TAG_PORT_PRI_AID( port, port_mask, priority, priority_mask, 0, 0 /* Don't care */ )

#define CAM_TAG_PORT( port )  CAM_TAG_PORT_PRI( port, 3, 0, 0 /* Don't care */ )
#define CAM_TAG_PRI( pri, pri_mask )  CAM_TAG_PORT_PRI( 0, 0, /* Don't care */ pri, pri_mask )
#define CAM_TAG_NONE    (0x7FFF7FFFu)

#define CAM_ENTRY_PRI_LATENCY( pri, ns )    ( 0x0D000000 | NS_TO_QOS_TIMEOUT(ns) | (!!(pri) << 25) )
#define CAM_ENTRY_LATENCY( ns )   CAM_ENTRY_PRI_LATENCY( 0, ns )

typedef struct unit_info_struct
{
	const char *name;
	uint32_t base;
} UNIT_INFO_T;

static UNIT_INFO_T
units[] =
{
	{ "SYS", MEMC0_OPEN_BASE_ADDR },
	{ "VC4", VC4_EMI_OPEN_BASE_ADDR }
};

static const int
num_units = (sizeof(units)/sizeof(units[0]));

static int do_memc_qos (int argc, char * const argv[]);
static int do_memc_bwc (int argc, char * const argv[]);
static int do_memc_spr (int argc, char * const argv[]);
static int do_memc_apal(int argc, char * const argv[]);
static int do_memc_regdump(int argc, char * const argv[]);
static int do_memc_crc(int argc, char * const argv[]);
static int get_qos_tag(char *str, unsigned long *out);
static int get_qos_val(char *str, unsigned long *out);
static int get_unit_mask(const char *str);
static int prefix_match(const char *substr, const char *str);
static int write_reg_hex(int unitmask, unsigned long regoffset,
	unsigned long reserved, const char *hex);
static int get_hex_value(const char *str, int digits, unsigned int *out);
static int read_key_value(char **p, char **key, char **value);

extern void
write_qos_entry(uint32_t base, int slot, unsigned long tag, unsigned long val);

static int
do_memc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd;
	int ret = -1;

	if (argc >= 2)
	{
		cmd = argv[1];

		if (strcmp(cmd, "qos") == 0)
			ret = do_memc_qos(argc - 1, argv + 1);
		else if (strcmp(cmd, "bwc") == 0)
			ret = do_memc_bwc(argc - 1, argv + 1);
		else if (strcmp(cmd, "spr") == 0)
			ret = do_memc_spr(argc - 1, argv + 1);
		else if (strcmp(cmd, "apal") == 0)
			ret = do_memc_apal(argc - 1, argv + 1);
		else if (strcmp(cmd, "regdump") == 0)
			ret = do_memc_regdump(argc - 1, argv + 1);
		else if (strcmp(cmd, "crc") == 0)
			ret = do_memc_crc(argc - 1, argv + 1);
	}

	if (ret < 0)
		cmd_usage(cmdtp);

	return ret;
}

U_BOOT_CMD(
		memc, 6, 0, do_memc,
		"display or modify MEMC settings",
		"apal [unit [value]]         - AXI port access limit\n"
		"memc bwc  [unit [value]]         - bandwidth control\n"
		"memc spr  [unit [value]]         - SPR register flags\n"
		"memc qos  [unit [slot [tag val]] - QOS settings\n"
		"  where:\n"
		"    unit is sys, vc4, or * for both\n"
		"    value is a 32-bit hexadecimal value\n"
		"    slot is 0-15\n"
		"    tag is a 32-bit hexadecimal value, or a comma-separated sequence of:\n"
		"      none       - never matches (i.e. a dummy)\n"
		"      all        - matches everything (i.e. a catch-all)\n"
		"      port:p[P]  - accesses matching port p(0-3) [with mask P]\n"
		"      pri:r[R]   - accesses matching priority r(0-3) [with mask R]\n"
		"      aid:aa[AA] - accesses matching AXI id aa(00-ff) [with mask AA]\n"
		"    val is a 32-bit hexadecimal value, or one of:\n"
		"      lat:n   - a latency value of n(decimal) nanoseconds\n"
		"      ticks:t - a latency value of t(hex) ticks\n"
      "memc regdump  [unit]             - dumps all registry (address, value)\n"
      "memc crc  [unit]                 - runs crc all registry values\n"
		);

/*******************************************************************/
/* QOS - EMI quality-of-service settings  */
/*******************************************************************/

void check_cam_tag_entry( int entry, unsigned tag, unsigned val )
{
	if( (entry == 0) || (tag != CAM_TAG_NONE) )
	{
		if( entry == 0 )
		{
			// Entry 0 is special - the tag is ignored, and the entry is always latency
			if( (val & CSR_CAM_ENTRY_0_CAM_LATENCY_0_MASK) == 0 )
			{
				printf( "        CAM_ENTRY_0 is hardwired as latency type, but bandwidth type is requested\n" );
			}
		}
		else // Entries 1..15
		{
			if( (tag & 0x13000000) && (tag != 0x7fff0000) )
			{
				printf( "        Tag tests non-standard bits - may not match\n" );
			}

			if( ((0 ^ ((tag>>13) & 0x3)) & ((tag>>29) & 0x3)) == 0 )  // Port matches VC
			{
				unsigned pritag = (tag >> 10) & 3;
				unsigned prien = (tag >> 26) & 3;

				if( (prien == 3)       // Matching {cam_sel,urgent}
						&& (pritag == 1) // Matches urgent with cam_sel==0
				  )
				{
					printf( "        Tag matches VC4 port, tests for (urgent && !cam_sel) - may not match\n" );
				}
			}
			else  // Does not match VC
			{
				if( tag & 0x0C000000 )
				{
					printf( "        Tag does not (only) match VC4, but tests VC4-only pri bits - may not match\n" );
				}

			}
		}


		if( (val & (CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_MASK| CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_MASK))
				!= (CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_MASK| CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_MASK) )
		{
			printf( "        Entry does not have both READ_ and WRITE_ENABLE set: QoS may be disabled.\n" );
		}

		if( val & CSR_CAM_ENTRY_0_CAM_PRIORITY_0_MASK )
		{
			printf( "        Entry has obsolete PRIORITY set. This setting has no effect.\n" );
		}
	}
}

static int
do_memc_qos(int argc, char * const argv[])
{
	int unitmask = (1 << num_units) - 1;
	int min_slot = 0;
	int max_slot = 15;
	int argn = 1;
	int i;

	if (argn < argc)
	{
		unitmask = get_unit_mask(argv[argn++]);
		if (!unitmask)
			return -1;

		if (argn < argc)
		{
			unsigned long slot;
			if ((strict_strtoul(argv[argn++], 10, &slot) != 0) ||
				(slot > max_slot))
				return -1;
			min_slot = max_slot = slot;
			if ((argn + 2) == argc)
			{
				unsigned long tag, val;
				int success = get_qos_tag(argv[argn++], &tag);
				if (success)
					success = get_qos_val(argv[argn++], &val);

				if (success)
				{
					for (i = 0; i < num_units; i++)
					{
						if (unitmask & (1 << i))
						{
							write_qos_entry(units[i].base, slot, tag, val);
						}
					}

					check_cam_tag_entry( slot, tag, val );

					return 0;
				}
				else
				{
					return -1;
				}
			}
			else if (argn != argc)
			{
				return -1;
			}
		}
	}

	for (i = 0; i < num_units; i++)
	{
		if (unitmask & (1 << i))
		{
			UNIT_INFO_T *unit = &units[i];
			int j;

			printf("%s QOS (entry: tag value):\n", unit->name);
			for (j = min_slot; j <= max_slot; j++)
			{
				unsigned int tag = (unsigned int)readl(unit->base + CSR_CAM_ENABLE_0_OFFSET + j * 4);
				unsigned int val = (unsigned int)readl(unit->base + CSR_CAM_ENTRY_0_OFFSET + j * 4);
				printf("    %2d: 0x%08x 0x%08x - "
					"aid/msk=%02x/%02x pri/msk=%d/%d port/msk=%d/%d -> "
					"thold=%d urg_ctr=%d(%dns) lat=%d pri=%d we=%d re=%d\n",
					j, tag, val,
					(tag>>0) & 0xff, (tag>>16) & 0xff,
					(tag>>10) & 0x3, (tag>>26) & 0x3,
					(tag>>13) & 0x3, (tag>>29) & 0x3,
					(val & CSR_CAM_ENTRY_0_CAM_THOLD_0_MASK) >> CSR_CAM_ENTRY_0_CAM_THOLD_0_SHIFT,
					(val & CSR_CAM_ENTRY_0_CAM_URG_CTR_0_MASK) >> CSR_CAM_ENTRY_0_CAM_URG_CTR_0_SHIFT,
					QOS_TIMEOUT_TO_NS((val & CSR_CAM_ENTRY_0_CAM_URG_CTR_0_MASK) >> CSR_CAM_ENTRY_0_CAM_URG_CTR_0_SHIFT),
					(val & CSR_CAM_ENTRY_0_CAM_LATENCY_0_MASK) >> CSR_CAM_ENTRY_0_CAM_LATENCY_0_SHIFT,
					(val & CSR_CAM_ENTRY_0_CAM_PRIORITY_0_MASK) >> CSR_CAM_ENTRY_0_CAM_PRIORITY_0_SHIFT,
					(val & CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_MASK) >> CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_SHIFT,
					(val & CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_MASK) >> CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_SHIFT
					);

				check_cam_tag_entry( j, tag, val );
			}
		}
	}

	return 0;
}

/*******************************************************************/
/* BWC - EMI bandwidth control  */
/*******************************************************************/

static int
do_memc_bwc(int argc, char * const argv[])
{
	int unitmask = (1 << num_units) - 1;
	int argn = 1;
	int i;

	if (argn < argc)
	{
		unitmask = get_unit_mask(argv[argn++]);
		if (!unitmask)
			return -1;

		if (argn < argc)
		{
			if (!write_reg_hex(unitmask, CSR_SEQ_RDWR_BANDWIDTH_CONTROL_OFFSET,
				CSR_SEQ_RDWR_BANDWIDTH_CONTROL_RESERVED_MASK,
				argv[argn++]))
			{
				return -1;
			}
			return 0;
		}
	}

	for (i = 0; i < num_units; i++)
	{
		if (unitmask & (1 << i))
		{
			UNIT_INFO_T *unit = &units[i];
			unsigned long regval;
			regval = readl(unit->base + CSR_SEQ_RDWR_BANDWIDTH_CONTROL_OFFSET);
			printf("%s SEQ_RDWR_BANDWIDTH_CONTROL: 0x%03lx\n", unit->name, regval);
			printf("        q_depth            = %ld\n",
				(regval & CSR_SEQ_RDWR_BANDWIDTH_CONTROL_HIT_Q_DEPTH_MASK) >>
				CSR_SEQ_RDWR_BANDWIDTH_CONTROL_HIT_Q_DEPTH_SHIFT);
			printf("        rd_transaction_cnt = %ld\n",
				(regval & CSR_SEQ_RDWR_BANDWIDTH_CONTROL_RD_TRANSACTION_CNT_MASK) >>
				CSR_SEQ_RDWR_BANDWIDTH_CONTROL_RD_TRANSACTION_CNT_SHIFT);
			printf("        wr_transaction_cnt = %ld\n",
				(regval & CSR_SEQ_RDWR_BANDWIDTH_CONTROL_WR_TRANSACTION_CNT_MASK) >>
				CSR_SEQ_RDWR_BANDWIDTH_CONTROL_WR_TRANSACTION_CNT_SHIFT);
		}
	}

	return 0;
}

/*******************************************************************/
/* SPR - Extra configuration settings  */
/*******************************************************************/

static int
do_memc_spr(int argc, char * const argv[])
{
	int unitmask = (1 << num_units) - 1;
	int argn = 1;
	int i;

	if (argn < argc)
	{
		unitmask = get_unit_mask(argv[argn++]);
		if (!unitmask)
			return -1;

		if (argn < argc)
		{
			if (!write_reg_hex(unitmask, CSR_CORE_SPR_RW_OFFSET,
				CSR_CORE_SPR_RW_RESERVED_MASK,
				argv[argn++]))
			{
				return -1;
			}
			return 0;
		}
	}

	for (i = 0; i < num_units; i++)
	{
		if (unitmask & (1 << i))
		{
			UNIT_INFO_T *unit = &units[i];
			unsigned long regval;
			regval = readl(unit->base + CSR_CORE_SPR_RW_OFFSET);
			printf("%s CORE_SPR_RW: 0x%08lx\n", unit->name, regval);
		}
	}

	return 0;
}

/*******************************************************************/
/* APAL - AXI Port Access Limit settings  */
/*******************************************************************/

static int
do_memc_apal(int argc, char * const argv[])
{
	int unitmask = (1 << num_units) - 1;
	int argn = 1;
	int i;

	if (argn < argc)
	{
		unitmask = get_unit_mask(argv[argn++]);
		if (!unitmask)
			return -1;

		if (argn < argc)
		{
			if (!write_reg_hex(unitmask, CSR_AXI_PORT_ACCESS_LIMIT_OFFSET,
				CSR_AXI_PORT_ACCESS_LIMIT_RESERVED_MASK,
				argv[argn++]))
			{
				return -1;
			}
			return 0;
		}
	}

	for (i = 0; i < num_units; i++)
	{
		if (unitmask & (1 << i))
		{
			UNIT_INFO_T *unit = &units[i];
			unsigned long regval;
			regval = readl(unit->base + CSR_AXI_PORT_ACCESS_LIMIT_OFFSET);
			printf("%s AXI_PORT_ACCESS_LIMIT: 0x%08lx\n", unit->name, regval);
			printf("        vc4      = %ld slots\n",
				(regval & CSR_AXI_PORT_ACCESS_LIMIT_PORT0_ACCESS_LIMIT_MASK) >>
				CSR_AXI_PORT_ACCESS_LIMIT_PORT0_ACCESS_LIMIT_SHIFT);
			printf("        arm      = %ld slots\n",
				(regval & CSR_AXI_PORT_ACCESS_LIMIT_PORT1_ACCESS_LIMIT_MASK) >>
				CSR_AXI_PORT_ACCESS_LIMIT_PORT1_ACCESS_LIMIT_SHIFT);
			printf("        periphs  = %ld slots\n",
				(regval & CSR_AXI_PORT_ACCESS_LIMIT_PORT2_ACCESS_LIMIT_MASK) >>
				CSR_AXI_PORT_ACCESS_LIMIT_PORT2_ACCESS_LIMIT_SHIFT);
			printf("        modem    = %ld slots\n",
				(regval & CSR_AXI_PORT_ACCESS_LIMIT_PORT3_ACCESS_LIMIT_MASK) >>
				CSR_AXI_PORT_ACCESS_LIMIT_PORT3_ACCESS_LIMIT_SHIFT);
			printf("        override = %ld slots\n",
				(regval & CSR_AXI_PORT_ACCESS_LIMIT_ACCESS_LIMIT_OVERRIDE_MASK) >>
				CSR_AXI_PORT_ACCESS_LIMIT_ACCESS_LIMIT_OVERRIDE_SHIFT);
		}
	}

	return 0;
}

/*******************************************************************/
/* REGDUMP - Dumps all set registers for each unit, this dump is   */
/*           raw format on purpose such that the information can   */
/*           be used directly for other use, as example dt-blob    */
/*           auto-configuration.                                   */
/*******************************************************************/
static int
do_memc_regdump(int argc, char * const argv[])
{
	int unitmask = (1 << num_units) - 1;
	int min_slot = 0;
	int max_slot = 15;
	int argn = 1;
	int i;

	if (argn < argc)
	{
		unitmask = get_unit_mask(argv[argn++]);
		if (!unitmask)
			return -1;
   }

	for (i = 0; i < num_units; i++)
	{
		if (unitmask & (1 << i))
		{
		   UNIT_INFO_T *unit = &units[i];
		   unsigned long regval;
			int j;

         printf("\n");

         /* APAL register */
         regval = readl(unit->base + CSR_AXI_PORT_ACCESS_LIMIT_OFFSET);
         printf("0x%08lx /* 0x%04x_%04x : %s AXI_PORT_ACCESS_LIMIT --> number of slots - vc4: %ld - arm: %ld - periphs: %ld - modem: %ld - override: %ld */\n",
                regval,
                (unit->base + CSR_AXI_PORT_ACCESS_LIMIT_OFFSET) >> 16,
                (unit->base + CSR_AXI_PORT_ACCESS_LIMIT_OFFSET) & 0xFFFF,
                unit->name,
                (regval & CSR_AXI_PORT_ACCESS_LIMIT_PORT0_ACCESS_LIMIT_MASK) >> CSR_AXI_PORT_ACCESS_LIMIT_PORT0_ACCESS_LIMIT_SHIFT,
                (regval & CSR_AXI_PORT_ACCESS_LIMIT_PORT1_ACCESS_LIMIT_MASK) >> CSR_AXI_PORT_ACCESS_LIMIT_PORT1_ACCESS_LIMIT_SHIFT,
                (regval & CSR_AXI_PORT_ACCESS_LIMIT_PORT2_ACCESS_LIMIT_MASK) >> CSR_AXI_PORT_ACCESS_LIMIT_PORT2_ACCESS_LIMIT_SHIFT,
                (regval & CSR_AXI_PORT_ACCESS_LIMIT_PORT3_ACCESS_LIMIT_MASK) >> CSR_AXI_PORT_ACCESS_LIMIT_PORT3_ACCESS_LIMIT_SHIFT,
                (regval & CSR_AXI_PORT_ACCESS_LIMIT_ACCESS_LIMIT_OVERRIDE_MASK) >> CSR_AXI_PORT_ACCESS_LIMIT_ACCESS_LIMIT_OVERRIDE_SHIFT );

         printf("\n");

         /* SPR register */
         regval = readl(unit->base + CSR_CORE_SPR_RW_OFFSET);
         printf("0x%08lx /* 0x%04x_%04x : %s CORE_SPR_RW */\n",
                regval,
                (unit->base + CSR_CORE_SPR_RW_OFFSET) >> 16,
                (unit->base + CSR_CORE_SPR_RW_OFFSET) & 0xFFFF,
                unit->name );

         printf("\n");

         /* BWC register */
         regval = readl(unit->base + CSR_SEQ_RDWR_BANDWIDTH_CONTROL_OFFSET);
			printf("0x%08lx /* 0x%04x_%04x : %s SEQ_RDWR_BANDWIDTH_CONTROL --> q_depth: %ld - rd_transaction_cnt: %ld - wr_transaction_cnt: %ld */\n",
                regval,
                (unit->base + CSR_SEQ_RDWR_BANDWIDTH_CONTROL_OFFSET) >> 16,
                (unit->base + CSR_SEQ_RDWR_BANDWIDTH_CONTROL_OFFSET) & 0xFFFF,
                unit->name,
                (regval & CSR_SEQ_RDWR_BANDWIDTH_CONTROL_HIT_Q_DEPTH_MASK) >> CSR_SEQ_RDWR_BANDWIDTH_CONTROL_HIT_Q_DEPTH_SHIFT,
                (regval & CSR_SEQ_RDWR_BANDWIDTH_CONTROL_RD_TRANSACTION_CNT_MASK) >> CSR_SEQ_RDWR_BANDWIDTH_CONTROL_RD_TRANSACTION_CNT_SHIFT,
                (regval & CSR_SEQ_RDWR_BANDWIDTH_CONTROL_WR_TRANSACTION_CNT_MASK) >> CSR_SEQ_RDWR_BANDWIDTH_CONTROL_WR_TRANSACTION_CNT_SHIFT );

         printf("\n");

         /* CAM entry */
			for (j = min_slot; j <= max_slot; j++)
			{
            regval = readl(unit->base + CSR_CAM_ENTRY_0_OFFSET + j * 4);
            printf("0x%08lx /* 0x%04x_%04x : %s CAM_ENTRY_%d --> thold: %d - urg_ctr: %d(%dns) - lat: %d - pri: %d - we: %d - re: %d */\n",
                   regval,
                   (unit->base + CSR_CAM_ENTRY_0_OFFSET + j * 4) >> 16,
                   (unit->base + CSR_CAM_ENTRY_0_OFFSET + j * 4) & 0xFFFF,
                   unit->name,
                   j,
                   (regval & CSR_CAM_ENTRY_0_CAM_THOLD_0_MASK) >> CSR_CAM_ENTRY_0_CAM_THOLD_0_SHIFT,
                   (regval & CSR_CAM_ENTRY_0_CAM_URG_CTR_0_MASK) >> CSR_CAM_ENTRY_0_CAM_URG_CTR_0_SHIFT,
                   QOS_TIMEOUT_TO_NS((regval & CSR_CAM_ENTRY_0_CAM_URG_CTR_0_MASK) >> CSR_CAM_ENTRY_0_CAM_URG_CTR_0_SHIFT),
                   (regval & CSR_CAM_ENTRY_0_CAM_LATENCY_0_MASK) >> CSR_CAM_ENTRY_0_CAM_LATENCY_0_SHIFT,
                   (regval & CSR_CAM_ENTRY_0_CAM_PRIORITY_0_MASK) >> CSR_CAM_ENTRY_0_CAM_PRIORITY_0_SHIFT,
                   (regval & CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_MASK) >> CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_SHIFT,
                   (regval & CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_MASK) >> CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_SHIFT );
         }

         printf("\n");

         /* CAM enable */
         for (j = min_slot; j <= max_slot; j++)
			{
            regval = readl(unit->base + CSR_CAM_ENABLE_0_OFFSET + j * 4);
            printf("0x%08lx /* 0x%04x_%04x : %s CAM_ENABLE_%d --> aid/msk: %02x/%02x - pri/msk: %d/%d - port/msk: %d/%d */\n",
                   regval,
                   (unit->base + CSR_CAM_ENABLE_0_OFFSET + j * 4) >> 16,
                   (unit->base + CSR_CAM_ENABLE_0_OFFSET + j * 4) & 0xFFFF,
                   unit->name,
                   j,
                   (regval>>0) & 0xff, (regval>>16) & 0xff,
                   (regval>>10) & 0x3, (regval>>26) & 0x3,
                   (regval>>13) & 0x3, (regval>>29) & 0x3 );
         }

         printf("\n");
      }
   }

   return 0;
}

/*******************************************************************/
/* CRC - Dumps CRC of all registers selected.                      */
/*******************************************************************/
int
do_memc_crc(int argc, char * const argv[])
{
	int unitmask = (1 << num_units) - 1;
	int min_slot = 0;
	int max_slot = 15;
	int argn = 1;
	int i;
   unsigned long crc = 0;
   unsigned long regval;

	if (argn < argc)
	{
		unitmask = get_unit_mask(argv[argn++]);
		if (!unitmask)
			return -1;
   }

   crc = crc32_no_comp( 0, 0, 0 );
	for (i = 0; i < num_units; i++)
	{
		if (unitmask & (1 << i))
		{
		   UNIT_INFO_T *unit = &units[i];
			int j;

         /* APAL register */
         regval = readl( (unsigned long)(unit->base + CSR_AXI_PORT_ACCESS_LIMIT_OFFSET) );
         crc = crc32_no_comp( crc, &regval, sizeof( unsigned long ) );

         /* SPR register */
         regval = readl( (unsigned long)(unit->base + CSR_CORE_SPR_RW_OFFSET) );
         crc = crc32_no_comp( crc, &regval, sizeof( unsigned long ) );

         /* BWC register */
         regval = readl( (unsigned long)(unit->base + CSR_SEQ_RDWR_BANDWIDTH_CONTROL_OFFSET) );
         crc = crc32_no_comp( crc, &regval, sizeof( unsigned long ) );

         /* QOS registers [CAM entry and CAM enable] */
			for (j = min_slot; j <= max_slot; j++)
			{
            regval = readl( (unsigned long)(unit->base + CSR_CAM_ENTRY_0_OFFSET + j * 4) );
            crc = crc32_no_comp( crc, &regval, sizeof( unsigned long ) );

            regval = readl( (unsigned long)(unit->base + CSR_CAM_ENABLE_0_OFFSET + j * 4) );
            crc = crc32_no_comp( crc, &regval, sizeof( unsigned long ) );
         }
      }
   }

   printf("\nMemory Controller (unit: %s) -> CRC: 0x%08lx\n\n", argv[1], crc );
   return 0;
}

/*******************************************************************/
/* Support functions */
/*******************************************************************/

static int
get_qos_tag(char *str, unsigned long *out)
{
	int success = 1;
	unsigned long tag;

	if (strict_strtoul(str, 16, &tag) != 0)
	{
		/* Check for user-friendly key:value,... options */
		char *key = NULL;
		char *value = NULL;
		char *p = str;
		tag = CAM_TAG_PORT_PRI(0,0,0,0);
		while (read_key_value(&p, &key, &value))
		{
			if (key && strcmp(key, "none") == 0)
			{
				tag = CAM_TAG_NONE;
			}
			else if (key && strcmp(key, "all") == 0)
			{
				tag = CAM_TAG_PORT_PRI(0,0,0,0);
			}
			else if (key && strcmp(key, "port") == 0)
			{
				unsigned int port;
				if (value && get_hex_value(value, 1, &port))
				{
					unsigned int port_msk = 3;
					if ((*(value + 1) != '\0') && !get_hex_value(value + 1, 1, &port_msk))
						break;
					tag |= CAM_TAG_PORT_PRI(port, port_msk, 0, 0);
				}
				else
					break;
			}
			else if (key && strcmp(key, "pri") == 0)
			{
				unsigned int pri;
				if (value && get_hex_value(value, 1, &pri))
				{
					unsigned int pri_msk = 3;
					if ((*(value + 1) != '\0') && !get_hex_value(value + 1, 1, &pri_msk))
						break;
					tag |= CAM_TAG_PORT_PRI(0, 0, pri, pri_msk);
				}
				else
					break;
			}
			else if (key && strcmp(key, "aid") == 0)
			{
				unsigned int aid;
				if (value && get_hex_value(value, 2, &aid))
				{
					unsigned int aid_msk = 0xff;
					if ((*(value + 1) != '\0') && !get_hex_value(value + 2, 2, &aid_msk))
						break;
					tag |= CAM_TAG_PORT_PRI_AID(0, 0, 0, 0, aid, aid_msk);
				}
				else
					break;
			}
			else
				break;
		}
		success = (p == NULL);
	}

	if (success)
		*out = tag;

	return success;
}

static int
get_qos_val(char *str, unsigned long *out)
{
	int success = 1;
	unsigned long val;

	if (strict_strtoul(str, 16, &val) != 0)
	{
		/* Check for user-friendly key:value,... options */
		char *key = NULL;
		char *value = NULL;
		char *p = str;
		val = CSR_CAM_ENTRY_0_CAM_WRITE_ENABLE_0_MASK |
			CSR_CAM_ENTRY_0_CAM_READ_ENABLE_0_MASK;

		while (read_key_value(&p, &key, &value))
		{
			if (key && strcmp(key, "lat") == 0)
			{
				long unsigned int ns;
				if (value && strict_strtoul(value, 10, &ns) == 0)
					val |= CSR_CAM_ENTRY_0_CAM_LATENCY_0_MASK | NS_TO_QOS_TIMEOUT(ns);
				else
					break;
			}
			else if (key && strcmp(key, "ticks") == 0)
			{
				long unsigned int ticks;
				if (value && strict_strtoul(value, 16, &ticks) == 0)
				{
					ticks <<= CSR_CAM_ENTRY_1_CAM_URG_CTR_1_SHIFT;
					if ((ticks & CSR_CAM_ENTRY_1_CAM_URG_CTR_1_MASK) != ticks)
						break;
					val |= CSR_CAM_ENTRY_0_CAM_LATENCY_0_MASK | ticks;
				}
				else
					break;
			}
			else if (key && strcmp(key, "pri") == 0)
			{
				if (value)
					break;
				val |= CSR_CAM_ENTRY_0_CAM_PRIORITY_0_MASK;
			}
			else
				break;

			if ( p == str ) { p = NULL; }
		}
	}

	if (success)
		*out = val;

	return success;
}

static int
get_unit_mask(const char *str)
{
	int unitmask = 0;

	if (strcmp(str, "*") == 0)
		unitmask = (1 << num_units) - 1;
	else
	{
		int i;
		for (i = 0; i < num_units; i++)
		{
			if (prefix_match(str, units[i].name))
			{
				unitmask = (1 << i);
				break;
			}
		}
	}

	return unitmask;
}

static int
prefix_match(const char *substr, const char *str)
{
	int i;
	for (i = 0; substr[i]; i++)
	{
		if ((str[i] == 0) || (tolower(substr[i]) != tolower(str[i])))
		{
			return 0;
		}
	}
	return 1;
}

static int
write_reg_hex(int unitmask, unsigned long regoffset, unsigned long reserved, const char *hex)
{
	unsigned long regval;
	int i;

	if (strict_strtoul(hex, 16, &regval) != 0)
		return 0;

	if ((regval & reserved) != 0)
	{
		printf("* %08lx are reserved bits - did you mean %lx?\n",
			reserved, regval & ~reserved);
		return 0;
	}

	for (i = 0; i < num_units; i++)
	{
		if (unitmask & (1 << i))
		{
			writel(regval, units[i].base + regoffset);
		}
	}

	return 1;
}

/* Match exactly 'digits' hex digits */
static int
get_hex_value(const char *str, int digits, unsigned int *out)
{
	unsigned int val = 0;
	while (digits)
	{
		char c = *(str++);
		val <<= 4;
		if ((c >= '0') && (c <= '9'))
			val |= (c - '0');
		else if ((c >= 'a') && (c <= 'f'))
			val |= (c - 'a') + 10;
		else if ((c >= 'A') && (c <= 'F'))
			val |= (c - 'A') + 10;
		else
			break;
		digits--;
	}

	if ( digits == 0 )
	{
		*out = val;
		return 1;
	}
	else
	{
		return 0;
	}
}

static int
read_key_value(char **str, char **key, char **value)
{
	char *p = *str;
	char *sep = NULL;
	char *end = strchr(p, ',');
	int eos = 0;

	if ( *str == '\0' )
		return 0;

	/*printf("read-key-value::str[%s]\n",
	        *str );*/

	eos = (end == NULL);
	if ( end ) { *end = '\0'; }

	*key = p;
	*value = NULL;
	sep = strchr(p, ':');
	if (sep)
	{
		*sep = '\0';
		*value = (sep + 1);
	}

	/*printf("read-key-value::key[%s]::value[%s]::end[%s](%s)\n",
	        *key,
	        *value ? *value : "(null)",
	        eos ? "yes" : "no",
	        eos ? "(null)" : end + 1 );*/

	if ( !eos )
	{
		*str = (end + 1);
	}
	else
	{
		*str = '\0';
	}
	return 1;
}

