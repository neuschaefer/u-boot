/*
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 *
 * WPCM450 serial driver.
 *  
 * Copyright (C) 2007 Avocent Corp.
 *
 * This file is subject to the terms and conditions of the GNU 
 * General Public License. This program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 */


#include "common.h"
#include "cdefs.h"
#include "com_defs.h"
#include "wpcm450_clk_regs.h"
#include "wpcm450_gctrl_regs.h"
#include "wpcm450_uart_regs.h"
#include "clock_init.h"
#include "wpcm450_platform.h"

#ifdef CONFIG_WPCM450_WHOVILLE
#define CLKSEL  VPlong(GCR_BA+0x204)
/* Host UARTs Clock Source Select Bit: 
   0: 48 MHz Clock (divide by 1, default).
   1: 24 MHz Clock (divide by 2). This option must be used. */
#define HUARTSEL_BIT 10
#endif

/* default setial port number */
static UINT8 serial_port = 0;
static UINT8 serial_mux = 0xFF;

/* platform ID */
extern UINT8 platform_id;


#if 0
#define PRINTD(fmt,args...) printf("SERIAL: " fmt ,##args)
#else
#define PRINTD(fmt,args...)
#endif


int serial_change_port(UINT8 port)
{
    PRINTD("serial_change_port, port=%d\n", port);
    
    if (port <= 1)
    {
        serial_port = port;
        
        /* re-init serial port */
        serial_init();
        
        return 0;
    }
    else
    {
        PRINTD("port number is not correct\n");
        return -1;
    }
}


int serial_mux_save(void)
{
    /* handle for external serial mux */
#ifdef CONFIG_WPCM450_WHOVILLE
    if ((platform_id != PF_ID_MCBEAN)
        && (platform_id != PF_ID_MAYZIE)
        && (platform_id != PF_ID_SAMIAM)
        && (platform_id != PF_ID_HORTON)
        && (platform_id != PF_ID_DIAMAS)
        && (platform_id != PF_ID_COASTER))
    {
        /* serial Mux, 0xC4000021 bit 0-2, CPLD base 0xC4000000 */
        serial_mux = *((UINT8 *) 0xC4000021);
        
        return 0;
    }
#endif
    
    /* handle for internal serial mux */
    serial_mux = GET_FIELD(SPSWC, SPSWC_SPMOD);
    
    return 0;
}


int serial_mux_restore(void)
{
    /* handle for external serial mux */
#ifdef CONFIG_WPCM450_WHOVILLE
    if ((platform_id != PF_ID_MCBEAN)
        && (platform_id != PF_ID_MAYZIE)
        && (platform_id != PF_ID_SAMIAM)
        && (platform_id != PF_ID_HORTON)
		&& (platform_id != PF_ID_DIAMAS)
		&& (platform_id != PF_ID_COASTER))
    {
        /* serial Mux, 0xC4000021 bit 0-2, CPLD base 0xC4000000 */
        *((UINT8 *) 0xC4000021) = serial_mux;
        
        /* delay for switching serial port */
        udelay(10000);
        
        return 0;
    }
#endif
    
    /* handle for internal serial mux */
    SET_FIELD(SPSWC, SPSWC_SPMOD, serial_mux);
    
    return 0;
}


