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
#include "kona_bsc.h"
#include <asm/arch/brcm_rdb_i2c_mm_hs.h>

#define DEFAULT_I2C_BUS_SPEED      BSC_BUS_SPEED_50K

#define I2C_TIMEOUT	100

/* upper 5 bits of the master code */
#define MASTERCODE                 0x08
#define MASTERCODE_MASK            0x07

#define MAX_I2C_CONTROLLERS        3
#define MAX_SLAVES_PER_CONTROLLER  32 

/*
 * BSC (I2C) private data structure
 */
struct bsc_i2c_dev
{
   unsigned int virt_base ;
   
   /* I2C bus speed */
   enum bsc_bus_speed speed;
};

#define I2C_M_TEN		0x0010
#define I2C_M_RD		0x0001
#define I2C_M_NOSTART		0x4000
#define I2C_M_REV_DIR_ADDR	0x2000
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_NO_RD_ACK		0x0800
#define I2C_M_RECV_LEN		0x0400

struct i2c_msg {
  unsigned short addr;
  unsigned short flags;
  unsigned short len;
  unsigned char * buf;
};  

/*
 * Bus speed lookup table
 */
static const unsigned int gBusSpeedTable[BSC_SPD_MAXIMUM] =
{
   BSC_SPD_32K,
   BSC_SPD_50K,
   BSC_SPD_230K,
   BSC_SPD_380K,
   BSC_SPD_400K,
   BSC_SPD_430K,
   BSC_SPD_HS,
   BSC_SPD_100K_FPGA,
   BSC_SPD_400K_FPGA,
   BSC_SPD_HS_FPGA,
};

static unsigned int bus_initialized[MAX_I2C_CONTROLLERS] = {0, 0, 0};
static unsigned int current_bus = CONFIG_SYS_I2C_INITIAL_BUS;

struct bsc_i2c_dev g_i2c_devs[MAX_I2C_CONTROLLERS] = 
{
   { 0x3e016000, BSC_SPD_50K } ,		
   { 0x3e017000, BSC_SPD_50K } ,		
   { 0x3500d000, BSC_SPD_50K } ,		
} ;

/*
 * We should not need to do this in software, but this is how the hardware was
 * designed and that leaves us with no choice but SUCK it
 *
 * When the CPU writes to the control, data, or CRC registers the CMDBUSY bit
 * will be set to high. It will be cleared after the writing action has been
 * transferred from APB clock domain to BSC clock domain and then the status
 * has transfered from BSC clock domain back to APB clock domain
 * 
 * We need to wait for the CMDBUSY to clear because the hardware does not have
 * CMD pipeline registers. This wait is to avoid a previous written CMD/data
 * to be overwritten by the following writing before the previous written
 * CMD/data was executed/synchronized by the hardware
 * 
 * We shouldn't set up an interrupt for this since the context switch overhead
 * is too expensive for this type of action and in fact 99% of time we will
 * experience no wait anyway
 * 
 */
static int bsc_wait_cmdbusy(struct bsc_i2c_dev *dev)
{
   int timeout = I2C_TIMEOUT;

   while((readl((uint32_t) dev->virt_base + I2C_MM_HS_ISR_OFFSET) & I2C_MM_HS_ISR_CMDBUSY_MASK) && timeout--)
   {
      udelay(100);
   }

   if (!timeout)
      printf("%s - timeout\n", __func__);

   return 0;
}

static int bsc_send_cmd(struct bsc_i2c_dev *dev, BSC_CMD_t cmd)
{
   int timeout = I2C_TIMEOUT;

   /* make sure the hareware is ready */
   bsc_wait_cmdbusy(dev);

   /* Enable session done interrupt */
   bsc_enable_intr((uint32_t)dev->virt_base, I2C_MM_HS_IER_I2C_INT_EN_MASK);

   /* send the command */
   kona_bsc_send_cmd((unsigned int)dev->virt_base, cmd); // TBD virt_base...

   /*
    * Block waiting for the transaction to finish. When it's finished we'll
    * be signaled by the interrupt
    */
   while(!(readl((uint32_t) dev->virt_base + I2C_MM_HS_ISR_OFFSET) & I2C_MM_HS_ISR_SES_DONE_MASK) && timeout--)
   {
      udelay(100);
   }

   if (!timeout)
      printf("%s - timeout\n", __func__);

   /* Disable session done interrupt */
   bsc_disable_intr((uint32_t)dev->virt_base, I2C_MM_HS_IER_I2C_INT_EN_MASK);

   /* clear command */
   kona_bsc_send_cmd((unsigned int)dev->virt_base, BSC_CMD_NOACTION);

   return 0;
}

