#include <common.h>
#include "asm/types.h"
#include "com_defs.h"
#include "wpcm450_aic.h"
#include "wpcm450_aic_regs.h"

// Interrupt Handler Table

typedef struct _irq_handler_entry_t{
 int_func_t func;
 u32 param;
}irq_handler_entry_t;

static irq_handler_entry_t irq_handler_table[MAX_INT_NUM];

static void wpcm450_irq_handler(void);

s32 wpcm450_aic_initialize (void)
{
  //s32 tmp;
	int i;

  /* clean interrupt handler function pointers table */
	
  for(i=0; i<MAX_INT_NUM; i++)
	{
	  irq_handler_table[i].func = NULL;
		irq_handler_table[i].param = 0;
	}

  /* Disable interrupts */
	
	disable_interrupts();

	/* Insert to interrupt vector int_handler function pointer */
	
	*((volatile u32 *)0x38)=(u32)wpcm450_irq_handler;

	/* Enable interrupts */
	
  enable_interrupts();

  return 0;
}


s32 wpcm450_register_int(s32 int_num, int_func_t func, u32 param)
{
  if((int_num >0) && (int_num < MAX_INT_NUM))
  {
    irq_handler_table[int_num].func = func;
		irq_handler_table[int_num].param = param;
  }
  else
  {
    return 1;
  }

	return 0;
}


void wpcm450_enable_int(u32 int_num)
{
  if(int_num < MAX_INT_NUM)
	{
     AIC_MECR = (u32)(1<<(int_num));
	}
    
}


void wpcm450_disable_int(u32 int_num)
{
  if(int_num < MAX_INT_NUM)
	{
    AIC_MDCR = (u32)(1<<(int_num));
	}
}


// Interrupt Service Routine                                                                         */
static void wpcm450_irq_handler(void)
{
 	u32 IPER;//, //ISNR;
	u32 irq;
  int i;
	
  irq=AIC_ISR;


  IPER = AIC_IPER;


  for (i=0; i<MAX_INT_NUM; i++)
	{
    if( irq & (0x1<<i) )
		{
      (*irq_handler_table[i].func)(irq_handler_table[i].param);
	  }
	}
  
  AIC_EOSCR = (u32)0;

}
