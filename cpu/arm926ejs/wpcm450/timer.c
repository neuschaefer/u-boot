/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 timer driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


#include <common.h>
#include <arm926ejs.h>

#define TIMER_LOAD_VAL 0x00ffffff

/* macro to read the 32 bit timer */
#define READ_TIMER ((*(volatile ulong *)(CFG_TIMERBASE + 16)) & TIMER_LOAD_VAL)

#define SUSPEND_TIMER (*(volatile ulong *)(CFG_TIMERBASE + 0) = 0x080000EF)
#define RESUME_TIMER (*(volatile ulong *)(CFG_TIMERBASE + 0) = 0x480000EF)

static ulong timestamp;
static ulong lastdec;


/* nothing really to do with interrupts, just starts up a counter. */
int timer_init (void)
{
    /* reset timer 0 */
    *(volatile ulong *)(CFG_TIMERBASE + 0) = 0x04000000;
    
    /* set timer initial count, 10us * 0xFFFFFF = 167.77s expired */
    *(volatile ulong *)(CFG_TIMERBASE + 8) = TIMER_LOAD_VAL;
    
    /* enable timer 0 as a count down timer, 01001000 ... EF */
    *(volatile ulong *)(CFG_TIMERBASE + 0) = 0x480000EF; /* 1 tick = 10 us */
    
    /* init the timestamp and lastdec value */
    reset_timer_masked();
    
    return 0;
}


/*
 * timer without interrupts
 */

void reset_timer (void)
{
    reset_timer_masked ();
}


ulong get_timer (ulong base)
{
    return get_timer_masked () - base;
}


void set_timer (ulong t)
{
    timestamp = t;
}


/* delay x useconds AND perserve advance timstamp value */
void udelay (unsigned long usec)
{
    ulong tmo, tmp, i;
    
    if(usec >= 1000){       /* if "big" number, spread normalization to seconds */
        tmo = usec / 1000;  /* start to normalize for usec to ticks per sec */
        tmo *= CFG_HZ;      /* find number of "ticks" to wait to achieve target */
        tmo /= 1000;        /* finish normalize. */
    }else{                  /* else small number, don't kill it prior to HZ multiply */
        tmo = usec * CFG_HZ;
        tmo /= (1000*1000);
    }
    
    tmp = get_timer (0);        /* get current timestamp */
    if( (tmo + tmp + 1) < tmp ) /* if setting this fordward will roll time stamp */
        reset_timer_masked ();  /* reset "advancing" timestamp to 0, set lastdec value */
    else
        tmo += tmp;             /* else, set advancing stamp wake up time */
    
    while (get_timer_masked() < tmo)/* loop till event */
    {
        /* NOP */
        for (i = 0; i < 500; i++);
    }
}


void reset_timer_masked (void)
{
    /* reset time */
    SUSPEND_TIMER;
    lastdec = READ_TIMER;   /* capure current decrementer value time */
    RESUME_TIMER;
    timestamp = 0;          /* start "advancing" time stamp from 0 */
}


ulong get_timer_masked (void)
{
    ulong now;              /* current tick value */
    
    SUSPEND_TIMER;
    now = READ_TIMER;
    RESUME_TIMER;
    
    if (lastdec >= now) {       /* normal mode (non roll) */
        /* normal mode */
        timestamp += lastdec - now; /* move stamp fordward with absoulte diff ticks */
    } else {            /* we have overflow of the count down timer */
        /* nts = ts + ld + (TLV - now)
         * ts=old stamp, ld=time that passed before passing through -1
         * (TLV-now) amount of time after passing though -1
         * nts = new "advancing time stamp"...it could also roll and cause problems.
         */
        timestamp += lastdec + TIMER_LOAD_VAL - now;
    }
    lastdec = now;
    
    return timestamp;
}


/* waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
{
    ulong tmo;
    ulong endtime;
    signed long diff;
    
    if (usec >= 1000) {     /* if "big" number, spread normalization to seconds */
        tmo = usec / 1000;  /* start to normalize for usec to ticks per sec */
        tmo *= CFG_HZ;      /* find number of "ticks" to wait to achieve target */
        tmo /= 1000;        /* finish normalize. */
    } else {            /* else small number, don't kill it prior to HZ multiply */
        tmo = usec * CFG_HZ;
        tmo /= (1000*1000);
    }
    
    endtime = get_timer_masked () + tmo;
    
    do {
        ulong now = get_timer_masked ();
        diff = endtime - now;
    } while (diff >= 0);
}


/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
    return get_timer(0);
}


/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
    ulong tbclk;
    
    tbclk = CFG_HZ;
    return tbclk;
}