static int bsc_xfer_start(struct bsc_i2c_dev *dev)
{
   int rc;

   /* now send the start command */
   rc = bsc_send_cmd(dev, BSC_CMD_START);
   if (rc < 0)
   {
      printf("bsc_xfer_start :: failed to send the start command\n");
      return rc;
   }

   return 0;
}


static int bsc_xfer_repstart(struct bsc_i2c_dev *dev)
{
   int rc;

   rc = bsc_send_cmd(dev, BSC_CMD_RESTART);
   if (rc < 0)
   {
      printf("failed to send the restart command\n");
      return rc;
   }

   return 0;
}

static int bsc_xfer_stop(struct bsc_i2c_dev *dev )
{
   int rc;

   rc = bsc_send_cmd(dev, BSC_CMD_STOP);
   if (rc < 0)
   {
      printf("failed to send the stop command\n");
      return rc;
   }

   return 0;
}

static int bsc_xfer_read_byte(struct bsc_i2c_dev *dev, unsigned short no_ack, unsigned char *data)
{
   int rc;
   BSC_CMD_t cmd;

   if (no_ack)
   {
      cmd = BSC_CMD_READ_NAK;
      debug("NO ACK, value is %d \n", no_ack) ;
   }
   else
   {
      cmd = BSC_CMD_READ_ACK;
      debug("ACK value is %d \n", no_ack) ;
   }

   /* send the read command */
   rc = bsc_send_cmd(dev, cmd);
   if (rc < 0)
      return rc;

   /*
    * Now read the data from the BSC DATA register. Since BSC does not have
    * an RX FIFO, we can only read one byte at a time
    */
   bsc_read_data((unsigned int)dev->virt_base, data, 1);
   
   return 0;
}

static int bsc_xfer_read(struct bsc_i2c_dev *dev, struct i2c_msg *msg)
{
   int rc;
   uint16_t no_ack;
   unsigned int bytes_read, cnt = msg->len;
   unsigned char data, *buf = msg->buf;

   debug(" inside bsc_xfer_read, cnt = %d \n", cnt ) ;

   no_ack = msg->flags & I2C_M_NO_RD_ACK;
   bytes_read = 0;
   while (cnt > 0)
   {
      rc = bsc_xfer_read_byte(dev, (no_ack || (cnt == 1)), &data);
      if (rc < 0)
      {
         debug("problem experienced during data read\n");
         break;     
      }

      debug("reading %2.2X\n", data);

      *buf = data;
      buf++;
      bytes_read++;
      cnt--;
   }

   return bytes_read;
}

static int bsc_xfer_write_byte(struct bsc_i2c_dev *dev, uint16_t nak_ok, unsigned char *data)
{
   int timeout = I2C_TIMEOUT;

   /* make sure the hareware is ready */
   bsc_wait_cmdbusy(dev);

   /* Enable session done interrupt */
   bsc_enable_intr((uint32_t)dev->virt_base, I2C_MM_HS_IER_I2C_INT_EN_MASK);

   /* send data */
   bsc_write_data((uint32_t)dev->virt_base, data, 1);

   /*
    * Block waiting for the transaction to finish. When it's finished we'll
    * be signaled by the interrupt
    */
   while(!(readl((uint32_t) dev->virt_base + I2C_MM_HS_ISR_OFFSET) & I2C_MM_HS_ISR_SES_DONE_MASK) && timeout--)
   {
      udelay(100);
   }

   if (!timeout)
      printf("%s - timeout\n", __func__);

   /* Disable session done interrupt */
   bsc_disable_intr((uint32_t)dev->virt_base, I2C_MM_HS_IER_I2C_INT_EN_MASK);

   /* unexpected NAK */
   if (!bsc_get_ack((uint32_t)dev->virt_base) && !nak_ok)
   {
      debug("unexpected NAK\n");
      return -1;
   }

   return 0;
}

