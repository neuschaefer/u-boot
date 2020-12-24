/*
 * OMAP1 CPU identification code
 *
 * Copyright (C) 2004 Nokia Corporation
 * Written by Tony Lindgren <tony@atomide.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <arm926ejs.h>

#if defined(CONFIG_DISPLAY_CPUINFO) 


#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

int print_cpuinfo (void)
{

	return 0;
}

#endif /* #if defined(CONFIG_DISPLAY_CPUINFO) && defined(CONFIG_OMAP) */
