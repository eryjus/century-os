
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#define PUT32(a,v) (*((volatile unsigned int *)a) = v)
#define GET32(a)   (*((volatile unsigned int *)a))
extern void dummy ( unsigned int );

#define GPFSEL1 0x3f200004
#define GPSET0  0x3f20001C
#define GPCLR0  0x3f200028
#define GPPUD       0x3f200094
#define GPPUDCLK0   0x3f200098

#define AUX_ENABLES     0x3f215004
#define AUX_MU_IO_REG   0x3f215040
#define AUX_MU_IER_REG  0x3f215044
#define AUX_MU_IIR_REG  0x3f215048
#define AUX_MU_LCR_REG  0x3f21504C
#define AUX_MU_MCR_REG  0x3f215050
#define AUX_MU_LSR_REG  0x3f215054
#define AUX_MU_MSR_REG  0x3f215058
#define AUX_MU_SCRATCH  0x3f21505C
#define AUX_MU_CNTL_REG 0x3f215060
#define AUX_MU_STAT_REG 0x3f215064
#define AUX_MU_BAUD_REG 0x3f215068

//GPIO14  TXD0 and TXD1
//GPIO15  RXD0 and RXD1
//alt function 5 for uart1
//alt function 0 for uart0

//((250,000,000/115200)/8)-1 = 270

void uart_putc(int c)
{
    while(1)
    {
        if(GET32(AUX_MU_LSR_REG)&0x20) break;
    }
    PUT32(AUX_MU_IO_REG,c);
}

int notmain ( void )
{
    unsigned int ra;

    PUT32(AUX_ENABLES,1);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_CNTL_REG,0);
    PUT32(AUX_MU_LCR_REG,3);
    PUT32(AUX_MU_MCR_REG,0);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_IIR_REG,0xC6);
    PUT32(AUX_MU_BAUD_REG,270);

    ra=GET32(GPFSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    PUT32(GPFSEL1,ra);

    PUT32(GPPUD,0);
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,(1<<14));
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,0);

    PUT32(AUX_MU_CNTL_REG,2);

    ra=0;
    while(1)
    {
        uart_putc(0x30 + (ra++&7));
    }

    return(0);
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//
// Copyright (c) 2012 David Welch dwelch@dwelch.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------
