/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
#include <common.h>
#include <asm/io.h>
#include <i2c.h>

#define BATTERY_THRESHOLD_VOLTAGE  3800 //  Millivolts.

#define VMBAT_INVALID     0x04
#define VMBAT_VALID_BITS  0x03 
#define VMBAT_SHIFT_MSB   8 

#define STABLE_VOLTAGE_REACHED_CNT     3
#define NUM_PERCENTAGES                4 

#define CHARGER_INSERTED       1
#define CHARGER_REMOVED        2
#define CHARGER_NO_CHANGE      0 

struct voltage_led_display
{
    unsigned int voltage_percentage ;
	unsigned char reg_dcl ;
	unsigned char reg_dch ;
	unsigned char lwl_0 ;
	unsigned char lwl_1 ;
	unsigned char lwh_0 ;
	unsigned char lwh_1 ;
} ; 

static struct voltage_led_display pmu_led_display_pattern[NUM_PERCENTAGES] = 
{
    { 0,  0x0,  0xFF , 0x00, 0x00, 0xFF, 0x07 } , // Charger not connected. Solid light.
    { 90, 0x00, 0xFF , 0x00, 0x00, 0xff, 0x00 } , // 0 - 90 
    { 95, 0x80, 0x80 , 0x80, 0x00, 0x80, 0x00 } , // 90 - 95 
    { 100, 0x01, 0x80 , 0xFF, 0x00, 0x33, 0x00 } , // above 95
} ; 

static int pmu_get_battery_voltage(unsigned int *millivolts)
{
    // Read address 0x82, and 0x83
	// Calculate voltage.
    unsigned char buffer[16] = {0} ;
	unsigned int regval = 0 ;
	unsigned int millivolt_mul = 1000 ;

    i2c_read (0x08, 0x82, 1, &buffer[0],2) ;

	if ( !(buffer[0] & VMBAT_INVALID ) )
	{
		buffer[0] = buffer[0] & VMBAT_VALID_BITS ;
		 
        regval = ( buffer[0] << VMBAT_SHIFT_MSB ) | ( buffer[1] ) ;

    	debug("regval is 0x%x \n", regval ) ;

    	regval = ( regval * millivolt_mul * 48 ) / ( 1024  * 10 ) ;

    	*millivolts = regval ;
    }
	else
	{
		printf("Invalid battery data from PMU \n") ;

        return 1 ; // Invalid data from PMU.
	}

	return 0 ;
}

static int pmu_is_charger_connected(void)
{

	unsigned char rw_buf[16] = {0} ;

    i2c_read (0x08, 0x31, 1, &rw_buf[0],1) ;

	if ( rw_buf[0] & 1 )  // Charger is inserted.
	{
        return CHARGER_INSERTED ; 
	}
	else if ( rw_buf[0] & 0x02 ) // Charger is removed.
	{
        return CHARGER_REMOVED ;
	}
	return CHARGER_NO_CHANGE ; // Read of register returns 0.
}

static int pmu_initialize_charging(void)
{
	unsigned char rw_buf[16] = {0} ;

    // First read all intr registers.
	// from 0x30 to 0x3d
    i2c_read (0x08, 0x30, 1, &rw_buf[0],14) ;

    // Disablei intr in register 0x41.
    i2c_write (0x08, 0x41, 1, &rw_buf[0],1) ;

    // Set battery initialization registers.
    rw_buf[0] = 0x08 ;
    i2c_write (0x08, 0x55, 1, &rw_buf[0],1) ;
    rw_buf[0] = 0x07 ;
    i2c_write (0x08, 0x57, 1, &rw_buf[0],1) ;
    rw_buf[0] = 0x01 ;
    i2c_write (0x08, 0x58, 1, &rw_buf[0],1) ;
    
	return 0 ;
}

static int pmu_stop_charging(void)
{
	unsigned char w_buf[16] = {0} ;

    // disable charging now for wall only.
    w_buf[0] = 0x04 ;
    i2c_write (0x08, 0x52, 1, &w_buf[0],1) ;

	// Stop blinking when charging stopped.
    w_buf[0] = 0x46 ;
    i2c_write (0x0C, 0x00, 1, &w_buf[0],1) ;

	return 0 ;
}

static int pmu_start_charging(void)
{
	unsigned char w_buf[16] = {0} ;

    // Enable charging now for wall only.
    w_buf[0] = 0x05 ;
    i2c_write (0x08, 0x52, 1, &w_buf[0],1) ;
    
	// Start blinking when start charging.
    w_buf[0] = 0x44 ;
    i2c_write (0x0C, 0x00, 1, &w_buf[0],1) ;

	return 0 ;
}