static int bsc_xfer_write(struct bsc_i2c_dev *dev,struct i2c_msg *msg)
{
   int rc;
   unsigned int bytes_written, cnt = msg->len;
   unsigned char *buf = msg->buf;
   uint16_t nak_ok = msg->flags & I2C_M_IGNORE_NAK;

   bytes_written = 0;
	while (cnt > 0)
   {
      rc = bsc_xfer_write_byte(dev,nak_ok, buf);
      if (rc < 0)
      {
         debug("problem experienced during data write\n");
         break;     
      }

      debug("writing %2.2X\n", *buf);
      buf++;
      bytes_written++;
      cnt--;
   }

	return bytes_written;
}

static int bsc_xfer_try_address(struct bsc_i2c_dev *dev , unsigned char addr, unsigned short nak_ok, unsigned int retries)
{
   unsigned int i;
   int rc = 0, success = 0;

   debug("0x%02x, %d\n", addr, retries);

   for (i = 0; i <= retries; i++)
   {
      rc = bsc_xfer_write_byte(dev,nak_ok, &addr);
      if (rc >= 0)
      {
         success = 1;
         break;
      }
   
      /* no luck, let's keep trying */
      rc = bsc_xfer_stop(dev);
      if (rc < 0)
         break;

      rc = bsc_xfer_start(dev);
      if (rc < 0)
         break;
   }

   /* unable to find a slave */
   if (!success)
   {
      debug("tried %u times to contact slave device at 0x%02x but no luck success=%d rc=%d\n", i + 1, addr >> 1, success, rc);
   }

   return rc;
}

static int bsc_xfer_do_addr(struct bsc_i2c_dev *dev ,struct i2c_msg *msg)
{
   int rc;
   unsigned int retries;
   unsigned short flags = msg->flags;
   unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
   unsigned char addr;

   retries = nak_ok ? 0 : 2 ;

   addr = msg->addr << 1;
   if (flags & I2C_M_RD)
      addr |= 1;
   if (flags & I2C_M_REV_DIR_ADDR)
      addr ^= 1;
   rc = bsc_xfer_try_address(dev, addr, nak_ok, retries);
   if (rc < 0)
      return -1;

   return 0;
}

static int bsc_xfer(struct bsc_i2c_dev *dev ,struct i2c_msg msgs[], int num)
{
   struct i2c_msg *pmsg;
   int rc;
   unsigned short i, nak_ok;

   /* send start command */
   rc = bsc_xfer_start(dev);
   if (rc < 0)
   {
      printf("start command failed\n");
      return rc;
   }

   /* loop through all messages */
   for (i = 0; i < num; i++)
   {
      pmsg = &msgs[i];
      nak_ok = pmsg->flags & I2C_M_IGNORE_NAK;

      /* need restart + slave address */
      if (!(pmsg->flags & I2C_M_NOSTART))
      {
         /* send repeated start only on subsequent messages */
         if (i)
         {
            rc = bsc_xfer_repstart(dev);
            if (rc < 0)
            {
               printf("restart command failed\n");
               return rc;
            }
         }

         rc = bsc_xfer_do_addr(dev, pmsg);
         if (rc < 0)
         {
            printf("NAK from device addr %2.2x msg#%d\n", pmsg->addr, i);
            return rc;
         }
      }

      /* read from the slave */
      if (pmsg->flags & I2C_M_RD)
      {
         rc = bsc_xfer_read(dev, pmsg);
         debug("read %d bytes msg#%d\n", rc, i);
         if (rc < pmsg->len)
         {
            debug("read %d bytes but asked for %d bytes\n", rc, pmsg->len);
            return (rc < 0)? rc : -1;
         }
      }
      else /* write to the slave */
      { 
         /* write bytes from buffer */
         rc = bsc_xfer_write(dev, pmsg);
         debug("wrote %d bytes msg#%d\n", rc, i);
         if (rc < pmsg->len)
         {
            debug("wrote %d bytes but asked for %d bytes\n", rc, pmsg->len);
            return (rc < 0)? rc : -1;
         }
      }
   }

   /* send stop command */
	rc = bsc_xfer_stop(dev);
   if (rc < 0)
      printf("stop command failed\n");

	return (rc < 0) ? rc : num;
}