int serial_init(void) 
{
    /* UINT32 clk_div; */
    /* UINT32 uart_div; */
    /* UINT32 baudRate; */
    UINT32 divisor;
    /* UINT32 uart_clock; */
    
    /* baudRate = 115200; */
    
    PRINTD("serial_init, port=%d\n", serial_port);
    
    /* save first boot mux position */
    if (serial_mux == 0xFF)
    {
        serial_mux_save();
    }
    
#ifdef CONFIG_WPCM450_WHOVILLE
    if((platform_id == PF_ID_MCBEAN)
                || (platform_id == PF_ID_MAYZIE)
                || (platform_id == PF_ID_SAMIAM)
                || (platform_id == PF_ID_HORTON)
                || (platform_id == PF_ID_DIAMAS)
                || (platform_id == PF_ID_COASTER))
    {
        SET_BIT(CLKSEL, HUARTSEL_BIT);
    }
#endif

    /************************************************************************/
    /* Muxing for UART                                                      */
    /************************************************************************/
    switch (serial_port)
    {
        case 0:
            
            /* set default as BMC UART 0 */
            SET_BIT(MFSEL1, MF_BSPSEL_BIT);
            CLEAR_BIT(MFSEL1, MF_HSP1SEL_BIT);
            CLEAR_BIT(MFSEL1, MF_HSP2SEL_BIT);
            
            break;
            
        case 1:
            
            /* selcet serial interface 2 */
            SET_BIT(MFSEL1, MF_BSPSEL_BIT);
            CLEAR_BIT(MFSEL1, MF_HSP1SEL_BIT);
            SET_BIT(MFSEL1, MF_HSP2SEL_BIT);
            
            /* handel for external serial mux */
#ifdef CONFIG_WPCM450_WHOVILLE
            if ((platform_id != PF_ID_MCBEAN)
                && (platform_id != PF_ID_MAYZIE)
                && (platform_id != PF_ID_SAMIAM)
                && (platform_id != PF_ID_HORTON)
                && (platform_id != PF_ID_DIAMAS)
                && (platform_id != PF_ID_COASTER))
            {
                /* serial Mux, 0xC4000021 bit 0-2 -> 3, CPLD base 0xC4000000 */
                
                /* #define CPLD_MUX_MODE_1     3 (011) */
                /* #define CPLD_MUX_MODE_2A    0 (000) */
                /* #define CPLD_MUX_MODE_2A_S  4 (100) */ 
                /* #define CPLD_MUX_MODE_2B    1 (001) */
                /* #define CPLD_MUX_MODE_2C    2 (010) */
                
                /* force to mode 1 to direct data to host serial port 2 */
                *((UINT8 *) 0xC4000021) = ((*((UINT8 *) 0xC4000021) & 0xF8) | 0x03);
                
                PRINTD("af *((UINT8 *) 0xC4000021)=%lx\n", *((UINT8 *) 0xC4000021));
                
                /* delay for switching serial port */
                udelay(10000);
            }
#endif
            
            /* handle for internal serial mux */
            switch (platform_id)
            {
#ifdef CONFIG_WPCM450_WHOVILLE
                case PF_ID_MCBEAN:
                case PF_ID_MAYZIE:
                case PF_ID_SAMIAM:
                case PF_ID_HORTON:
                case PF_ID_DIAMAS:
                case PF_ID_COASTER:
                    /* mode 3: core direct to host serial port 2 */
                    SET_FIELD(SPSWC, SPSWC_SPMOD, SERIAL_MODE3_DIRECT);
                    
                    break;
#endif
                    
                default:
                    /* mode 2: core take-over */
                    SET_FIELD(SPSWC, SPSWC_SPMOD, SERIAL_MODE2_TAKEOVER);
                    break;
            }
            
            break;
            
        default:
            
            /* set default as BMC UART 0 */
            SET_BIT(MFSEL1, MF_BSPSEL_BIT);
            CLEAR_BIT(MFSEL1, MF_HSP1SEL_BIT);
            CLEAR_BIT(MFSEL1, MF_HSP2SEL_BIT);
            
            break;
    }
    
#if 0
    
    /* UART clock can not be larger than APB clock */
    clk_div = CLK_DIV;
    
    /* CPU divider */
    uart_div = 2;
    
    /* MC divider */
    uart_div = uart_div * (GET_FIELD(clk_div, CLK_MCCKDIV) + 1);
    
    /* AHB1 divider */
    uart_div = uart_div * (1 << GET_FIELD(clk_div, CLK_AHBCKDIV));
    
    /* APB divider */
    uart_div = uart_div * (1 << GET_FIELD(clk_div, CLK_APBCKDIV));
    
    SET_FIELD(CLK_DIV, CLK_UARTDIV, (uart_div - 1));
    
    uart_clock = RMII_RCKREF_CLK_RATE / uart_div;
    
    /* First, compute the baud rate divisor. */
    divisor = (uart_clock / (baudRate * 16)) - 2;
    
#else

    divisor = 11;

#endif
    
    if(divisor < 0)
    {
        divisor = 0;
    }
    
    /************************************************************************/
    /* Select 48M clock source for UART in case that real chip and          */
    /* PLL0 in case that PALLADIUM chip emulator                            */
    /************************************************************************/
    
    SET_FIELD(CLK_SEL, CLK_UART_SRC, CLK_48M_SRC_SEL);
    
    /* CWS: Disable interrupts */
    UART_LCR(serial_port) = 0;          /* prepare to Init UART */
    UART_IER(serial_port) = 0x0;        /* Disable all UART interrupt */
    
    /* CWS: Set baud rate to baudRate bps */
    UART_LCR(serial_port) |= DLAB;      /* prepare to access Divisor */
    UART_DLL(serial_port) = GET_LSB((int) divisor);
    UART_DLM(serial_port) = GET_MSB((int) divisor);
    UART_LCR(serial_port) &= ~DLAB;     /* prepare to access RBR, THR, IER */
    
    /* CWS: Set port for 8 bit, 1 stop, no parity */
    UART_LCR(serial_port) = UART_8bit;
    
    if (serial_port == 1)
    {
        UART_MCR(serial_port) = 0x00;
    }
    
    /* CWS: Set the RX FIFO trigger level, reset RX, TX FIFO */
    UART_FCR(serial_port) = rUART_FCR;
    UART_TOR(serial_port) = 0x0;
    
    return 0;
}


void serial_putc(const char c)
{
    unsigned int Status;
    
    do
    {
        Status = GET_STATUS(serial_port);
    }
    while (!TX_READY(Status));      /* wait until ready */
    
    PUT_CHAR(serial_port, c);
    
    if (c == '\n')
    {
        do
        {
            Status = GET_STATUS(serial_port);
        }
        while (!TX_READY(Status));   /* wait until ready */
        
        PUT_CHAR(serial_port, '\r');
    }
}


void serial_puts(const char *s)
{
    while (*s) 
    {
        serial_putc(*s++);
    }
}


int serial_getc(void)
{
    unsigned int Status;
    unsigned int Ch;
    
    Ch = 0;
    
    if (Ch == 0)
    {
        do
        {
            Status = GET_STATUS(serial_port);
        }
        while (!RX_DATA(Status));   /* wait until ready */
        
        Ch = GET_CHAR(serial_port);
    }
    
    return ((int)Ch);
}


int serial_tstc(void)
{
    unsigned int Status; 
    
    Status = GET_STATUS(serial_port);
    
    if (RX_DATA(Status)) 
        return (1); 
    
    /* no chars available */ 
    return 0;
}


void serial_setbrg(void)
{
    return;
}