static void pmu_led_display(unsigned int present_voltage, unsigned int threshold )
{
	unsigned char w_buf[4] = {0} ;
	unsigned int percentage = 0 ;
	unsigned int index = 0 ;
    percentage = ( present_voltage * 100 ) / threshold ; 

	if ( percentage == 0 ) 
	{
        w_buf[0] = 0x46 ;
        i2c_write (0x0C, 0x00, 1, &w_buf[0],1) ;
	}
	else
	{
		index = 1 ;
        while ( index < NUM_PERCENTAGES ) 
		{
            if ( percentage <= pmu_led_display_pattern[index].voltage_percentage ) 
			{
                w_buf[0] = 0x44 ;
                i2c_write (0x0C, 0x00, 1, &w_buf[0],1) ;

                i2c_write (0x0C, 0x02, 1, &pmu_led_display_pattern[index].reg_dcl,1) ;
                i2c_write (0x0C, 0x03, 1, &pmu_led_display_pattern[index].reg_dch,1) ;
                i2c_write (0x0C, 0x04, 1, &pmu_led_display_pattern[index].lwl_0,1) ;
                i2c_write (0x0C, 0x05, 1, &pmu_led_display_pattern[index].lwl_1,1) ;
                i2c_write (0x0C, 0x06, 1, &pmu_led_display_pattern[index].lwh_0,1) ;
                i2c_write (0x0C, 0x07, 1, &pmu_led_display_pattern[index].lwh_1,1) ;
				break ; 
			}
			index++ ;
		}
	}
}

void wait_second(unsigned int num)
{
    unsigned int i = 0 ;

	for ( i = 0 ; i < (num * 1000) ; i++) 
	{
        udelay(1000) ;
	}
}

int battery_status_check(unsigned int threshold_microvolts)
{
    unsigned int battery_millivolts = 0 ;
	unsigned int present_charger_connected_state = CHARGER_NO_CHANGE ;
	unsigned int old_charger_connected_state = CHARGER_NO_CHANGE ;
	unsigned int threshold_voltage = 0 ;
	unsigned int stable_voltage_reached = 0 ;
	unsigned int retval = 0 ;

    threshold_voltage = threshold_microvolts / 1000 ;

	while ( ( battery_millivolts <= threshold_voltage ) || ( stable_voltage_reached != STABLE_VOLTAGE_REACHED_CNT ) ) 
	{
        retval = pmu_get_battery_voltage(&battery_millivolts) ;

        present_charger_connected_state = pmu_is_charger_connected() ;

		// Check to see if charger is connected.
		if ( ( present_charger_connected_state == CHARGER_INSERTED )  && 
             ( ( old_charger_connected_state == CHARGER_REMOVED ) || ( old_charger_connected_state == CHARGER_NO_CHANGE) ) )
		{
            // Initialize charging.
            pmu_initialize_charging() ;

			// Start charging.
            pmu_start_charging() ;

			// Set state correctly
			old_charger_connected_state = present_charger_connected_state ;
		}
		else if ( ( present_charger_connected_state == CHARGER_REMOVED ) && 
                  ( ( old_charger_connected_state == CHARGER_INSERTED ) || ( old_charger_connected_state == CHARGER_NO_CHANGE ) ) )
		{
            // Stop charging.
            pmu_stop_charging() ;
			
			printf("Connect wall charger , present voltage %d, threshold set to %d \n", battery_millivolts, threshold_voltage ) ;

            // Set state correctly
            old_charger_connected_state = present_charger_connected_state ;
		}
		else if ( present_charger_connected_state == CHARGER_NO_CHANGE )
        {
            if ( old_charger_connected_state == CHARGER_INSERTED ) 
			{
                pmu_led_display(battery_millivolts,threshold_voltage ) ;
    			printf("Present voltage %d, threshold set to %d \n", battery_millivolts, threshold_voltage ) ;
			}
			else if ( ( old_charger_connected_state == CHARGER_REMOVED ) || ( old_charger_connected_state == CHARGER_NO_CHANGE ) )
			{
				if ( battery_millivolts <= threshold_voltage) 
				{
        			printf("Connect wall charger , present voltage %d, threshold set to %d \n", battery_millivolts, threshold_voltage ) ;
				}
				else 
				{
        			printf("Present voltage %d, threshold set to %d \n", battery_millivolts, threshold_voltage ) ;
				}
			}
        }
						
        if ( retval == 0 )
        {
            if ( battery_millivolts >= threshold_voltage )
            {
                stable_voltage_reached++ ;
            }
        }
        else
        {
            printf(" Invalid voltage reading \n") ;
        }

		wait_second(1) ;
	}

    printf("Correct voltage needed to boot linux attained, %d mv \n", battery_millivolts ) ;

	return 0 ;
}
