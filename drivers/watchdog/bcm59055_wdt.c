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
#include <watchdog.h>
#include <common.h>
#include <asm/io.h>
#include <i2c.h>

#define BCM59055_WDT_I2C_SLAVE		0x08
#define BCM59055_HOSTCTRL1		0x01
#define BCM59055_HOSTCTRL2		0x02
#define BCM59055_SYS_WDT_EN		0x01
#define BCM59055_SYS_WDT_CLR		0x02
#define BCM59055_SYS_WDT_TIME_MASK	0x7F

int bcm59055_wdt_gettimeout(void)
{
	unsigned char buf;
	i2c_read(BCM59055_WDT_I2C_SLAVE, BCM59055_HOSTCTRL2, 1, &buf, 1);
	return (buf & BCM59055_SYS_WDT_TIME_MASK);
}

void bcm59055_wdt_settimeout(unsigned int timeout)
{
	unsigned char buf;
	i2c_read(BCM59055_WDT_I2C_SLAVE, BCM59055_HOSTCTRL2, 1, &buf, 1);
	buf &= ~BCM59055_SYS_WDT_TIME_MASK;
	buf |= (timeout & BCM59055_SYS_WDT_TIME_MASK);
	i2c_write(BCM59055_WDT_I2C_SLAVE, BCM59055_HOSTCTRL2, 1, &buf, 1);
}

void bcm59055_wdt_reset(void)
{
	unsigned char buf;
	i2c_read(BCM59055_WDT_I2C_SLAVE, BCM59055_HOSTCTRL1, 1, &buf, 1);
	buf |= BCM59055_SYS_WDT_CLR;
	i2c_write(BCM59055_WDT_I2C_SLAVE, BCM59055_HOSTCTRL1, 1, &buf, 1);
}

void bcm59055_wdt_enable(int enable)
{
	unsigned char buf;
	i2c_read(BCM59055_WDT_I2C_SLAVE, BCM59055_HOSTCTRL1, 1, &buf, 1);
	if (enable)
		buf |= BCM59055_SYS_WDT_EN;
	else
		buf &= ~BCM59055_SYS_WDT_EN;
	i2c_write(BCM59055_WDT_I2C_SLAVE, BCM59055_HOSTCTRL1, 1, &buf, 1);
}

void bcm59055_wdt_init(void)
{
	unsigned char buf;

	i2c_read(BCM59055_WDT_I2C_SLAVE, BCM59055_HOSTCTRL1, 1, &buf, 1);
	
	if(buf & BCM59055_SYS_WDT_EN)
	{
		bcm59055_wdt_enable(1);
		bcm59055_wdt_settimeout(127);
	}
}
