/*
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <asm/io.h>

#define TIMER_VALUE (*((volatile unsigned long *)ARM_T_VALUE))
#define TIMER_LOAD_VAL 0xFFFFFFFFUL

static unsigned long timestamp;
static unsigned long lastdec;

void reset_timer_masked(void)
{
    lastdec = TIMER_VALUE;
    timestamp = 0;
}

unsigned long get_timer_masked(void)
{
    unsigned long now = TIMER_VALUE;

    if (lastdec >= now)
        timestamp += lastdec - now;
    else
        timestamp += lastdec + TIMER_LOAD_VAL - now;

    lastdec = now;
    return timestamp;
}

int timer_init(void)
{
    /*
     * Initialise to a known state (all timers off)
     */
    unsigned long ctrl = 0;
    writel(0, ARM_T_CONTROL);

    ctrl = TIMER_CTRL_32BIT | TIMER_CTRL_DBGHALT;
    writel(ctrl, ARM_T_CONTROL);

    ctrl |= TIMER_CTRL_ENABLE;
    writel(ctrl, ARM_T_CONTROL);

    reset_timer_masked();
}

void reset_timer(void)
{
    reset_timer_masked();
}

unsigned long get_timer(unsigned long base)
{
    return get_timer_masked() - base;
}

void set_timer(unsigned long t)
{
	timestamp = t;
}

void __udelay(unsigned long usec)
{
    unsigned long tmo, tmp;

    if (usec >= 1000) {
        /*
         * if "big" number, spread normalization
         * to seconds
         * 1. start to normalize for usec to ticks per sec
         * 2. find number of "ticks" to wait to achieve target
         * 3. finish normalize.
         */
        tmo = usec / 1000;
        tmo *= (CONFIG_SYS_HZ);
        tmo /= 1000;
    } else {
        /* else small number, don't kill it prior to HZ multiply */
        tmo = usec * CONFIG_SYS_HZ;
        tmo /= (1000 * 1000);
    }

    /* get current timestamp */
    tmp = get_timer(0);

    /* if setting this forward will roll time stamp */
    /* reset "advancing" timestamp to 0, set lastdec value */
    /* else, set advancing stamp wake up time */
    if ((tmo + tmp + 1) < tmp)
        reset_timer_masked();
    else
        tmo += tmp;

    /* loop till event */
    while (get_timer_masked() < tmo)
        ;   /* NOP */
}

unsigned long long get_ticks(void)
{
	return get_timer(0);
}

unsigned long get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