static int bsc_initialize(unsigned char index)
{
   struct bsc_i2c_dev *dev ; 

   dev = &g_i2c_devs[index] ;

   bsc_set_bus_speed((uint32_t)dev->virt_base, gBusSpeedTable[dev->speed]);

   /* init I2C controller */
   kona_bsc_init((uint32_t)dev->virt_base);

   /* disable and clear interrupts */
   bsc_disable_intr((uint32_t)dev->virt_base, 0xFF);
   bsc_clear_intr_status((uint32_t)dev->virt_base, 0xFF);

   /* enable command busy interrupt */
   bsc_enable_intr((uint32_t)dev->virt_base, I2C_MM_HS_IER_CMDBUSY_INT_EN_MASK);

   /* disable BSC TX FIFO */
   bsc_set_FIFO((uint32_t)dev->virt_base, 0);

   bsc_set_autosense((uint32_t)dev->virt_base, 1);

   return 0;
}


/**************** uboot layer ********************/
void i2c_init (int speed, int slaveaddr)
{
	debug("i2c_init: init bus number %d\n", current_bus);
    bsc_initialize(current_bus) ;
	bus_initialized[current_bus] = 1;
}


int i2c_read (uchar chip, uint addr, int alen, uchar *buffer, int len)
{

	unsigned char msgbuf0[64] = {0} ;
	unsigned int i = 0 ;
	struct i2c_msg msg[2] = { { 0, 0, 1, msgbuf0 },
	                          { 0, 0 | I2C_M_RD, 0, 0 }
	                        };

	msg[0].addr = chip ;
	msg[1].addr = chip ;
	msg[1].buf = &buffer[0] ;

    debug("i2c_read function called , inputs are chip = 0x%x, addr = 0x%x, alen = %d, len = %d \n", chip, addr, alen, len ) ;


	for ( i = 0 ; i < len ; i++ ) 
	{
        msg[1].len = 1 ;
        msgbuf0[0] = (unsigned char)(addr+i) ;
		msg[1].buf = &buffer[i] ;
        if (bsc_xfer(&g_i2c_devs[current_bus],msg, 2) < 0)  /* Sending 2 i2c messages */ 
        {
			i2c_init(CONFIG_SYS_I2C_SPEED, 0);
			debug ("I2C read: I/O error\n");
			return 1;
        }
    }
    return 0 ;
}

int i2c_write (uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	unsigned char msgbuf0[64] = {0} ;
	struct i2c_msg msg[1] = { { 0x08, 0, 2, msgbuf0 } };

    debug("i2c_write function called , inputs are chip = 0x%x, addr = 0x%x, alen = %d, len = %d \n", chip, addr, alen, len ) ;

	msg[0].addr = chip ;
	msgbuf0[0] = addr ;
	msgbuf0[1] = *buffer ;

    if (bsc_xfer(&g_i2c_devs[current_bus],msg, 1) < 0)
    {
		i2c_init(CONFIG_SYS_I2C_SPEED, 0);
		debug ("I2C read: I/O error\n");
		return 1;
    }

    return 0 ;
}

int i2c_probe (uchar chip)
{
	uchar tmp;

	/*
	 * read addr 0x0 of the given chip. 
	 */
	return (i2c_read(chip, 0x0, 1, &tmp, 1));
}

int i2c_set_bus_num(unsigned int bus)
{
    debug("i2c_set_bus_num , %d \n", bus ) ;

	if ((bus < 0) || (bus >= MAX_I2C_CONTROLLERS)) {
		printf("Bad bus: %d\n", bus);
		return -1;
	}

	current_bus = bus;

	if(!bus_initialized[current_bus])
		i2c_init(CONFIG_SYS_I2C_SPEED, 0);

	return 0;
}

int i2c_get_bus_num(void)
{
    debug("i2c_get_bus_num function called \n") ;
	return current_bus;
}


