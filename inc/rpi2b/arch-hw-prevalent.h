//===================================================================================================================
//
//  arch-hw.h -- This file aggregates all the hardware port definitions and bit flags for the rpi2b
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  TODO: The USB hardware documentation is available at
//  https://www.synopsys.com/dw/doc.php/iip/DWC_otg/latest/doc/DWC_otg_databook.pdf .  This site requires
//  sign-up in order to access the documentation.  While I will eventually want to get to this document and
//  and support the USB controller, I have no appetite to take that on with a sign-up (costs?) at the moment.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Mar-28  Initial   0.0.0   ADCL  Initial version
//  2018-Nov-13  Initial   0.2.0   ADCL  Copied this file from century
//
//===================================================================================================================


#ifndef __HW_H__
#   error "Do not include 'arch-hw-prevalent.h' directly.  Include 'hw.h' to pick up this file."
#endif


#ifndef KERNEL_OFFSET
#   define KERNEL_OFFSET 0
#endif


//
// -- This is the hardware MMIO base location -- everything is based off this location
//    --------------------------------------------------------------------------------
#define HW_BASE             (0x3f000000 + KERNEL_OFFSET)


//-------------------------------------------------------------------------------------------------------------------
// Auxiliaries: UART1 & SPI1 & SPI2
//-------------------------------------------------------------------------------------------------------------------


//
// -- The 3 auxiliary peripherals (which are intermingled and controlled together)
//    ----------------------------------------------------------------------------

#define AUX_IRQ             (HW_BASE+0x215000)          // Auxiliary Interrupt Status
//-------------------------------------------------------------------------------------------------------------------
#define AUXIRQ_UART         (1<<0)                      // For AUX_IRQ, UART interrupt pending?
#define AUXIRQ_SPI1         (1<<1)                      // For AUX_IRQ, SPI1 interrupt pending?
#define AUXIRQ_SPI2         (1<<2)                      // For AUX_IRQ, SPI2 interrupt pending?


#define AUX_ENABLES         (HW_BASE+0x215004)          // Auxiliary Enables
//-------------------------------------------------------------------------------------------------------------------
#define AUXENB_UART         (1<<0)                      // For AUX_IRQ, UART enable
#define AUXENB_SPI1         (1<<1)                      // For AUX_IRQ, SPI1 enable
#define AUXENB_SPI2         (1<<2)                      // For AUX_IRQ, SPI2 enable


//
// -- The auxiliary Mini UART
//    -----------------------

#define AUX_MU_IO_REG       (HW_BASE+0x215040)          // Mini UART I/O Data
//-------------------------------------------------------------------------------------------------------------------
#define AUXMUART_TX         (0xff)                      // Transmit data bits
#define AUXMUART_RX         (0xff)                      // Receive data bits


#define AUX_MU_IER_REG      (HW_BASE+0x215044)          // Mini UART Interrupt Enable
//-------------------------------------------------------------------------------------------------------------------
#define AUXMUIER_RXENB      (1<<1)                      // Generate interrupt when RX FIFO queue has data
#define AUXMUIER_TXENB      (1<<1)                      // Generate interrupt when TX FIFO queue is empty


#define AUX_MU_IIR_REG      (HW_BASE+0x215048)          // Mini UART Interrupt Identify
//-------------------------------------------------------------------------------------------------------------------
#define AUXMUIIR_TXEMPTY    (1<<1)                      // Set when TX FIFO is empty
#define AUXMUIIR_RXEMPTY    (2<<1)                      // Set when RX FIFO is empty
#define AUXMUIIR_RXCLR      (1<<1)                      // Clear RX FIFO queue
#define AUXMUIIR_TXCLR      (2<<1)                      // Clear RX FIFO queue

#define SH_AUXMUIIR(x)    (((x)&0x3)<<1)                // Shift the bits for this field


#define AUX_MU_LCR_REG      (HW_BASE+0x21504c)          // Mini UART Line Control
//-------------------------------------------------------------------------------------------------------------------
#define AUXMULCR_DLAB       (1<<7)                      // Set to access baud rate register; clear for operation
#define AUXMULCR_BRK        (1<<6)                      // Set to indicate break conditions
#define AUXMULCR_SIZE       (1<<0)                      // Data Size: 0 = 7-bits; 1 = 8 bits


#define AUX_MU_MCR_REG      (HW_BASE+0x215050)          // Mini UART Modem Control
//-------------------------------------------------------------------------------------------------------------------
#define AUXMUMCR_RTS        (1<<1)                      // Clr if RTS it high; Set if RTS is low


#define AUX_MU_LSR_REG      (HW_BASE+0x215054)          // Mini UART Line Status
//-------------------------------------------------------------------------------------------------------------------
#define AUXMULSR_TXIDL      (1<<6)                      // Set if TX FIFO is empty and transmitter is idle
#define AUXMULSR_TXRMPTY    (1<<5)                      // Set if TX FIFO can accept at least 1 char
#define AUXMULSR_RXOVER     (1<<1)                      // Set if RX FIFO overrun (recent chars discarded)
#define AUXMULSR_RXRDY      (1<<0)                      // Set if RX FIFO has 1 char


#define AUX_MU_MSR_REG      (HW_BASE+0x215058)          // Mini UART Modem Status
//-------------------------------------------------------------------------------------------------------------------
#define AUXMUMSR_CTS        (1<<5)                      // Set if CTS is low; clr if CTS hi


#define AUX_MU_SCRATCH      (HW_BASE+0x21505c)          // Mini UART Scratch
//-------------------------------------------------------------------------------------------------------------------
#define AUXMU_BYTE          (0xff)                      // Single byte of storage


#define AUX_MU_CNTL_REG     (HW_BASE+0x215060)          // Mini UART Extra Control
//-------------------------------------------------------------------------------------------------------------------
#define AUXMUCTL_CTS        (1<<7)                      // clear if CTS assert level low
#define AUXMUCTL_RTS        (1<<6)                      // clear if RTS assert level low
#define AUXMUCTL_RTSAUTO    (3<<4)                      // de-assert: 00: 3 Byt; 01: 2 Byt; 10: 1 Byt; 11: 4 Byt
#define AUXMUCTL_ENBCTSAUTO (1<<3)                      // enable auto flow control using CTS
#define AUXMUCTL_ENBRTSAUTO (1<<2)                      // enable auto flow control using RTS
#define AUXMUCTL_ENBTX      (1<<1)                      // enable transmitter
#define AUCMUCTL_RNBRX      (1<<0)                      // enable receiver

#define SH_AUXMUCTLRTS(x)   (((x)&0x3)<<4)              // adjust RTS-AUTO to the right bits


#define AUX_MU_STAT_REG     (HW_BASE+0x215064)          // Mini UART Extra Status
//-------------------------------------------------------------------------------------------------------------------
#define AUXMUST_TXLVL       (0xf<<24)                   // TX FIFO level (0-8)
#define AUXMUST_RXLVL       (0xf<<16)                   // RX FIFO level (0-8)
#define AUXMUST_TXDONE      (1<<9)                      // Logical And of AUXMUST_TXEMPTY & AUXMUST_RXIDLE
#define AUXMUST_TXEMPTY     (1<<8)                      // TX FIFO empty (AUXMUST_TXLVL == 0)
#define AUXMUST_CTS         (1<<7)                      // Status of CTS line
#define AUXMUST_RTS         (1<<6)                      // Status of RTS line
#define AUXMUST_TXFULL      (1<<5)                      // TX FIFO Full
#define AUXMUST_RXOVER      (1<<4)                      // Receiver Overrun
#define AUXMUST_TXIDLE      (1<<3)                      // TX Idle (!AUXMUST_SPACE)
#define AUXMUST_RXIDLE      (1<<2)                      // RXC Idle
#define AUXMUST_SPACE       (1<<1)                      // TX FIFO has room
#define AUXMUST_AVAIL       (1<<0)                      // RX FIFO has data


#define AUX_MU_BAUD_REG     (HW_BASE+0x215068)          // Mini UART Baudrate
//-------------------------------------------------------------------------------------------------------------------
#define AUXMU_BAUD          (0xffff)                    // Baud Rate


//
// -- The auxiliary SPI 1
//    -------------------
#define AUX_SPI0_CNTL0_REG  (HW_BASE+0x215080)          // SPI 1 Control register 0
//-------------------------------------------------------------------------------------------------------------------
#define AUXSPI0CTL0_SPD     (0xfff<<20)                 // Speed Control (System Clock Freq/(2 * (SPD+1)))
#define AUXSPI0CTL0_CHP     (0x7<<17)                   // Chip Select
#define AUXSPI0CTL0_PIM     (1<<16)                     // Post Input Mode
#define AUXSPI0CTL0_VCS     (1<<15)                     // Variable Chip Sel (1=TX FIFO; 0=CHP)
#define AUXSPI0CTL0_VWID    (1<<14)                     // Variable Width (1=TX FIFO; 0=SHL)
#define AUXSPI0CTL0_DOUT    (3<<12)                     // Extra Hold clk cycles (00=None; 01=1clk; 10=4clk; 11=7clk)
#define AUXSPI0CTL0_ENB     (1<<11)                     // Enabled
#define AUXSPI0CTL0_IRIS    (1<<10)                     // in: data clocked on (1=rising; 0=falling) edge of cycle
#define AUXSPI0CTL0_CLR     (1<<9)                      // Clear FIFO queues
#define AUXSPI0CTL0_ORIS    (1<<8)                      // out: data clocked on (1=rising; 0=falling) edge of cycle
#define AUXSPI0CTL0_INV     (1<<7)                      // invert clock (1=idle high)
#define AUXSPI0CTL0_SBIT    (1<<6)                      // shift out starting with (1=MS; 0=LS) bit first
#define AUXSPI0CTL0_SHL     (0x3f)                      // Shift length

#define SH_AUXSPI0CTL0SPD(x) (((x)&0xfff)<<20)          // shift to the correct position
#define SH_AUXSPI0CTL0CHP(x) (((x)&0x7)<<17)            // shift to the correct position


#define AUX_SPI0_CNTL1_REG  (HW_BASE+0x215084)          // SPI 1 Control register 1
//-------------------------------------------------------------------------------------------------------------------
#define AUXSPI0CTL1_CSHI    (7<<8)                      // Additional clock cycles where CS is high
#define AUXSPI0CTL1_TXIRQ   (1<<7)                      // Set for IRQ when TX FIFO empty
#define AUXSPI0CTL1_DONE    (1<<6)                      // Set for IRQ when idle
#define AUXSPI0CTL1_SBIT    (1<<1)                      // shift in starting with (1=MS; 0=LS) bit first (CTL0: out)
#define AUXSPI0CTL1_KEEP    (1<<0)                      // set to keep input

#define SH_AUXSPI0CTL1CSHI(x) (((x)&0x7)<<8)            // shift to the correct position


#define AUX_SPI0_STAT_REG   (HW_BASE+0x215088)          // SPI 1 Status
//-------------------------------------------------------------------------------------------------------------------
#define AUXSPI0STAT_TX      (0xff<<24)                  // Number of units in TX FIFO
#define AUXSPI0STAT_RX      (0xfff<<12)                 // Number of units in RX FIFO
#define AUXSPI0STAT_TXFULL  (1<<9)                      // TX FIFO Full
#define AUXSPI0STAT_TXEMPTY (1<<8)                      // TX FIFO Empty
#define AUXSPI0STAT_RXEMPTY (1<<7)                      // RX FIFO Empty
#define AUXSPI0STAT_BUSY    (1<<6)                      // Transferring Data
#define AUXSPI0STAT_BCNT    (0x3f)                      // Bit Count to process

#define SH_AUXSPI0STATTX(x) (((x)&0xff)<<24)            // shift to the correct position
#define SH_AUXSPI0STATRX(x) (((x)&0xfff)<<12)           // shift to the correct position


#define AUX_SPI0_IO_REG     (HW_BASE+0x215090)          // SPI 1 Data
//-------------------------------------------------------------------------------------------------------------------
#define AUX_SPI0_IO_DATA    (0xffff)                    // This is the data portion


#define AUX_SPI0_PEEK_REG   (HW_BASE+0x215094)          // SPI 1 Peek
//-------------------------------------------------------------------------------------------------------------------
#define AUX_SPI0_PEEK_DATA  (0xffff)                    // This is the data portion


//
// -- The auxiliary SPI 2
//    -------------------
#define AUX_SPI1_CNTL0_REG  (HW_BASE+0x2150c0)          // SPI 2 Control register 0
//-------------------------------------------------------------------------------------------------------------------
#define AUXSPI1CTL0_SPD     (0xfff<<20)                 // Speed Control (System Clock Freq/(2 * (SPD+1)))
#define AUXSPI1CTL0_CHP     (0x7<<17)                   // Chip Select
#define AUXSPI1CTL0_PIM     (1<<16)                     // Post Input Mode
#define AUXSPI1CTL0_VCS     (1<<15)                     // Variable Chip Sel (1=TX FIFO; 0=CHP)
#define AUXSPI1CTL0_VWID    (1<<14)                     // Variable Width (1=TX FIFO; 0=SHL)
#define AUXSPI1CTL0_DOUT    (3<<12)                     // Extra Hold clk cycles (00=None; 01=1clk; 10=4clk; 11=7clk)
#define AUXSPI1CTL0_ENB     (1<<11)                     // Enabled
#define AUXSPI1CTL0_IRIS    (1<<10)                     // in: data clocked on (1=rising; 0=falling) edge of cycle
#define AUXSPI1CTL0_CLR     (1<<9)                      // Clear FIFO queues
#define AUXSPI1CTL0_ORIS    (1<<8)                      // out: data clocked on (1=rising; 0=falling) edge of cycle
#define AUXSPI1CTL0_INV     (1<<7)                      // invert clock (1=idle high)
#define AUXSPI1CTL0_SBIT    (1<<6)                      // shift out starting with (1=MS; 0=LS) bit first
#define AUXSPI1CTL0_SHL     (0x3f)                      // Shift length

#define SH_AUXSPI1CTL0SPD(x) (((x)&0xfff)<<20)          // shift to the correct position
#define SH_AUXSPI1CTL0CHP(x) (((x)&0x7)<<17)            // shift to the correct position


#define AUX_SPI1_CNTL1_REG  (HW_BASE+0x2150c4)          // SPI 2 Control register 1
//-------------------------------------------------------------------------------------------------------------------
#define AUXSPI1CTL1_CSHI    (7<<8)                      // Additional clock cycles where CS is high
#define AUXSPI1CTL1_TXIRQ   (1<<7)                      // Set for IRQ when TX FIFO empty
#define AUXSPI1CTL1_DONE    (1<<6)                      // Set for IRQ when idle
#define AUXSPI1CTL1_SBIT    (1<<1)                      // shift in starting with (1=MS; 0=LS) bit first (CTL0: out)
#define AUXSPI1CTL1_KEEP    (1<<0)                      // set to keep input

#define SH_AUXSPI1CTL1CSHI(x) (((x)&0x7)<<8)            // shift to the correct position


#define AUX_SPI1_STAT_REG   (HW_BASE+0x2150c8)          // SPI 2 Status
//-------------------------------------------------------------------------------------------------------------------
#define AUXSPI1STAT_TX      (0xff<<24)                  // Number of units in TX FIFO
#define AUXSPI1STAT_RX      (0xfff<<12)                 // Number of units in RX FIFO
#define AUXSPI1STAT_TXFULL  (1<<9)                      // TX FIFO Full
#define AUXSPI1STAT_TXEMPTY (1<<8)                      // TX FIFO Empty
#define AUXSPI1STAT_RXEMPTY (1<<7)                      // RX FIFO Empty
#define AUXSPI1STAT_BUSY    (1<<6)                      // Transferring Data
#define AUXSPI1STAT_BCNT    (0x3f)                      // Bit Count to process

#define SH_AUXSPI1STATTX(x) (((x)&0xff)<<24)            // shift to the correct position
#define SH_AUXSPI1STATRX(x) (((x)&0xfff)<<12)           // shift to the correct position


#define AUX_SPI1_IO_REG     (HW_BASE+0x2150d0)          // SPI 2 Data
//-------------------------------------------------------------------------------------------------------------------
#define AUX_SPI1_IO_DATA    (0xffff)                    // This is the data portion


#define AUX_SPI1_PEEK_REG   (HW_BASE+0x2150d4)          // SPI 2 Peek
//-------------------------------------------------------------------------------------------------------------------
#define AUX_SPI1_PEEK_DATA  (0xffff)                    // This is the data portion


//-------------------------------------------------------------------------------------------------------------------
// Broadcom Serial Controllers: BSC0 & BSC1 (note BSC2 should not be accessed by software)
//-------------------------------------------------------------------------------------------------------------------


//
// -- The BSC0 controller
//    -------------------
#define BSC0_BASE           (HW_BASE+0x205000)          // The base address of BSC0


#define BSC0_C              (BSC0_BASE)                 // Control
//-------------------------------------------------------------------------------------------------------------------
#define BSC0C_I2CEN         (1<<15)                     // I2C Enable
#define BSC0C_INTR          (1<<10)                     // Interrupt on RXR=1
#define BSC0C_INTT          (1<<9)                      // Interrupt on TXW=1
#define BSC0C_INTD          (1<<8)                      // Interrupt on DONE=1
#define BSC0C_ST            (1<<7)                      // Start new transfer
#define BSC0C_CLEAR         (3<<4)                      // Clear FIFO if either bit is set
#define BSC0C_READ          (1<<0)                      // Read Packet transfer

#define SH_BSC0C_CLEAR(x)   (((x)&3)<<4)                // shift bits to the right location


#define BSC0_S              (BSC0_BASE+4)               // Status
//-------------------------------------------------------------------------------------------------------------------
#define BSC0S_CLKT          (1<<9)                      // 1=Slave has held SCL too long;  Write 1 to clear
#define BSC0S_ERR           (1<<8)                      // 1=Slave has not ACK address;  Write 1 to clear
#define BSC0S_RXF           (1<<7)                      // 1=FIFO full
#define BSC0S_TXE           (1<<6)                      // 1=FIFO empty
#define BSC0S_RXD           (1<<5)                      // 1=FIFO has data
#define BSC0S_TXD           (1<<4)                      // 1=FIFO has room
#define BSC0S_RXR           (1<<3)                      // 1=FIFO full; read underway
#define BSC0S_TXW           (1<<2)                      // 1=FIFO fill; write underway
#define BSC0S_DONE          (1<<1)                      // 1=Transfer complete
#define BSC0S_TA            (1<<0)                      // 1=Transfer active


#define BSC0_DLEN           (BSC0_BASE+8)               // Data Length
//-------------------------------------------------------------------------------------------------------------------
#define BSC0DLEN_DLEN       (0xffff)                    // Data Length


#define BSC0_A              (BSC0_BASE+0xc)             // Slave Address
//-------------------------------------------------------------------------------------------------------------------
#define BSC0A_ADDR          (0xffff)                    // Slave Address


#define BSC0_FIFO           (BSC0_BASE+0x10)            // Data FIFO
//-------------------------------------------------------------------------------------------------------------------
#define BSC0FIFO_DATA       (0xffff)                    // Data


#define BSC0_DIV            (BSC0_BASE+0x14)            // Clock Divider
//-------------------------------------------------------------------------------------------------------------------
#define BSC0DIV_CDIV        (0xffff)                    // Clock Divider


#define BSC0_DEL            (BSC0_BASE+0x18)            // Data Delay
//-------------------------------------------------------------------------------------------------------------------
#define BSC0DEL_FEDL        (0xffff<<16)                // Falling Edge Delay
#define BSC0DEL_REDL        (0xffff)                    // Rising Edge Delay

#define SH_BSC0DELFEDL(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define BSC0_CLKT           (BSC0_BASE+0x1c)            // Clock Stretch Timeout
//-------------------------------------------------------------------------------------------------------------------
#define BSC0CLKT_TOUT       (0xffff)                    // Clock Stretch Timeout value


//
// -- The BSC1 controller
//    -------------------
#define BSC1_BASE           (HW_BASE+0x804000)          // The base address of BSC1


#define BSC1_C              (BSC1_BASE)                 // Control
//-------------------------------------------------------------------------------------------------------------------
#define BSC1C_I2CEN         (1<<15)                     // I2C Enable
#define BSC1C_INTR          (1<<10)                     // Interrupt on RXR=1
#define BSC1C_INTT          (1<<9)                      // Interrupt on TXW=1
#define BSC1C_INTD          (1<<8)                      // Interrupt on DONE=1
#define BSC1C_ST            (1<<7)                      // Start new transfer
#define BSC1C_CLEAR         (3<<4)                      // Clear FIFO if either bit is set
#define BSC1C_READ          (1<<0)                      // Read Packet transfer

#define SH_BSC1C_CLEAR(x)   (((x)&3)<<4)                // shift bits to the right location


#define BSC1_S              (BSC1_BASE+4)               // Status
//-------------------------------------------------------------------------------------------------------------------
#define BSC1S_CLKT          (1<<9)                      // 1=Slave has held SCL too long;  Write 1 to clear
#define BSC1S_ERR           (1<<8)                      // 1=Slave has not ACK address;  Write 1 to clear
#define BSC1S_RXF           (1<<7)                      // 1=FIFO full
#define BSC1S_TXE           (1<<6)                      // 1=FIFO empty
#define BSC1S_RXD           (1<<5)                      // 1=FIFO has data
#define BSC1S_TXD           (1<<4)                      // 1=FIFO has room
#define BSC1S_RXR           (1<<3)                      // 1=FIFO full; read underway
#define BSC1S_TXW           (1<<2)                      // 1=FIFO fill; write underway
#define BSC1S_DONE          (1<<1)                      // 1=Transfer complete
#define BSC1S_TA            (1<<0)                      // 1=Transfer active


#define BSC1_DLEN           (BSC1_BASE+8)               // Data Length
//-------------------------------------------------------------------------------------------------------------------
#define BSC1DLEN_DLEN       (0xffff)                    // Data Length


#define BSC1_A              (BSC1_BASE+0xc)             // Slave Address
//-------------------------------------------------------------------------------------------------------------------
#define BSC1A_ADDR          (0xffff)                    // Slave Address


#define BSC1_FIFO           (BSC1_BASE+0x10)            // Data FIFO
//-------------------------------------------------------------------------------------------------------------------
#define BSC1FIFO_DATA       (0xffff)                    // Data


#define BSC1_DIV            (BSC1_BASE+0x14)            // Clock Divider
//-------------------------------------------------------------------------------------------------------------------
#define BSC1DIV_CDIV        (0xffff)                    // Clock Divider


#define BSC1_DEL            (BSC1_BASE+0x18)            // Data Delay
//-------------------------------------------------------------------------------------------------------------------
#define BSC1DEL_FEDL        (0xffff<<16)                // Falling Edge Delay
#define BSC1DEL_REDL        (0xffff)                    // Rising Edge Delay

#define SH_BSC1DELFEDL(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define BSC1_CLKT           (BSC1_BASE+0x1c)            // Clock Stretch Timeout
//-------------------------------------------------------------------------------------------------------------------
#define BSC1CLKT_TOUT       (0xffff)                    // Clock Stretch Timeout value


//-------------------------------------------------------------------------------------------------------------------
// DMA Controllers DMA0-DMA15
//-------------------------------------------------------------------------------------------------------------------

//
// -- The DMA0 controller
//    -------------------
#define DMA0_BASE           (HW_BASE+0x7000)            // Base Address of DMA0


#define DMA0_CS             (DMA0_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA0CS_RESET        (1<<31)                     // reset DMA
#define DMA0CS_ABORT        (1<<30)                     // abort current CB
#define DMA0CS_DISDBG       (1<<29)                     // disable debug
#define DMA0CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA0CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA0CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA0CS_ERROR        (1<<8)                      // Channel has error
#define DMA0CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA0CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA0CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA0CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA0CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA0CS_END          (1<<1)                      // 1 when complete
#define DMA0CS_ACTIVE       (1<<0)                      // active

#define SH_DMA0CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA0CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA0_CBAD           (DMA0_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA0CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA0_TI             (DMA0_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA0TI_NOWIDE       (1<<26)                     // Do not do write as 2 beat bursts
#define DMA0TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA0TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA0TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA0TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA0TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA0TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA0TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA0TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA0TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA0TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA0TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA0TI_WRESP        (1<<3)                      // Wait for Response
#define DMA0TI_TMODE        (1<<1)                      // 2D Mode
#define DMA0TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA0TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA0TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA0_SRCAD          (DMA0_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA0SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA0_DSTAD          (DMA0_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA0DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA0_XLEN           (DMA0_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA0XLEN_Y          (0x3fff<<16)                // in 2D mode, number of DMA0XLEN_X
#define DMA0XLEN_X          (0xffff)                    // Transfer bytes length

#define SH_DMA0XLENY(x)     (((x)&0x3fff)<<16)          // shift the value to the correct position


#define DMA0_STRIDE         (DMA0_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA0STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA0STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA0STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA0_NXTCB          (DMA0_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA0NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA0_DEBUG          (DMA0_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA0DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA0DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA0DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA0DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA0DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA0DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA0DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA0DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA1 controller
//    -------------------
#define DMA1_BASE           (DMA1_BASE+0x100)           // Base Address of DMA1
//-------------------------------------------------------------------------------------------------------------------


#define DMA1_CS             (DMA1_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA1CS_RESET        (1<<31)                     // reset DMA
#define DMA1CS_ABORT        (1<<30)                     // abort current CB
#define DMA1CS_DISDBG       (1<<29)                     // disable debug
#define DMA1CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA1CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA1CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA1CS_ERROR        (1<<8)                      // Channel has error
#define DMA1CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA1CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA1CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA1CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA1CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA1CS_END          (1<<1)                      // 1 when complete
#define DMA1CS_ACTIVE       (1<<0)                      // active

#define SH_DMA1CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA1CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA1_CBAD           (DMA1_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA1CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA1_TI             (DMA1_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA1TI_NOWIDE       (1<<26)                     // Do not do write as 2 beat bursts
#define DMA1TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA1TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA1TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA1TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA1TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA1TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA1TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA1TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA1TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA1TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA1TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA1TI_WRESP        (1<<3)                      // Wait for Response
#define DMA1TI_TMODE        (1<<1)                      // 2D Mode
#define DMA1TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA1TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA1TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA1_SRCAD          (DMA1_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA1SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA1_DSTAD          (DMA1_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA1DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA1_XLEN           (DMA1_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA1XLEN_Y          (0x3fff<<16)                // in 2D mode, number of DMA0XLEN_X
#define DMA1XLEN_X          (0xffff)                    // Transfer bytes length

#define SH_DMA1XLENY(x)     (((x)&0x3fff)<<16)          // shift the value to the correct position


#define DMA1_STRIDE         (DMA1_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA1STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA1STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA1STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA1_NXTCB          (DMA1_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA1NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA1_DEBUG          (DMA1_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA1DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA1DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA1DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA1DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA1DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA1DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA1DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA1DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA2 controller
//    -------------------
#define DMA2_BASE           (DMA1_BASE+0x100)           // Base Address of DMA2
//-------------------------------------------------------------------------------------------------------------------


#define DMA2_CS             (DMA2_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA2CS_RESET        (1<<31)                     // reset DMA
#define DMA2CS_ABORT        (1<<30)                     // abort current CB
#define DMA2CS_DISDBG       (1<<29)                     // disable debug
#define DMA2CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA2CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA2CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA2CS_ERROR        (1<<8)                      // Channel has error
#define DMA2CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA2CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA2CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA2CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA2CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA2CS_END          (1<<1)                      // 1 when complete
#define DMA2CS_ACTIVE       (1<<0)                      // active

#define SH_DMA2CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA2CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA2_CBAD           (DMA2_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA2CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA2_TI             (DMA2_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA2TI_NOWIDE       (1<<26)                     // Do not do write as 2 beat bursts
#define DMA2TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA2TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA2TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA2TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA2TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA2TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA2TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA2TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA2TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA2TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA2TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA2TI_WRESP        (1<<3)                      // Wait for Response
#define DMA2TI_TMODE        (1<<1)                      // 2D Mode
#define DMA2TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA2TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA2TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA2_SRCAD          (DMA2_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA2SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA2_DSTAD          (DMA2_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA2DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA2_XLEN           (DMA2_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA2XLEN_Y          (0x3fff<<16)                // in 2D mode, number of DMA0XLEN_X
#define DMA2XLEN_X          (0xffff)                    // Transfer bytes length

#define SH_DMA2XLENY(x)     (((x)&0x3fff)<<16)          // shift the value to the correct position


#define DMA2_STRIDE         (DMA2_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA2STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA2STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA2STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA2_NXTCB          (DMA2_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA2NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA2_DEBUG          (DMA2_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA2DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA2DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA2DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA2DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA2DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA2DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA2DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA2DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA3 controller
//    -------------------
#define DMA3_BASE           (DMA2_BASE+0x100)           // Base Address of DMA4
//-------------------------------------------------------------------------------------------------------------------


#define DMA3_CS             (DMA3_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA3CS_RESET        (1<<31)                     // reset DMA
#define DMA3CS_ABORT        (1<<30)                     // abort current CB
#define DMA3CS_DISDBG       (1<<29)                     // disable debug
#define DMA3CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA3CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA3CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA3CS_ERROR        (1<<8)                      // Channel has error
#define DMA3CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA3CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA3CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA3CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA3CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA3CS_END          (1<<1)                      // 1 when complete
#define DMA3CS_ACTIVE       (1<<0)                      // active

#define SH_DMA3CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA3CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA3_CBAD           (DMA3_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA3CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA3_TI             (DMA3_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA3TI_NOWIDE       (1<<26)                     // Do not do write as 2 beat bursts
#define DMA3TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA3TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA3TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA3TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA3TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA3TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA3TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA3TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA3TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA3TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA3TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA3TI_WRESP        (1<<3)                      // Wait for Response
#define DMA3TI_TMODE        (1<<1)                      // 2D Mode
#define DMA3TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA3TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA3TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA3_SRCAD          (DMA3_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA3SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA3_DSTAD          (DMA3_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA3DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA3_XLEN           (DMA3_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA3XLEN_Y          (0x3fff<<16)                // in 2D mode, number of DMA0XLEN_X
#define DMA3XLEN_X          (0xffff)                    // Transfer bytes length

#define SH_DMA3XLENY(x)     (((x)&0x3fff)<<16)          // shift the value to the correct position


#define DMA3_STRIDE         (DMA3_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA3STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA3STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA3STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA3_NXTCB          (DMA3_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA3NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA3_DEBUG          (DMA3_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA3DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA3DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA3DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA3DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA3DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA3DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA3DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA3DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA4 controller
//    -------------------
#define DMA4_BASE           (DMA3_BASE+0x100)           // Base Address of DMA4
//-------------------------------------------------------------------------------------------------------------------


#define DMA4_CS             (DMA4_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA4CS_RESET        (1<<31)                     // reset DMA
#define DMA4CS_ABORT        (1<<30)                     // abort current CB
#define DMA4CS_DISDBG       (1<<29)                     // disable debug
#define DMA4CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA4CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA4CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA4CS_ERROR        (1<<8)                      // Channel has error
#define DMA4CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA4CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA4CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA4CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA4CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA4CS_END          (1<<1)                      // 1 when complete
#define DMA4CS_ACTIVE       (1<<0)                      // active

#define SH_DMA4CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA4CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA4_CBAD           (DMA4_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA4CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA4_TI             (DMA4_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA4TI_NOWIDE       (1<<26)                     // Do not do write as 2 beat bursts
#define DMA4TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA4TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA4TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA4TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA4TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA4TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA4TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA4TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA4TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA4TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA4TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA4TI_WRESP        (1<<3)                      // Wait for Response
#define DMA4TI_TMODE        (1<<1)                      // 2D Mode
#define DMA4TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA4TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA4TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA4_SRCAD          (DMA4_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA4SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA4_DSTAD          (DMA4_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA4DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA4_XLEN           (DMA4_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA4XLEN_Y          (0x3fff<<16)                // in 2D mode, number of DMA0XLEN_X
#define DMA4XLEN_X          (0xffff)                    // Transfer bytes length

#define SH_DMA4XLENY(x)     (((x)&0x3fff)<<16)          // shift the value to the correct position


#define DMA4_STRIDE         (DMA4_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA4STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA4STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA4STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA4_NXTCB          (DMA4_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA4NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA4_DEBUG          (DMA4_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA4DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA4DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA4DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA4DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA4DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA4DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA4DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA4DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA5 controller
//    -------------------
#define DMA5_BASE           (DMA4_BASE+0x100)           // Base Address of DMA5
//-------------------------------------------------------------------------------------------------------------------


#define DMA5_CS             (DMA5_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA5CS_RESET        (1<<31)                     // reset DMA
#define DMA5CS_ABORT        (1<<30)                     // abort current CB
#define DMA5CS_DISDBG       (1<<29)                     // disable debug
#define DMA5CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA5CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA5CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA5CS_ERROR        (1<<8)                      // Channel has error
#define DMA5CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA5CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA5CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA5CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA5CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA5CS_END          (1<<1)                      // 1 when complete
#define DMA5CS_ACTIVE       (1<<0)                      // active

#define SH_DMA5CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA5CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA5_CBAD           (DMA5_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA5CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA5_TI             (DMA5_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA5TI_NOWIDE       (1<<26)                     // Do not do write as 2 beat bursts
#define DMA5TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA5TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA5TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA5TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA5TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA5TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA5TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA5TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA5TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA5TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA5TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA5TI_WRESP        (1<<3)                      // Wait for Response
#define DMA5TI_TMODE        (1<<1)                      // 2D Mode
#define DMA5TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA5TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA5TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA5_SRCAD          (DMA5_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA5SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA5_DSTAD          (DMA5_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA5DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA5_XLEN           (DMA5_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA5XLEN_Y          (0x3fff<<16)                // in 2D mode, number of DMA0XLEN_X
#define DMA5XLEN_X          (0xffff)                    // Transfer bytes length

#define SH_DMA5XLENY(x)     (((x)&0x3fff)<<16)          // shift the value to the correct position


#define DMA5_STRIDE         (DMA5_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA5STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA5STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA5STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA5_NXTCB          (DMA5_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA5NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA5_DEBUG          (DMA5_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA5DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA5DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA5DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA5DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA5DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA5DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA5DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA5DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA6 controller
//    -------------------
#define DMA6_BASE           (DMA5_BASE+0x100)           // Base Address of DMA6
//-------------------------------------------------------------------------------------------------------------------


#define DMA6_CS             (DMA6_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA6CS_RESET        (1<<31)                     // reset DMA
#define DMA6CS_ABORT        (1<<30)                     // abort current CB
#define DMA6CS_DISDBG       (1<<29)                     // disable debug
#define DMA6CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA6CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA6CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA6CS_ERROR        (1<<8)                      // Channel has error
#define DMA6CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA6CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA6CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA6CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA6CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA6CS_END          (1<<1)                      // 1 when complete
#define DMA6CS_ACTIVE       (1<<0)                      // active

#define SH_DMA6CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA6CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA6_CBAD           (DMA6_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA6CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA6_TI             (DMA6_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA6TI_NOWIDE       (1<<26)                     // Do not do write as 2 beat bursts
#define DMA6TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA6TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA6TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA6TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA6TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA6TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA6TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA6TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA6TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA6TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA6TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA6TI_WRESP        (1<<3)                      // Wait for Response
#define DMA6TI_TMODE        (1<<1)                      // 2D Mode
#define DMA6TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA6TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA6TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA6_SRCAD          (DMA6_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA6SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA6_DSTAD          (DMA6_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA6DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA6_XLEN           (DMA6_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA6XLEN_Y          (0x3fff<<16)                // in 2D mode, number of DMA0XLEN_X
#define DMA6XLEN_X          (0xffff)                    // Transfer bytes length

#define SH_DMA6XLENY(x)     (((x)&0x3fff)<<16)          // shift the value to the correct position


#define DMA6_STRIDE         (DMA6_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA6STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA6STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA6STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA6_NXTCB          (DMA6_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA6NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA6_DEBUG          (DMA6_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA6DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA6DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA6DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA6DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA6DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA6DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA6DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA6DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA7 controller
//    -------------------
#define DMA7_BASE           (DMA6_BASE+0x100)           // Base Address of DMA7
//-------------------------------------------------------------------------------------------------------------------


#define DMA7_CS             (DMA7_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA7CS_RESET        (1<<31)                     // reset DMA
#define DMA7CS_ABORT        (1<<30)                     // abort current CB
#define DMA7CS_DISDBG       (1<<29)                     // disable debug
#define DMA7CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA7CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA7CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA7CS_ERROR        (1<<8)                      // Channel has error
#define DMA7CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA7CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA7CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA7CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA7CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA7CS_END          (1<<1)                      // 1 when complete
#define DMA7CS_ACTIVE       (1<<0)                      // active

#define SH_DMA7CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA7CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA7_CBAD           (DMA7_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA7CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA7_TI             (DMA7_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA7TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA7TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA7TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA7TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA7TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA7TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA7TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA7TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA7TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA7TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA7TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA7TI_WRESP        (1<<3)                      // Wait for Response
#define DMA7TI_TMODE        (1<<1)                      // 2D Mode
#define DMA7TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA7TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA7TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA7_SRCAD          (DMA7_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA7SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA7_DSTAD          (DMA7_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA7DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA7_XLEN           (DMA7_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA7XLEN_X          (0xffff)                    // Transfer bytes length


#define DMA7_STRIDE         (DMA7_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA7STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA7STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA7STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA7_NXTCB          (DMA7_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA7NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA7_DEBUG          (DMA7_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA7DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA7DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA7DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA7DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA7DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA7DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA7DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA7DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA8 controller
//    -------------------
#define DMA8_BASE           (DMA7_BASE+0x100)           // Base Address of DMA8
//-------------------------------------------------------------------------------------------------------------------


#define DMA8_CS             (DMA8_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA8CS_RESET        (1<<31)                     // reset DMA
#define DMA8CS_ABORT        (1<<30)                     // abort current CB
#define DMA8CS_DISDBG       (1<<29)                     // disable debug
#define DMA8CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA8CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA8CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA8CS_ERROR        (1<<8)                      // Channel has error
#define DMA8CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA8CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA8CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA8CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA8CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA8CS_END          (1<<1)                      // 1 when complete
#define DMA8CS_ACTIVE       (1<<0)                      // active

#define SH_DMA8CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA8CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA8_CBAD           (DMA8_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA8CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA8_TI             (DMA8_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA8TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA8TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA8TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA8TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA8TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA8TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA8TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA8TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA8TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA8TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA8TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA8TI_WRESP        (1<<3)                      // Wait for Response
#define DMA8TI_TMODE        (1<<1)                      // 2D Mode
#define DMA8TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA8TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA8TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA8_SRCAD          (DMA8_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA8SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA8_DSTAD          (DMA8_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA8DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA8_XLEN           (DMA8_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA8XLEN_X          (0xffff)                    // Transfer bytes length


#define DMA8_STRIDE         (DMA8_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA8STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA8STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA8STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA8_NXTCB          (DMA8_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA8NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA8_DEBUG          (DMA8_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA8DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA8DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA8DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA8DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA8DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA8DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA8DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA8DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA9 controller
//    -------------------
#define DMA9_BASE           (DMA8_BASE+0x100)           // Base Address of DMA9
//-------------------------------------------------------------------------------------------------------------------


#define DMA9_CS             (DMA9_BASE)                 // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA9CS_RESET        (1<<31)                     // reset DMA
#define DMA9CS_ABORT        (1<<30)                     // abort current CB
#define DMA9CS_DISDBG       (1<<29)                     // disable debug
#define DMA9CS_WAIT         (1<<28)                     // wait for outstanding writes
#define DMA9CS_PANICPTY     (0xf<<20)                   // priority of panicking transactions
#define DMA9CS_PTY          (0xf<<16)                   // priority of transactions
#define DMA9CS_ERROR        (1<<8)                      // Channel has error
#define DMA9CS_WAITING      (1<<6)                      // is waiting for outstanding writes
#define DMA9CS_REQSTOP      (1<<5)                      // DREQ stops DMA
#define DMA9CS_PAUSE        (1<<4)                      // DMA is paused
#define DMA9CS_DREQ         (1<<3)                      // 1=requesting data
#define DMA9CS_INT          (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA9CS_END          (1<<1)                      // 1 when complete
#define DMA9CS_ACTIVE       (1<<0)                      // active

#define SH_DMA9CSPANIC(x)   (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA9CSPTY(x)     (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA9_CBAD           (DMA9_BASE+4)               // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA9CBAD_SCB        (0xffffffff)                // Source Control Block address


#define DMA9_TI             (DMA9_BASE+8)               // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA9TI_WAITS        (0x1f<<21)                  // Add wait cycles
#define DMA9TI_PMAP         (0x1f<<16)                  // peripheral map
#define DMA9TI_BLEN         (0xf<<12)                   // burst transfer length
#define DMA9TI_SRCIGN       (1<<11)                     // Ignore Reads
#define DMA9TI_SRCDREQ      (1<<10)                     // Control source reads with DREQ
#define DMA9TI_SRCWID       (1<<9)                      // Source Transfer Width
#define DMA9TI_SRCINC       (1<<8)                      // Source Address Increment
#define DMA9TI_DSTIGN       (1<<7)                      // Ignore Writes
#define DMA9TI_DSTDREQ      (1<<6)                      // Control source writes with DREQ
#define DMA9TI_DSTWID       (1<<5)                      // Dest Transfer Width
#define DMA9TI_DSTINC       (1<<4)                      // Dest Address Increment
#define DMA9TI_WRESP        (1<<3)                      // Wait for Response
#define DMA9TI_TMODE        (1<<1)                      // 2D Mode
#define DMA9TI_INTEN        (1<<0)                      // Interrupt Enable

#define SH_DMA9TIWAITS(x)   (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA9TIPMAP(x)    (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA9_SRCAD          (DMA9_BASE+0xc)             // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA9SRCAD_SRC       (0xffffffff)                // DMA Source address


#define DMA9_DSTAD          (DMA9_BASE+0x10)            // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA9DSTAD_SRC       (0xffffffff)                // DMA Dest address


#define DMA9_XLEN           (DMA9_BASE+0x14)            // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA9XLEN_X          (0xffff)                    // Transfer bytes length


#define DMA9_STRIDE         (DMA9_BASE+0x18)            // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA9STRIDE_D        (0xffff<<16)                // Destination Stride
#define DMA9STRIDE_S        (0xffff)                    // Source Stride

#define SH_DMA9STRIDED(x)   (((x)&0xffff)<<16)          // shift to the correct position


#define DMA9_NXTCB          (DMA9_BASE+0x1c)            // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA9NXTCB_ADD       (0xffffffff)                // NExt Control Block Address


#define DMA9_DEBUG          (DMA9_BASE+0x20)            // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA9DEBUG_LITE      (1<<28)                     // DMA Lite
#define DMA9DEBUG_VERS      (7<<25)                     // DMA Version
#define DMA9DEBUG_STATE     (0x1ff<<16)                 // DMA State
#define DMA9DEBUG_ID        (0xff<<8)                   // DMA ID
#define DMA9DEBUG_OWRITE    (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA9DEBUG_RERROR    (1<<2)                      // Slave read response error
#define DMA9DEBUG_FERROR    (1<<1)                      // FIFO error
#define DMA9DEBUG_LAST      (1<<0)                      // Read Last Not Set Error


//
// -- The DMA10 controller
//    --------------------
#define DMA10_BASE          (DMA9_BASE+0x100)           // Base Address of DMA10
//-------------------------------------------------------------------------------------------------------------------


#define DMA10_CS            (DMA10_BASE)                // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA10CS_RESET       (1<<31)                     // reset DMA
#define DMA10CS_ABORT       (1<<30)                     // abort current CB
#define DMA10CS_DISDBG      (1<<29)                     // disable debug
#define DMA10CS_WAIT        (1<<28)                     // wait for outstanding writes
#define DMA10CS_PANICPTY    (0xf<<20)                   // priority of panicking transactions
#define DMA10CS_PTY         (0xf<<16)                   // priority of transactions
#define DMA10CS_ERROR       (1<<8)                      // Channel has error
#define DMA10CS_WAITING     (1<<6)                      // is waiting for outstanding writes
#define DMA10CS_REQSTOP     (1<<5)                      // DREQ stops DMA
#define DMA10CS_PAUSE       (1<<4)                      // DMA is paused
#define DMA10CS_DREQ        (1<<3)                      // 1=requesting data
#define DMA10CS_INT         (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA10CS_END         (1<<1)                      // 1 when complete
#define DMA10CS_ACTIVE      (1<<0)                      // active

#define SH_DMA10CSPANIC(x)  (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA10CSPTY(x)    (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA10_CBAD          (DMA10_BASE+4)              // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA10CBAD_SCB       (0xffffffff)                // Source Control Block address


#define DMA10_TI            (DMA10_BASE+8)              // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA10TI_WAITS       (0x1f<<21)                  // Add wait cycles
#define DMA10TI_PMAP        (0x1f<<16)                  // peripheral map
#define DMA10TI_BLEN        (0xf<<12)                   // burst transfer length
#define DMA10TI_SRCIGN      (1<<11)                     // Ignore Reads
#define DMA10TI_SRCDREQ     (1<<10)                     // Control source reads with DREQ
#define DMA10TI_SRCWID      (1<<9)                      // Source Transfer Width
#define DMA10TI_SRCINC      (1<<8)                      // Source Address Increment
#define DMA10TI_DSTIGN      (1<<7)                      // Ignore Writes
#define DMA10TI_DSTDREQ     (1<<6)                      // Control source writes with DREQ
#define DMA10TI_DSTWID      (1<<5)                      // Dest Transfer Width
#define DMA10TI_DSTINC      (1<<4)                      // Dest Address Increment
#define DMA10TI_WRESP       (1<<3)                      // Wait for Response
#define DMA10TI_TMODE       (1<<1)                      // 2D Mode
#define DMA10TI_INTEN       (1<<0)                      // Interrupt Enable

#define SH_DMA10TIWAITS(x)  (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA10TIPMAP(x)   (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA10_SRCAD         (DMA10_BASE+0xc)            // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA10SRCAD_SRC      (0xffffffff)                // DMA Source address


#define DMA10_DSTAD         (DMA10_BASE+0x10)           // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA10DSTAD_SRC      (0xffffffff)                // DMA Dest address


#define DMA10_XLEN          (DMA10_BASE+0x14)           // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA10XLEN_X         (0xffff)                    // Transfer bytes length


#define DMA10_STRIDE        (DMA10_BASE+0x18)           // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA10STRIDE_D       (0xffff<<16)                // Destination Stride
#define DMA10STRIDE_S       (0xffff)                    // Source Stride

#define SH_DMA10STRIDED(x)  (((x)&0xffff)<<16)          // shift to the correct position


#define DMA10_NXTCB         (DMA10_BASE+0x1c)           // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA10NXTCB_ADD      (0xffffffff)                // NExt Control Block Address


#define DMA10_DEBUG         (DMA10_BASE+0x20)           // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA10DEBUG_LITE     (1<<28)                     // DMA Lite
#define DMA10DEBUG_VERS     (7<<25)                     // DMA Version
#define DMA10DEBUG_STATE    (0x1ff<<16)                 // DMA State
#define DMA10DEBUG_ID       (0xff<<8)                   // DMA ID
#define DMA10DEBUG_OWRITE   (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA10DEBUG_RERROR   (1<<2)                      // Slave read response error
#define DMA10DEBUG_FERROR   (1<<1)                      // FIFO error
#define DMA10DEBUG_LAST     (1<<0)                      // Read Last Not Set Error


//
// -- The DMA11 controller
//    --------------------
#define DMA11_BASE          (DMA10_BASE+0x100)          // Base Address of DMA11
//-------------------------------------------------------------------------------------------------------------------


#define DMA11_CS            (DMA11_BASE)                // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA11CS_RESET       (1<<31)                     // reset DMA
#define DMA11CS_ABORT       (1<<30)                     // abort current CB
#define DMA11CS_DISDBG      (1<<29)                     // disable debug
#define DMA11CS_WAIT        (1<<28)                     // wait for outstanding writes
#define DMA11CS_PANICPTY    (0xf<<20)                   // priority of panicking transactions
#define DMA11CS_PTY         (0xf<<16)                   // priority of transactions
#define DMA11CS_ERROR       (1<<8)                      // Channel has error
#define DMA11CS_WAITING     (1<<6)                      // is waiting for outstanding writes
#define DMA11CS_REQSTOP     (1<<5)                      // DREQ stops DMA
#define DMA11CS_PAUSE       (1<<4)                      // DMA is paused
#define DMA11CS_DREQ        (1<<3)                      // 1=requesting data
#define DMA11CS_INT         (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA11CS_END         (1<<1)                      // 1 when complete
#define DMA11CS_ACTIVE      (1<<0)                      // active

#define SH_DMA11CSPANIC(x)  (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA11CSPTY(x)    (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA11_CBAD          (DMA11_BASE+4)              // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA11CBAD_SCB       (0xffffffff)                // Source Control Block address


#define DMA11_TI            (DMA11_BASE+8)              // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA11TI_WAITS       (0x1f<<21)                  // Add wait cycles
#define DMA11TI_PMAP        (0x1f<<16)                  // peripheral map
#define DMA11TI_BLEN        (0xf<<12)                   // burst transfer length
#define DMA11TI_SRCIGN      (1<<11)                     // Ignore Reads
#define DMA11TI_SRCDREQ     (1<<10)                     // Control source reads with DREQ
#define DMA11TI_SRCWID      (1<<9)                      // Source Transfer Width
#define DMA11TI_SRCINC      (1<<8)                      // Source Address Increment
#define DMA11TI_DSTIGN      (1<<7)                      // Ignore Writes
#define DMA11TI_DSTDREQ     (1<<6)                      // Control source writes with DREQ
#define DMA11TI_DSTWID      (1<<5)                      // Dest Transfer Width
#define DMA11TI_DSTINC      (1<<4)                      // Dest Address Increment
#define DMA11TI_WRESP       (1<<3)                      // Wait for Response
#define DMA11TI_TMODE       (1<<1)                      // 2D Mode
#define DMA11TI_INTEN       (1<<0)                      // Interrupt Enable

#define SH_DMA11TIWAITS(x)  (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA11TIPMAP(x)   (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA11_SRCAD         (DMA11_BASE+0xc)            // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA11SRCAD_SRC      (0xffffffff)                // DMA Source address


#define DMA11_DSTAD         (DMA11_BASE+0x10)           // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA11DSTAD_SRC      (0xffffffff)                // DMA Dest address


#define DMA11_XLEN          (DMA11_BASE+0x14)           // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA11XLEN_X         (0xffff)                    // Transfer bytes length


#define DMA11_STRIDE        (DMA11_BASE+0x18)           // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA11STRIDE_D       (0xffff<<16)                // Destination Stride
#define DMA11STRIDE_S       (0xffff)                    // Source Stride

#define SH_DMA11STRIDED(x)  (((x)&0xffff)<<16)          // shift to the correct position


#define DMA11_NXTCB         (DMA11_BASE+0x1c)           // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA11NXTCB_ADD      (0xffffffff)                // NExt Control Block Address


#define DMA11_DEBUG         (DMA11_BASE+0x20)           // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA11DEBUG_LITE     (1<<28)                     // DMA Lite
#define DMA11DEBUG_VERS     (7<<25)                     // DMA Version
#define DMA11DEBUG_STATE    (0x1ff<<16)                 // DMA State
#define DMA11DEBUG_ID       (0xff<<8)                   // DMA ID
#define DMA11DEBUG_OWRITE   (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA11DEBUG_RERROR   (1<<2)                      // Slave read response error
#define DMA11DEBUG_FERROR   (1<<1)                      // FIFO error
#define DMA11DEBUG_LAST     (1<<0)                      // Read Last Not Set Error


//
// -- The DMA12 controller
//    --------------------
#define DMA12_BASE          (DMA11_BASE+0x100)          // Base Address of DMA12
//-------------------------------------------------------------------------------------------------------------------


#define DMA12_CS            (DMA12_BASE)                // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA12CS_RESET       (1<<31)                     // reset DMA
#define DMA12CS_ABORT       (1<<30)                     // abort current CB
#define DMA12CS_DISDBG      (1<<29)                     // disable debug
#define DMA12CS_WAIT        (1<<28)                     // wait for outstanding writes
#define DMA12CS_PANICPTY    (0xf<<20)                   // priority of panicking transactions
#define DMA12CS_PTY         (0xf<<16)                   // priority of transactions
#define DMA12CS_ERROR       (1<<8)                      // Channel has error
#define DMA12CS_WAITING     (1<<6)                      // is waiting for outstanding writes
#define DMA12CS_REQSTOP     (1<<5)                      // DREQ stops DMA
#define DMA12CS_PAUSE       (1<<4)                      // DMA is paused
#define DMA12CS_DREQ        (1<<3)                      // 1=requesting data
#define DMA12CS_INT         (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA12CS_END         (1<<1)                      // 1 when complete
#define DMA12CS_ACTIVE      (1<<0)                      // active

#define SH_DMA12CSPANIC(x)  (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA12CSPTY(x)    (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA12_CBAD          (DMA12_BASE+4)              // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA12CBAD_SCB       (0xffffffff)                // Source Control Block address


#define DMA12_TI            (DMA12_BASE+8)              // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA12TI_WAITS       (0x1f<<21)                  // Add wait cycles
#define DMA12TI_PMAP        (0x1f<<16)                  // peripheral map
#define DMA12TI_BLEN        (0xf<<12)                   // burst transfer length
#define DMA12TI_SRCIGN      (1<<11)                     // Ignore Reads
#define DMA12TI_SRCDREQ     (1<<10)                     // Control source reads with DREQ
#define DMA12TI_SRCWID      (1<<9)                      // Source Transfer Width
#define DMA12TI_SRCINC      (1<<8)                      // Source Address Increment
#define DMA12TI_DSTIGN      (1<<7)                      // Ignore Writes
#define DMA12TI_DSTDREQ     (1<<6)                      // Control source writes with DREQ
#define DMA12TI_DSTWID      (1<<5)                      // Dest Transfer Width
#define DMA12TI_DSTINC      (1<<4)                      // Dest Address Increment
#define DMA12TI_WRESP       (1<<3)                      // Wait for Response
#define DMA12TI_TMODE       (1<<1)                      // 2D Mode
#define DMA12TI_INTEN       (1<<0)                      // Interrupt Enable

#define SH_DMA12TIWAITS(x)  (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA12TIPMAP(x)   (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA12_SRCAD         (DMA12_BASE+0xc)            // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA12SRCAD_SRC      (0xffffffff)                // DMA Source address


#define DMA12_DSTAD         (DMA12_BASE+0x10)           // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA12DSTAD_SRC      (0xffffffff)                // DMA Dest address


#define DMA12_XLEN          (DMA12_BASE+0x14)           // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA12XLEN_X         (0xffff)                    // Transfer bytes length


#define DMA12_STRIDE        (DMA12_BASE+0x18)           // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA12STRIDE_D       (0xffff<<16)                // Destination Stride
#define DMA12STRIDE_S       (0xffff)                    // Source Stride

#define SH_DMA12STRIDED(x)  (((x)&0xffff)<<16)          // shift to the correct position


#define DMA12_NXTCB         (DMA12_BASE+0x1c)           // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA12NXTCB_ADD      (0xffffffff)                // NExt Control Block Address


#define DMA12_DEBUG         (DMA12_BASE+0x20)           // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA12DEBUG_LITE     (1<<28)                     // DMA Lite
#define DMA12DEBUG_VERS     (7<<25)                     // DMA Version
#define DMA12DEBUG_STATE    (0x1ff<<16)                 // DMA State
#define DMA12DEBUG_ID       (0xff<<8)                   // DMA ID
#define DMA12DEBUG_OWRITE   (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA12DEBUG_RERROR   (1<<2)                      // Slave read response error
#define DMA12DEBUG_FERROR   (1<<1)                      // FIFO error
#define DMA12DEBUG_LAST     (1<<0)                      // Read Last Not Set Error


//
// -- The DMA13 controller
//    --------------------
#define DMA13_BASE          (DMA12_BASE+0x100)          // Base Address of DMA13
//-------------------------------------------------------------------------------------------------------------------


#define DMA13_CS            (DMA13_BASE)                // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA13CS_RESET       (1<<31)                     // reset DMA
#define DMA13CS_ABORT       (1<<30)                     // abort current CB
#define DMA13CS_DISDBG      (1<<29)                     // disable debug
#define DMA13CS_WAIT        (1<<28)                     // wait for outstanding writes
#define DMA13CS_PANICPTY    (0xf<<20)                   // priority of panicking transactions
#define DMA13CS_PTY         (0xf<<16)                   // priority of transactions
#define DMA13CS_ERROR       (1<<8)                      // Channel has error
#define DMA13CS_WAITING     (1<<6)                      // is waiting for outstanding writes
#define DMA13CS_REQSTOP     (1<<5)                      // DREQ stops DMA
#define DMA13CS_PAUSE       (1<<4)                      // DMA is paused
#define DMA13CS_DREQ        (1<<3)                      // 1=requesting data
#define DMA13CS_INT         (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA13CS_END         (1<<1)                      // 1 when complete
#define DMA13CS_ACTIVE      (1<<0)                      // active

#define SH_DMA13CSPANIC(x)  (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA13CSPTY(x)    (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA13_CBAD          (DMA13_BASE+4)              // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA13CBAD_SCB       (0xffffffff)                // Source Control Block address


#define DMA13_TI            (DMA13_BASE+8)              // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA13TI_WAITS       (0x1f<<21)                  // Add wait cycles
#define DMA13TI_PMAP        (0x1f<<16)                  // peripheral map
#define DMA13TI_BLEN        (0xf<<12)                   // burst transfer length
#define DMA13TI_SRCIGN      (1<<11)                     // Ignore Reads
#define DMA13TI_SRCDREQ     (1<<10)                     // Control source reads with DREQ
#define DMA13TI_SRCWID      (1<<9)                      // Source Transfer Width
#define DMA13TI_SRCINC      (1<<8)                      // Source Address Increment
#define DMA13TI_DSTIGN      (1<<7)                      // Ignore Writes
#define DMA13TI_DSTDREQ     (1<<6)                      // Control source writes with DREQ
#define DMA13TI_DSTWID      (1<<5)                      // Dest Transfer Width
#define DMA13TI_DSTINC      (1<<4)                      // Dest Address Increment
#define DMA13TI_WRESP       (1<<3)                      // Wait for Response
#define DMA13TI_TMODE       (1<<1)                      // 2D Mode
#define DMA13TI_INTEN       (1<<0)                      // Interrupt Enable

#define SH_DMA13TIWAITS(x)  (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA13TIPMAP(x)   (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA13_SRCAD         (DMA13_BASE+0xc)            // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA13SRCAD_SRC      (0xffffffff)                // DMA Source address


#define DMA13_DSTAD         (DMA13_BASE+0x10)           // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA13DSTAD_SRC      (0xffffffff)                // DMA Dest address


#define DMA13_XLEN          (DMA13_BASE+0x14)           // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA13XLEN_X         (0xffff)                    // Transfer bytes length


#define DMA13_STRIDE        (DMA13_BASE+0x18)           // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA13STRIDE_D       (0xffff<<16)                // Destination Stride
#define DMA13STRIDE_S       (0xffff)                    // Source Stride

#define SH_DMA13STRIDED(x)  (((x)&0xffff)<<16)          // shift to the correct position


#define DMA13_NXTCB         (DMA13_BASE+0x1c)           // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA13NXTCB_ADD      (0xffffffff)                // NExt Control Block Address


#define DMA13_DEBUG         (DMA13_BASE+0x20)           // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA13DEBUG_LITE     (1<<28)                     // DMA Lite
#define DMA13DEBUG_VERS     (7<<25)                     // DMA Version
#define DMA13DEBUG_STATE    (0x1ff<<16)                 // DMA State
#define DMA13DEBUG_ID       (0xff<<8)                   // DMA ID
#define DMA13DEBUG_OWRITE   (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA13DEBUG_RERROR   (1<<2)                      // Slave read response error
#define DMA13DEBUG_FERROR   (1<<1)                      // FIFO error
#define DMA13DEBUG_LAST     (1<<0)                      // Read Last Not Set Error


//
// -- The DMA14 controller
//    --------------------
#define DMA14_BASE          (DMA13_BASE+0x100)          // Base Address of DMA14
//-------------------------------------------------------------------------------------------------------------------


#define DMA14_CS            (DMA14_BASE)                // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA14CS_RESET       (1<<31)                     // reset DMA
#define DMA14CS_ABORT       (1<<30)                     // abort current CB
#define DMA14CS_DISDBG      (1<<29)                     // disable debug
#define DMA14CS_WAIT        (1<<28)                     // wait for outstanding writes
#define DMA14CS_PANICPTY    (0xf<<20)                   // priority of panicking transactions
#define DMA14CS_PTY         (0xf<<16)                   // priority of transactions
#define DMA14CS_ERROR       (1<<8)                      // Channel has error
#define DMA14CS_WAITING     (1<<6)                      // is waiting for outstanding writes
#define DMA14CS_REQSTOP     (1<<5)                      // DREQ stops DMA
#define DMA14CS_PAUSE       (1<<4)                      // DMA is paused
#define DMA14CS_DREQ        (1<<3)                      // 1=requesting data
#define DMA14CS_INT         (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA14CS_END         (1<<1)                      // 1 when complete
#define DMA14CS_ACTIVE      (1<<0)                      // active

#define SH_DMA14CSPANIC(x)  (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA14CSPTY(x)    (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA14_CBAD          (DMA14_BASE+4)              // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA14CBAD_SCB       (0xffffffff)                // Source Control Block address


#define DMA14_TI            (DMA14_BASE+8)              // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA14TI_WAITS       (0x1f<<21)                  // Add wait cycles
#define DMA14TI_PMAP        (0x1f<<16)                  // peripheral map
#define DMA14TI_BLEN        (0xf<<12)                   // burst transfer length
#define DMA14TI_SRCIGN      (1<<11)                     // Ignore Reads
#define DMA14TI_SRCDREQ     (1<<10)                     // Control source reads with DREQ
#define DMA14TI_SRCWID      (1<<9)                      // Source Transfer Width
#define DMA14TI_SRCINC      (1<<8)                      // Source Address Increment
#define DMA14TI_DSTIGN      (1<<7)                      // Ignore Writes
#define DMA14TI_DSTDREQ     (1<<6)                      // Control source writes with DREQ
#define DMA14TI_DSTWID      (1<<5)                      // Dest Transfer Width
#define DMA14TI_DSTINC      (1<<4)                      // Dest Address Increment
#define DMA14TI_WRESP       (1<<3)                      // Wait for Response
#define DMA14TI_TMODE       (1<<1)                      // 2D Mode
#define DMA14TI_INTEN       (1<<0)                      // Interrupt Enable

#define SH_DMA14TIWAITS(x)  (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA14TIPMAP(x)   (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA14_SRCAD         (DMA14_BASE+0xc)            // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA14SRCAD_SRC      (0xffffffff)                // DMA Source address


#define DMA14_DSTAD         (DMA14_BASE+0x10)           // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA14DSTAD_SRC      (0xffffffff)                // DMA Dest address


#define DMA14_XLEN          (DMA14_BASE+0x14)           // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA14XLEN_X         (0xffff)                    // Transfer bytes length


#define DMA14_STRIDE        (DMA14_BASE+0x18)           // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA14STRIDE_D       (0xffff<<16)                // Destination Stride
#define DMA14STRIDE_S       (0xffff)                    // Source Stride

#define SH_DMA14STRIDED(x)  (((x)&0xffff)<<16)          // shift to the correct position


#define DMA14_NXTCB         (DMA14_BASE+0x1c)           // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA14NXTCB_ADD      (0xffffffff)                // NExt Control Block Address


#define DMA14_DEBUG         (DMA14_BASE+0x20)           // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA14DEBUG_LITE     (1<<28)                     // DMA Lite
#define DMA14DEBUG_VERS     (7<<25)                     // DMA Version
#define DMA14DEBUG_STATE    (0x1ff<<16)                 // DMA State
#define DMA14DEBUG_ID       (0xff<<8)                   // DMA ID
#define DMA14DEBUG_OWRITE   (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA14DEBUG_RERROR   (1<<2)                      // Slave read response error
#define DMA14DEBUG_FERROR   (1<<1)                      // FIFO error
#define DMA14DEBUG_LAST     (1<<0)                      // Read Last Not Set Error


//
// -- The DMA15 controller
//    --------------------
#define DMA15_BASE          (HW_BASE+0xe05000)          // Base Address of DMA15
//-------------------------------------------------------------------------------------------------------------------


#define DMA15_CS            (DMA15_BASE)                // Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define DMA15CS_RESET       (1<<31)                     // reset DMA
#define DMA15CS_ABORT       (1<<30)                     // abort current CB
#define DMA15CS_DISDBG      (1<<29)                     // disable debug
#define DMA15CS_WAIT        (1<<28)                     // wait for outstanding writes
#define DMA15CS_PANICPTY    (0xf<<20)                   // priority of panicking transactions
#define DMA15CS_PTY         (0xf<<16)                   // priority of transactions
#define DMA15CS_ERROR       (1<<8)                      // Channel has error
#define DMA15CS_WAITING     (1<<6)                      // is waiting for outstanding writes
#define DMA15CS_REQSTOP     (1<<5)                      // DREQ stops DMA
#define DMA15CS_PAUSE       (1<<4)                      // DMA is paused
#define DMA15CS_DREQ        (1<<3)                      // 1=requesting data
#define DMA15CS_INT         (1<<2)                      // 1=cb ended and INTEN was 1; write 1 to clr
#define DMA15CS_END         (1<<1)                      // 1 when complete
#define DMA15CS_ACTIVE      (1<<0)                      // active

#define SH_DMA15CSPANIC(x)  (((x)&0xf)<<20)             // shift the value to the correct position
#define SH_DMA15CSPTY(x)    (((x)&0xf)<<16)             // shift the value to the correct position


#define DMA15_CBAD          (DMA15_BASE+4)              // Control Block Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA15CBAD_SCB       (0xffffffff)                // Source Control Block address


#define DMA15_TI            (DMA15_BASE+8)              // Transfer Information
//-------------------------------------------------------------------------------------------------------------------
#define DMA15TI_WAITS       (0x1f<<21)                  // Add wait cycles
#define DMA15TI_PMAP        (0x1f<<16)                  // peripheral map
#define DMA15TI_BLEN        (0xf<<12)                   // burst transfer length
#define DMA15TI_SRCIGN      (1<<11)                     // Ignore Reads
#define DMA15TI_SRCDREQ     (1<<10)                     // Control source reads with DREQ
#define DMA15TI_SRCWID      (1<<9)                      // Source Transfer Width
#define DMA15TI_SRCINC      (1<<8)                      // Source Address Increment
#define DMA15TI_DSTIGN      (1<<7)                      // Ignore Writes
#define DMA15TI_DSTDREQ     (1<<6)                      // Control source writes with DREQ
#define DMA15TI_DSTWID      (1<<5)                      // Dest Transfer Width
#define DMA15TI_DSTINC      (1<<4)                      // Dest Address Increment
#define DMA15TI_WRESP       (1<<3)                      // Wait for Response
#define DMA15TI_TMODE       (1<<1)                      // 2D Mode
#define DMA15TI_INTEN       (1<<0)                      // Interrupt Enable

#define SH_DMA15TIWAITS(x)  (((x)&0x1f)<<21)            // shift the value to the correct position
#define SH_DMA15TIPMAP(x)   (((x)&0x1f)<<16)            // shift the value to the correct position


#define DMA15_SRCAD         (DMA15_BASE+0xc)            // Source Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA15SRCAD_SRC      (0xffffffff)                // DMA Source address


#define DMA15_DSTAD         (DMA15_BASE+0x10)           // Destination Address
//-------------------------------------------------------------------------------------------------------------------
#define DMA15DSTAD_SRC      (0xffffffff)                // DMA Dest address


#define DMA15_XLEN          (DMA15_BASE+0x14)           // Transfer Length
//-------------------------------------------------------------------------------------------------------------------
#define DMA15XLEN_X         (0xffff)                    // Transfer bytes length


#define DMA15_STRIDE        (DMA15_BASE+0x18)           // 2D stride
//-------------------------------------------------------------------------------------------------------------------
#define DMA15STRIDE_D       (0xffff<<16)                // Destination Stride
#define DMA15STRIDE_S       (0xffff)                    // Source Stride

#define SH_DMA15STRIDED(x)  (((x)&0xffff)<<16)          // shift to the correct position


#define DMA15_NXTCB         (DMA15_BASE+0x1c)           // Next Control Block
//-------------------------------------------------------------------------------------------------------------------
#define DMA15NXTCB_ADD      (0xffffffff)                // NExt Control Block Address


#define DMA15_DEBUG         (DMA15_BASE+0x20)           // Debug
//-------------------------------------------------------------------------------------------------------------------
#define DMA15DEBUG_LITE     (1<<28)                     // DMA Lite
#define DMA15DEBUG_VERS     (7<<25)                     // DMA Version
#define DMA15DEBUG_STATE    (0x1ff<<16)                 // DMA State
#define DMA15DEBUG_ID       (0xff<<8)                   // DMA ID
#define DMA15DEBUG_OWRITE   (0xf<<4)                    // DMA Outstanding Writes Counter
#define DMA15DEBUG_RERROR   (1<<2)                      // Slave read response error
#define DMA15DEBUG_FERROR   (1<<1)                      // FIFO error
#define DMA15DEBUG_LAST     (1<<0)                      // Read Last Not Set Error


//
// -- The DMA INT_STATUS register
//    ---------------------------
#define DMA_INTSTAT         (DMA0_BASE+0xfe0)           // Interrupt Status Register
//-------------------------------------------------------------------------------------------------------------------
#define DMAINT_15           (1<<15)                     // Interrupt for DMA 15
#define DMAINT_14           (1<<14)                     // Interrupt for DMA 14
#define DMAINT_13           (1<<13)                     // Interrupt for DMA 13
#define DMAINT_12           (1<<12)                     // Interrupt for DMA 12
#define DMAINT_11           (1<<11)                     // Interrupt for DMA 11
#define DMAINT_10           (1<<10)                     // Interrupt for DMA 10
#define DMAINT_9            (1<<9)                      // Interrupt for DMA 9
#define DMAINT_8            (1<<8)                      // Interrupt for DMA 8
#define DMAINT_7            (1<<7)                      // Interrupt for DMA 7
#define DMAINT_6            (1<<6)                      // Interrupt for DMA 6
#define DMAINT_5            (1<<5)                      // Interrupt for DMA 5
#define DMAINT_4            (1<<4)                      // Interrupt for DMA 4
#define DMAINT_3            (1<<3)                      // Interrupt for DMA 3
#define DMAINT_2            (1<<2)                      // Interrupt for DMA 2
#define DMAINT_1            (1<<1)                      // Interrupt for DMA 1
#define DMAINT_0            (1<<0)                      // Interrupt for DMA 0


//
// -- The DMA ENABLE register
//    -----------------------
#define DMA_ENABLE          (DMA0_BASE+0xff0)           // DMA Enable Register
//-------------------------------------------------------------------------------------------------------------------
#define DMAENB_14           (1<<14)                     // Enable DMA 14
#define DMAENB_13           (1<<13)                     // Enable DMA 13
#define DMAENB_12           (1<<12)                     // Enable DMA 12
#define DMAENB_11           (1<<11)                     // Enable DMA 11
#define DMAENB_10           (1<<10)                     // Enable DMA 10
#define DMAENB_9            (1<<9)                      // Enable DMA 9
#define DMAENB_8            (1<<8)                      // Enable DMA 8
#define DMAENB_7            (1<<7)                      // Enable DMA 7
#define DMAENB_6            (1<<6)                      // Enable DMA 6
#define DMAENB_5            (1<<5)                      // Enable DMA 5
#define DMAENB_4            (1<<4)                      // Enable DMA 4
#define DMAENB_3            (1<<3)                      // Enable DMA 3
#define DMAENB_2            (1<<2)                      // Enable DMA 2
#define DMAENB_1            (1<<1)                      // Enable DMA 1
#define DMAENB_0            (1<<0)                      // Enable DMA 0


//-------------------------------------------------------------------------------------------------------------------
// External Mass Media Controller
//-------------------------------------------------------------------------------------------------------------------

//
// -- The EMMC controller
//    -------------------
#define EMMC_BASE           (HW_BASE+0x300000)          // This is the base address of the EMMC controller


#define EMMC_ARG2           (EMMC_BASE)                 // ACMD23 Argument
//-------------------------------------------------------------------------------------------------------------------
#define EMMCARG2_ARGUMENT   (0xffffffff)                // Argument to be issued with ACMD23


#define EMMC_BLKSIZCNT      (EMMC_BASE+4)               // Block Size and Count
//-------------------------------------------------------------------------------------------------------------------
#define EMMCBLK_BLKCNT      (0xffff<<16)                // Number of blocks to be transferred
#define EMMCBLK_BLKSIZE     (0x3ff)                     // Block size in bytes

#define SH_BLKBLKCNT(x)     (((x)&0xffff)<<16)          // Shift the value to the correct position


#define EMMC_ARG1           (EMMC_BASE+8)               // Argument
//-------------------------------------------------------------------------------------------------------------------
#define EMMCARG1_ARGUMENT   (0xffffffff)                // Argument to be issued with command


#define EMMC_CMDTM          (EMMC_BASE+0xc)             // Command and Transfer Mode
//-------------------------------------------------------------------------------------------------------------------
#define EMMCCMDTM_CMDINDEX  (0x3f<<24)                  // Index of the command to issue to the card
#define EMMCCMDTM_CMDTYPE   (0x3<<22)                   // Type of command to be issues to the card
#define EMMCCMDTM_CMDISFATA (1<<21)                     // Command involves data transfer
#define EMMCCMDTM_CMDIXEN   (1<<20)                     // enable chk response same index
#define EMMCCMDTM_CMDCRCEN  (1<<19)                     // check respons CRC
#define EMMCCMDTM_CMDRESPTP (3<<16)                     // Type of expected response from card
#define EMMCCMDTM_TMMBLK    (1<<5)                      // (0=single; 1=multi) block transfer
#define EMMCCMDTM_TMDATDIR  (1<<4)                      // 0=host->card; 1=card->host
#define EMMCCMDTM_TMACMDEN  (3<<2)                      // select the commadn to be sent after completion
#define EMMCCMDTM_TMBCNTEN  (1<<1)                      // enable block counter for multi block transfers

#define SH_EMMCCMDINDEX(x)  (((x)&0x3f)<<24)            // shift the value to the correct position
#define SH_CMDTYPENORMAL    (0b00<<22)                  // Normal command type
#define SH_CMDTYPESUSP      (0b01<<22)                  // suspend (current transfer)
#define SH_CMDTYPERES       (0b10<<22)                  // resume (the last data transfer)
#define SH_CMDTYPEABORT     (0b11<<22)                  // abort (the cuttent data transfer)
#define SH_CMDRESPNORMAL    (0b00<<16)                  // No response
#define SH_CMDRESP136       (0b01<<16)                  // 136-bit response
#define SH_CMDRESP48        (0b10<<16)                  // 48-bit response
#define SH_CMDRESP48BUSY    (0b11<<16)                  // 48-bit response using busy
#define SH_TMAUTOCMDNONE    (0b00<<2)                   // no command
#define SH_TMAUTOCMD12      (0b01<<2)                   // command CMD12
#define SH_TMAUTOCMD23      (0b10<<2)                   // command CMD23
#define SH_TMAUTORESERVED   (0b11<<2)                   // reserved


#define EMMC_RESP0          (EMMC_BASE+0x10)            // Response bits 31:0
//-------------------------------------------------------------------------------------------------------------------
#define EMMCRESP0_RESP      (0xffffffff)                // response bits


#define EMMC_RESP1          (EMMC_BASE+0x14)            // Response bits 63:32
//-------------------------------------------------------------------------------------------------------------------
#define EMMCRESP1_RESP      (0xffffffff)                // response bits


#define EMMC_RESP2          (EMMC_BASE+0x18)            // Response bits 95:64
//-------------------------------------------------------------------------------------------------------------------
#define EMMCRESP2_RESP      (0xffffffff)                // response bits


#define EMMC_RESP3          (EMMC_BASE+0x1c)            // Response bits 127:96
//-------------------------------------------------------------------------------------------------------------------
#define EMMCRESP3_RESP      (0xffffffff)                // response bits


#define EMMC_DATA           (EMMC_BASE+0x20)            // Data
//-------------------------------------------------------------------------------------------------------------------
#define EMMCDATA_DATA       (0xffffffff)                // data to/from card


#define EMMC_STATUS         (EMMC_BASE+0x24)            // Status
//-------------------------------------------------------------------------------------------------------------------
#define EMMCSTS_DAT7        (1<<28)                     // value of data line DAT7
#define EMMCSTS_DAT6        (1<<27)                     // value of data line DAT6
#define EMMCSTS_DAT5        (1<<26)                     // value of data line DAT5
#define EMMCSTS_DAT4        (1<<25)                     // value of data line DAT4
#define EMMCSTS_CMD         (1<<24)                     // value of command line CMD
#define EMMCSTS_DAT3        (1<<23)                     // value of data line DAT3
#define EMMCSTS_DAT2        (1<<22)                     // value of data line DAT2
#define EMMCSTS_DAT1        (1<<21)                     // value of data line DAT1
#define EMMCSTS_DAT0        (1<<20)                     // value of data line DAT0
#define EMMCSTS_RDXFR       (1<<9)                      // new data can be read from EMMC
#define EMMCSTS_WRXFR       (1<<8)                      // new data can be written from EMMC
#define EMMCSTS_ACTIVE      (1<<2)                      // at least one data line is active
#define EMMCSTS_DINHIBIT    (1<<1)                      // data lines still used by previous transfer
#define EMMCSTS_CINHIBIT    (1<<0)                      // command lines still used by previous transfer


#define EMMC_CONTROL0       (EMMC_BASE+0x28)            // Host configuration bits
//-------------------------------------------------------------------------------------------------------------------
#define EMMCCTL0_ALTBOOT    (1<<22)                     // enable alt boot mode access
#define EMMCCTL0_BOOT       (1<<21)                     // (0=stop; 1=start) boot mode access
#define EMMCCTL0_SPI        (1<<20)                     // SPI mode enable
#define EMMCCTL0_GAPI       (1<<19)                     // Enable interrupt at block gap
#define EMMCCTL0_RDWAIT     (1<<18)                     // Use DAT2 for read-wait protocol
#define EMMCCTL0_GAPRESTART (1<<17)                     // restart a transaction stopped using GAP_STOP
#define EMMCCTL0_GAPSTOP    (1<<16)                     // stop the cuurent transaction at the next block gap
#define EMMCCTL0_HCL8BIT    (1<<5)                      // use 8 data lines
#define EMMCCTL0_HS         (1<<2)                      // use high speed mode
#define EMMCCTL0_DWIDTH     (1<<1)                      // use 4 data lines


#define EMMC_CONTROL1       (EMMC_BASE+0x2c)            // Host configuration bits
//-------------------------------------------------------------------------------------------------------------------
#define EMMCCTL1_RESETDATA  (1<<26)                     // Reset data handling circuit
#define EMMCCTL1_RESETCMD   (1<<25)                     // Reset command handling circuit
#define EMMCCTL1_RESETHC    (1<<24)                     // Reset host circuit
#define EMMCCTL1_DATATOUT   (0xf<<16)                   // Data timeout unit exponent
#define EMMCCTL1_CLKFREQ8   (0xff<<8)                   // SD Clock base Divisor (LSBs)
#define EMMCCTL1_CLKFREQ2   (0x3<<6)                    // SD Clock base Divisor (MSBs)
#define EMMCCTL1_CLKGENSEL  (1<<5)                      // clock (0=divided; 1=programmed) mode
#define EMMCCTL1_CLKEN      (1<<2)                      // SD clock enable
#define EMMCCTL1_CLKSTABLE  (1<<1)                      // SD Clock stable
#define EMMCCTL1_CLKINTLEN  (1<<0)                      // clock enabled for internal power saving

#define SH_EMMCCTL1DTO(x)   (((x)&0xf)<<16)             // shift to the correct bits
#define SH_EMMCCTL1DTO_DIS  (0xf<<16)                   // disable data timeout unit
#define SH_EMMCCTL1FREQ8(x) (((x)&0xff)<<8)             // shift to the correct bits
#define SH_EMMCCTL1FREQ2(x) (((x)&0x3)<<8)              // shift to the correct bits


#define EMMC_INTERRUPT      (EMMC_BASE+0x30)            // Interrupt flags
//-------------------------------------------------------------------------------------------------------------------
#define EMMCINT_ACMDERR     (1<<24)                     // auto command error
#define EMMCINT_DENDERR     (1<<22)                     // end bit on data line not 1
#define EMMCINT_DCRCERR     (1<<21)                     // Data CRC error
#define EMMCINT_DTOERR      (1<<20)                     // timeout on data line
#define EMMCINT_CBADERR     (1<<19)                     // incorrect cmd index response
#define EMMCINT_CENDERR     (1<<18)                     // end bit on command not 1
#define EMMCINT_CCRCERR     (1<<17)                     // Command CRC error
#define EMMCINT_CTOERR      (1<<16)                     // timeout on command line
#define EMMCINT_ERR         (1<<15)                     // an error has occured
#define EMMCINT_ENDBOOT     (1<<14)                     // boot operation terminated
#define EMMCINT_BOOTACK     (1<<13)                     // boot ACK received
#define EMMCINT_RETUNE      (1<<12)                     // clock retune request made
#define EMMCINT_CARD        (1<<8)                      // card made interrupt request
#define EMMCINT_RDRDY       (1<<5)                      // data register contains data to read
#define EMMCINT_WRRDY       (1<<4)                      // data can be written to the data register
#define EMMCINT_BLOCKGAP    (1<<2)                      // data transfer has stopped at block gap
#define EMMCINT_DATANONE    (1<<1)                      // data transfer finished
#define EMMCINT_CMDDONE     (1<<0)                      // Command has finished


#define EMMC_IRPTMASK       (EMMC_BASE+0x34)            // Interrupt flag enable
//-------------------------------------------------------------------------------------------------------------------
#define EMMCMSK_ACMDERR     (1<<24)                     // auto command error
#define EMMCMSK_DENDERR     (1<<22)                     // end bit on data line not 1
#define EMMCMSK_DCRCERR     (1<<21)                     // Data CRC error
#define EMMCMSK_DTOERR      (1<<20)                     // timeout on data line
#define EMMCMSK_CBADERR     (1<<19)                     // incorrect cmd index response
#define EMMCMSK_CENDERR     (1<<18)                     // end bit on command not 1
#define EMMCMSK_CCRCERR     (1<<17)                     // Command CRC error
#define EMMCMSK_CTOERR      (1<<16)                     // timeout on command line
#define EMMCMSK_ENDBOOT     (1<<14)                     // boot operation terminated
#define EMMCMSK_BOOTACK     (1<<13)                     // boot ACK received
#define EMMCMSK_RETUNE      (1<<12)                     // clock retune request made
#define EMMCMSK_CARD        (1<<8)                      // card made interrupt request
#define EMMCMSK_RDRDY       (1<<5)                      // data register contains data to read
#define EMMCMSK_WRRDY       (1<<4)                      // data can be written to the data register
#define EMMCMSK_BLOCKGAP    (1<<2)                      // data transfer has stopped at block gap
#define EMMCMSK_DATANONE    (1<<1)                      // data transfer finished
#define EMMCMSK_CMDDONE     (1<<0)                      // Command has finished


#define EMMC_IRPTEN         (EMMC_BASE+0x38)            // Interrupt Generation Enable
//-------------------------------------------------------------------------------------------------------------------
#define EMMCIEN_ACMDERR     (1<<24)                     // auto command error
#define EMMCIEN_DENDERR     (1<<22)                     // end bit on data line not 1
#define EMMCIEN_DCRCERR     (1<<21)                     // Data CRC error
#define EMMCIEN_DTOERR      (1<<20)                     // timeout on data line
#define EMMCIEN_CBADERR     (1<<19)                     // incorrect cmd index response
#define EMMCIEN_CENDERR     (1<<18)                     // end bit on command not 1
#define EMMCIEN_CCRCERR     (1<<17)                     // Command CRC error
#define EMMCIEN_CTOERR      (1<<16)                     // timeout on command line
#define EMMCIEN_ENDBOOT     (1<<14)                     // boot operation terminated
#define EMMCIEN_BOOTACK     (1<<13)                     // boot ACK received
#define EMMCIEN_RETUNE      (1<<12)                     // clock retune request made
#define EMMCIEN_CARD        (1<<8)                      // card made interrupt request
#define EMMCIEN_RDRDY       (1<<5)                      // data register contains data to read
#define EMMCIEN_WRRDY       (1<<4)                      // data can be written to the data register
#define EMMCIEN_BLOCKGAP    (1<<2)                      // data transfer has stopped at block gap
#define EMMCIEN_DATANONE    (1<<1)                      // data transfer finished
#define EMMCIEN_CMDDONE     (1<<0)                      // Command has finished


#define EMMC_CONTROL2       (EMMC_BASE+0x3c)            // Host configuration bits
//-------------------------------------------------------------------------------------------------------------------
#define EMMCCTL2_TUNED      (1<<23)                     // Tuned clock is used
#define EMMCCTL2_TUNEON     (1<<22)                     // start tuning the clock
#define EMMCCTL2_SPDMOD     (7<<16)                     // Speed mode of the SD card
#define EMMCCTL2_C12ERR     (1<<7)                      // CMD12 error
#define EMMCCTL2_ACBADERR   (1<<4)                      // auto cmd execution error
#define EMMCCTL2_ACENDERR   (1<<3)                      // end bits not 1 during auto cmd exec
#define EMMCCTL2_ACCRCERR   (1<<2)                      // Commend CRC error
#define EMMCCTL2_ACTOERR    (1<<1)                      // timeout occurred
#define EMMCCTL2_NOTEXERR   (1<<0)                      // not executed due to error

#define SH_EMMCCTL2SDR12    (0b000<<16)                 // SDR16
#define SH_EMMCCTL2SDR25    (0b001<<16)                 // SDR25
#define SH_EMMCCTL2SDR50    (0b010<<16)                 // SDR50
#define SH_EMMCCTL2SDR104   (0b011<<16)                 // SDR104
#define SH_EMMCCTL2DDR50    (0b100<<16)                 // DDR50


#define EMMC_FORCEIRPT      (EMMC_BASE+0x50)            // Force interrupt event
//-------------------------------------------------------------------------------------------------------------------
#define EMMCFRC_ACMDERR     (1<<24)                     // auto command error
#define EMMCFRC_DENDERR     (1<<22)                     // end bit on data line not 1
#define EMMCFRC_DCRCERR     (1<<21)                     // Data CRC error
#define EMMCFRC_DTOERR      (1<<20)                     // timeout on data line
#define EMMCFRC_CBADERR     (1<<19)                     // incorrect cmd index response
#define EMMCFRC_CENDERR     (1<<18)                     // end bit on command not 1
#define EMMCFRC_CCRCERR     (1<<17)                     // Command CRC error
#define EMMCFRC_CTOERR      (1<<16)                     // timeout on command line
#define EMMCFRC_ENDBOOT     (1<<14)                     // boot operation terminated
#define EMMCFRC_BOOTACK     (1<<13)                     // boot ACK received
#define EMMCFRC_RETUNE      (1<<12)                     // clock retune request made
#define EMMCFRC_CARD        (1<<8)                      // card made interrupt request
#define EMMCFRC_RDRDY       (1<<5)                      // data register contains data to read
#define EMMCFRC_WRRDY       (1<<4)                      // data can be written to the data register
#define EMMCFRC_BLOCKGAP    (1<<2)                      // data transfer has stopped at block gap
#define EMMCFRC_DATANONE    (1<<1)                      // data transfer finished
#define EMMCFRC_CMDDONE     (1<<0)                      // Command has finished


#define EMMC_BOOTTIMEOUT    (EMMC_BASE+0x70)            // Timeout in boot mode
//-------------------------------------------------------------------------------------------------------------------
#define EMMCBOOTTO_TOUT     (0xffffffff)                // # clock cycles for timeout


#define EMMC_DBGSEL         (EMMC_BASE+0x74)            // Debug bus configuration
//-------------------------------------------------------------------------------------------------------------------
#define EMMCDBGSEL_SEL      (1<<0)                      // submodules accessed by dbg bus (0=receiver/FIFO; 1=others)


#define EMMC_EXRDFIFOCFG    (EMMC_BASE+0x80)            // Extension FIFO configuration
//-------------------------------------------------------------------------------------------------------------------
#define EMMCRDCFG_SEL       (7<<0)                      // read threshold in 32-bit words


#define EMMC_EXRDFIFOEN     (EMMC_BASE+0x84)            // Extension FIFO enable
//-------------------------------------------------------------------------------------------------------------------
#define EMMCRDENB_ENABLE    (1<<0)                      // enable extension FIFO


#define EMMC_TUNESTEP       (EMMC_BASE+0x88)            // Delay per card clock tuning step
//-------------------------------------------------------------------------------------------------------------------
#define EMMCTUNESTP_DLY     (7<<0)                      // Sampling clock delay per step

#define SH_EMMCDLY200PS     (0b000)                     // 200ps
#define SH_EMMCDLY400PS     (0b001)                     // 400ps
#define SH_EMMCDLY400PS2    (0b010)                     // 400ps
#define SH_EMMCDLY600PS     (0b011)                     // 600ps
#define SH_EMMCDLY700PS     (0b100)                     // 700ps
#define SH_EMMCDLY900PS     (0b101)                     // 900ps
#define SH_EMMCDLY900PS2    (0b110)                     // 900ps
#define SH_EMMCDLY1100PS    (0b111)                     // 1100ps


#define EMMC_TUNESTEPSTD    (EMMC_BASE+0x8c)            // Card clock tuning steps for SDR
//-------------------------------------------------------------------------------------------------------------------
#define EMMCTUNSTD_STEPS    (0x3f)                      // Number of steps (0-40)


#define EMMC_TUNESTEPDDR    (EMMC_BASE+0x90)            // Card clock tuning steps for DDR
//-------------------------------------------------------------------------------------------------------------------
#define EMMCTUNDDR_STEPS    (0x3f)                      // Number of steps (0-40)


#define EMMC_SPIINTSPT      (EMMC_BASE+0xf0)            // SPI Interrupt Support
//-------------------------------------------------------------------------------------------------------------------
#define EMMCSPIINT7         (1<<7)                      // Interrupt independent of card sel line 7
#define EMMCSPIINT6         (1<<6)                      // Interrupt independent of card sel line 6
#define EMMCSPIINT5         (1<<5)                      // Interrupt independent of card sel line 5
#define EMMCSPIINT4         (1<<4)                      // Interrupt independent of card sel line 4
#define EMMCSPIINT3         (1<<3)                      // Interrupt independent of card sel line 3
#define EMMCSPIINT2         (1<<2)                      // Interrupt independent of card sel line 2
#define EMMCSPIINT1         (1<<1)                      // Interrupt independent of card sel line 1
#define EMMCSPIINT0         (1<<0)                      // Interrupt independent of card sel line 0


#define EMMC_SLOTISRVER     (EMMC_BASE+0xfc)            // Slot Interrupt Status and Version
//-------------------------------------------------------------------------------------------------------------------
#define EMMCVER_VEND        (0xff<<24)                  // Vendor version number
#define EMMCVER_SDVERS      (0xff<<16)                  // Hast Controller specification version
#define EMMCVER_SLOTSTS7    (1<<7)                      // wakeup signal for each slot 7
#define EMMCVER_SLOTSTS6    (1<<6)                      // wakeup signal for each slot 6
#define EMMCVER_SLOTSTS5    (1<<5)                      // wakeup signal for each slot 5
#define EMMCVER_SLOTSTS4    (1<<4)                      // wakeup signal for each slot 4
#define EMMCVER_SLOTSTS3    (1<<3)                      // wakeup signal for each slot 3
#define EMMCVER_SLOTSTS2    (1<<2)                      // wakeup signal for each slot 2
#define EMMCVER_SLOTSTS1    (1<<1)                      // wakeup signal for each slot 1
#define EMMCVER_SLOTSTS0    (1<<0)                      // wakeup signal for each slot 0


//-------------------------------------------------------------------------------------------------------------------
// General Purpose I/O
//-------------------------------------------------------------------------------------------------------------------

//
// -- The GPIO
//    --------
#define GPIO_BASE           (HW_BASE+0x200000)          // This is the base address of the GPIO


//
// -- since there is a lot of duplication, reuse these values
//    -------------------------------------------------------
#define GPIO_PININ          (0b000)                     // Pin is an input
#define GPIO_PINOUT         (0b001)                     // Pin is an output
#define GPIO_AFUNC0         (0b100)                     // Takes alternate function 0
#define GPIO_AFUNC1         (0b101)                     // Takes alternate function 1
#define GPIO_AFUNC2         (0b110)                     // Takes alternate function 2
#define GPIO_AFUNC3         (0b111)                     // Takes alternate function 3
#define GPIO_AFUNC4         (0b011)                     // Takes alternate function 4
#define GPIO_AFUNC5         (0b110)                     // Takes alternate function 5


#define GPIO_FSEL0          (GPIO_BASE)                 // GPIO Function Select 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL0_SEL9      (7<<27)                     // Function Select 9
#define GPIOFSEL0_SEL8      (7<<24)                     // Function Select 8
#define GPIOFSEL0_SEL7      (7<<21)                     // Function Select 7
#define GPIOFSEL0_SEL6      (7<<18)                     // Function Select 6
#define GPIOFSEL0_SEL5      (7<<15)                     // Function Select 5
#define GPIOFSEL0_SEL4      (7<<12)                     // Function Select 4
#define GPIOFSEL0_SEL3      (7<<9)                      // Function Select 3
#define GPIOFSEL0_SEL2      (7<<6)                      // Function Select 2
#define GPIOFSEL0_SEL1      (7<<3)                      // Function Select 1
#define GPIOFSEL0_SEL0      (7<<0)                      // Function Select 0

#define SH_SEL9(x)          (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL8(x)          (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL7(x)          (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL6(x)          (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL5(x)          (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL4(x)          (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL3(x)          (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL2(x)          (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL1(x)          (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL0(x)          (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL1          (GPIO_BASE+4)               // GPIO Function Select 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL1_SEL19     (7<<27)                     // Function Select 19
#define GPIOFSEL1_SEL18     (7<<24)                     // Function Select 18
#define GPIOFSEL1_SEL17     (7<<21)                     // Function Select 17
#define GPIOFSEL1_SEL16     (7<<18)                     // Function Select 16
#define GPIOFSEL1_SEL15     (7<<15)                     // Function Select 15
#define GPIOFSEL1_SEL14     (7<<12)                     // Function Select 14
#define GPIOFSEL1_SEL13     (7<<9)                      // Function Select 13
#define GPIOFSEL1_SEL12     (7<<6)                      // Function Select 12
#define GPIOFSEL1_SEL11     (7<<3)                      // Function Select 11
#define GPIOFSEL1_SEL10     (7<<0)                      // Function Select 10

#define SH_SEL19(x)         (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL18(x)         (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL17(x)         (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL16(x)         (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL15(x)         (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL14(x)         (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL13(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL12(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL11(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL10(x)         (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL2          (GPIO_BASE+8)               // GPIO Function Select 2
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL2_SEL29     (7<<27)                     // Function Select 29
#define GPIOFSEL2_SEL28     (7<<24)                     // Function Select 28
#define GPIOFSEL2_SEL27     (7<<21)                     // Function Select 27
#define GPIOFSEL2_SEL26     (7<<18)                     // Function Select 26
#define GPIOFSEL2_SEL25     (7<<15)                     // Function Select 25
#define GPIOFSEL2_SEL24     (7<<12)                     // Function Select 24
#define GPIOFSEL2_SEL23     (7<<9)                      // Function Select 23
#define GPIOFSEL2_SEL22     (7<<6)                      // Function Select 22
#define GPIOFSEL2_SEL21     (7<<3)                      // Function Select 21
#define GPIOFSEL2_SEL20     (7<<0)                      // Function Select 20

#define SH_SEL29(x)         (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL28(x)         (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL27(x)         (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL26(x)         (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL25(x)         (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL24(x)         (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL23(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL22(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL21(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL20(x)         (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL3          (GPIO_BASE+0xc)             // GPIO Function Select 3
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL3_SEL39     (7<<27)                     // Function Select 39
#define GPIOFSEL3_SEL38     (7<<24)                     // Function Select 38
#define GPIOFSEL3_SEL37     (7<<21)                     // Function Select 37
#define GPIOFSEL3_SEL36     (7<<18)                     // Function Select 36
#define GPIOFSEL3_SEL35     (7<<15)                     // Function Select 35
#define GPIOFSEL3_SEL34     (7<<12)                     // Function Select 34
#define GPIOFSEL3_SEL33     (7<<9)                      // Function Select 33
#define GPIOFSEL3_SEL32     (7<<6)                      // Function Select 32
#define GPIOFSEL3_SEL31     (7<<3)                      // Function Select 31
#define GPIOFSEL3_SEL30     (7<<0)                      // Function Select 30

#define SH_SEL39(x)         (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL38(x)         (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL37(x)         (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL36(x)         (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL35(x)         (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL34(x)         (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL33(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL32(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL31(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL30(x)         (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL4          (GPIO_BASE+0x10)            // GPIO Function Select 4
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL4_SEL49     (7<<27)                     // Function Select 49
#define GPIOFSEL4_SEL48     (7<<24)                     // Function Select 48
#define GPIOFSEL4_SEL47     (7<<21)                     // Function Select 47
#define GPIOFSEL4_SEL46     (7<<18)                     // Function Select 46
#define GPIOFSEL4_SEL45     (7<<15)                     // Function Select 45
#define GPIOFSEL4_SEL44     (7<<12)                     // Function Select 44
#define GPIOFSEL4_SEL43     (7<<9)                      // Function Select 43
#define GPIOFSEL4_SEL42     (7<<6)                      // Function Select 42
#define GPIOFSEL4_SEL41     (7<<3)                      // Function Select 41
#define GPIOFSEL4_SEL40     (7<<0)                      // Function Select 40

#define SH_SEL49(x)         (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL48(x)         (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL47(x)         (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL46(x)         (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL45(x)         (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL44(x)         (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL43(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL42(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL41(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL40(x)         (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL5          (GPIO_BASE+0x14)            // GPIO Function Select 5
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL5_SEL53     (7<<9)                      // Function Select 53
#define GPIOFSEL5_SEL52     (7<<6)                      // Function Select 52
#define GPIOFSEL5_SEL51     (7<<3)                      // Function Select 51
#define GPIOFSEL5_SEL50     (7<<0)                      // Function Select 50

#define SH_SEL53(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL52(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL51(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL50(x)         (((x)&0x7)<<0)              // Shift to the proper bits



#define GPIO_SET0           (GPIO_BASE+0x1c)            // GPIO Pin Output Set 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOSET0_31         (1<<31)                     // Set GPIO pin 31
#define GPIOSET0_30         (1<<30)                     // Set GPIO pin 30
#define GPIOSET0_29         (1<<29)                     // Set GPIO pin 29
#define GPIOSET0_28         (1<<28)                     // Set GPIO pin 28
#define GPIOSET0_27         (1<<27)                     // Set GPIO pin 27
#define GPIOSET0_26         (1<<26)                     // Set GPIO pin 26
#define GPIOSET0_25         (1<<25)                     // Set GPIO pin 25
#define GPIOSET0_24         (1<<24)                     // Set GPIO pin 24
#define GPIOSET0_23         (1<<23)                     // Set GPIO pin 23
#define GPIOSET0_22         (1<<22)                     // Set GPIO pin 22
#define GPIOSET0_21         (1<<21)                     // Set GPIO pin 21
#define GPIOSET0_20         (1<<20)                     // Set GPIO pin 20
#define GPIOSET0_19         (1<<19)                     // Set GPIO pin 19
#define GPIOSET0_18         (1<<18)                     // Set GPIO pin 18
#define GPIOSET0_17         (1<<17)                     // Set GPIO pin 17
#define GPIOSET0_16         (1<<16)                     // Set GPIO pin 16
#define GPIOSET0_15         (1<<15)                     // Set GPIO pin 15
#define GPIOSET0_14         (1<<14)                     // Set GPIO pin 14
#define GPIOSET0_13         (1<<13)                     // Set GPIO pin 13
#define GPIOSET0_12         (1<<12)                     // Set GPIO pin 12
#define GPIOSET0_11         (1<<11)                     // Set GPIO pin 11
#define GPIOSET0_10         (1<<10)                     // Set GPIO pin 10
#define GPIOSET0_9          (1<<9)                      // Set GPIO pin 9
#define GPIOSET0_8          (1<<8)                      // Set GPIO pin 8
#define GPIOSET0_7          (1<<7)                      // Set GPIO pin 7
#define GPIOSET0_6          (1<<6)                      // Set GPIO pin 6
#define GPIOSET0_5          (1<<5)                      // Set GPIO pin 5
#define GPIOSET0_4          (1<<4)                      // Set GPIO pin 4
#define GPIOSET0_3          (1<<3)                      // Set GPIO pin 3
#define GPIOSET0_2          (1<<2)                      // Set GPIO pin 2
#define GPIOSET0_1          (1<<1)                      // Set GPIO pin 1
#define GPIOSET0_0          (1<<0)                      // Set GPIO pin 0


#define GPIO_SET1           (GPIO_BASE+0x20)            // GPIO Pin Output Set 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOSET1_53         (1<<21)                     // Set GPIO pin 53
#define GPIOSET1_52         (1<<20)                     // Set GPIO pin 52
#define GPIOSET1_51         (1<<19)                     // Set GPIO pin 51
#define GPIOSET1_50         (1<<18)                     // Set GPIO pin 50
#define GPIOSET1_49         (1<<17)                     // Set GPIO pin 49
#define GPIOSET1_48         (1<<16)                     // Set GPIO pin 48
#define GPIOSET1_47         (1<<15)                     // Set GPIO pin 47
#define GPIOSET1_46         (1<<14)                     // Set GPIO pin 46
#define GPIOSET1_45         (1<<13)                     // Set GPIO pin 45
#define GPIOSET1_44         (1<<12)                     // Set GPIO pin 44
#define GPIOSET1_43         (1<<11)                     // Set GPIO pin 43
#define GPIOSET1_42         (1<<10)                     // Set GPIO pin 42
#define GPIOSET1_41         (1<<9)                      // Set GPIO pin 41
#define GPIOSET1_40         (1<<8)                      // Set GPIO pin 40
#define GPIOSET1_39         (1<<7)                      // Set GPIO pin 39
#define GPIOSET1_38         (1<<6)                      // Set GPIO pin 38
#define GPIOSET1_37         (1<<5)                      // Set GPIO pin 37
#define GPIOSET1_36         (1<<4)                      // Set GPIO pin 36
#define GPIOSET1_35         (1<<3)                      // Set GPIO pin 35
#define GPIOSET1_34         (1<<2)                      // Set GPIO pin 34
#define GPIOSET1_33         (1<<1)                      // Set GPIO pin 33
#define GPIOSET1_32         (1<<0)                      // Set GPIO pin 32


#define GPIO_CLR0           (GPIO_BASE+0x28)            // GPIO Pin Output Clear 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOCLR0_31         (1<<31)                     // Clear GPIO pin 31
#define GPIOCLR0_30         (1<<30)                     // Clear GPIO pin 30
#define GPIOCLR0_29         (1<<29)                     // Clear GPIO pin 29
#define GPIOCLR0_28         (1<<28)                     // Clear GPIO pin 28
#define GPIOCLR0_27         (1<<27)                     // Clear GPIO pin 27
#define GPIOCLR0_26         (1<<26)                     // Clear GPIO pin 26
#define GPIOCLR0_25         (1<<25)                     // Clear GPIO pin 25
#define GPIOCLR0_24         (1<<24)                     // Clear GPIO pin 24
#define GPIOCLR0_23         (1<<23)                     // Clear GPIO pin 23
#define GPIOCLR0_22         (1<<22)                     // Clear GPIO pin 22
#define GPIOCLR0_21         (1<<21)                     // Clear GPIO pin 21
#define GPIOCLR0_20         (1<<20)                     // Clear GPIO pin 20
#define GPIOCLR0_19         (1<<19)                     // Clear GPIO pin 19
#define GPIOCLR0_18         (1<<18)                     // Clear GPIO pin 18
#define GPIOCLR0_17         (1<<17)                     // Clear GPIO pin 17
#define GPIOCLR0_16         (1<<16)                     // Clear GPIO pin 16
#define GPIOCLR0_15         (1<<15)                     // Clear GPIO pin 15
#define GPIOCLR0_14         (1<<14)                     // Clear GPIO pin 14
#define GPIOCLR0_13         (1<<13)                     // Clear GPIO pin 13
#define GPIOCLR0_12         (1<<12)                     // Clear GPIO pin 12
#define GPIOCLR0_11         (1<<11)                     // Clear GPIO pin 11
#define GPIOCLR0_10         (1<<10)                     // Clear GPIO pin 10
#define GPIOCLR0_9          (1<<9)                      // Clear GPIO pin 9
#define GPIOCLR0_8          (1<<8)                      // Clear GPIO pin 8
#define GPIOCLR0_7          (1<<7)                      // Clear GPIO pin 7
#define GPIOCLR0_6          (1<<6)                      // Clear GPIO pin 6
#define GPIOCLR0_5          (1<<5)                      // Clear GPIO pin 5
#define GPIOCLR0_4          (1<<4)                      // Clear GPIO pin 4
#define GPIOCLR0_3          (1<<3)                      // Clear GPIO pin 3
#define GPIOCLR0_2          (1<<2)                      // Clear GPIO pin 2
#define GPIOCLR0_1          (1<<1)                      // Clear GPIO pin 1
#define GPIOCLR0_0          (1<<0)                      // Clear GPIO pin 0


#define GPIO_CLR1           (GPIO_BASE+0x2c)            // GPIO Pin Output Clear 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOCLR1_53         (1<<21)                     // Clear GPIO pin 53
#define GPIOCLR1_52         (1<<20)                     // Clear GPIO pin 52
#define GPIOCLR1_51         (1<<19)                     // Clear GPIO pin 51
#define GPIOCLR1_50         (1<<18)                     // Clear GPIO pin 50
#define GPIOCLR1_49         (1<<17)                     // Clear GPIO pin 49
#define GPIOCLR1_48         (1<<16)                     // Clear GPIO pin 48
#define GPIOCLR1_47         (1<<15)                     // Clear GPIO pin 47
#define GPIOCLR1_46         (1<<14)                     // Clear GPIO pin 46
#define GPIOCLR1_45         (1<<13)                     // Clear GPIO pin 45
#define GPIOCLR1_44         (1<<12)                     // Clear GPIO pin 44
#define GPIOCLR1_43         (1<<11)                     // Clear GPIO pin 43
#define GPIOCLR1_42         (1<<10)                     // Clear GPIO pin 42
#define GPIOCLR1_41         (1<<9)                      // Clear GPIO pin 41
#define GPIOCLR1_40         (1<<8)                      // Clear GPIO pin 40
#define GPIOCLR1_39         (1<<7)                      // Clear GPIO pin 39
#define GPIOCLR1_38         (1<<6)                      // Clear GPIO pin 38
#define GPIOCLR1_37         (1<<5)                      // Clear GPIO pin 37
#define GPIOCLR1_36         (1<<4)                      // Clear GPIO pin 36
#define GPIOCLR1_35         (1<<3)                      // Clear GPIO pin 35
#define GPIOCLR1_34         (1<<2)                      // Clear GPIO pin 34
#define GPIOCLR1_33         (1<<1)                      // Clear GPIO pin 33
#define GPIOCLR1_32         (1<<0)                      // Clear GPIO pin 32


#define GPIO_LEV0           (GPIO_BASE+0x34)            // GPIO Pin Level 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOLVL0_31         (1<<31)                     // Level GPIO pin 31
#define GPIOLVL0_30         (1<<30)                     // Level GPIO pin 30
#define GPIOLVL0_29         (1<<29)                     // Level GPIO pin 29
#define GPIOLVL0_28         (1<<28)                     // Level GPIO pin 28
#define GPIOLVL0_27         (1<<27)                     // Level GPIO pin 27
#define GPIOLVL0_26         (1<<26)                     // Level GPIO pin 26
#define GPIOLVL0_25         (1<<25)                     // Level GPIO pin 25
#define GPIOLVL0_24         (1<<24)                     // Level GPIO pin 24
#define GPIOLVL0_23         (1<<23)                     // Level GPIO pin 23
#define GPIOLVL0_22         (1<<22)                     // Level GPIO pin 22
#define GPIOLVL0_21         (1<<21)                     // Level GPIO pin 21
#define GPIOLVL0_20         (1<<20)                     // Level GPIO pin 20
#define GPIOLVL0_19         (1<<19)                     // Level GPIO pin 19
#define GPIOLVL0_18         (1<<18)                     // Level GPIO pin 18
#define GPIOLVL0_17         (1<<17)                     // Level GPIO pin 17
#define GPIOLVL0_16         (1<<16)                     // Level GPIO pin 16
#define GPIOLVL0_15         (1<<15)                     // Level GPIO pin 15
#define GPIOLVL0_14         (1<<14)                     // Level GPIO pin 14
#define GPIOLVL0_13         (1<<13)                     // Level GPIO pin 13
#define GPIOLVL0_12         (1<<12)                     // Level GPIO pin 12
#define GPIOLVL0_11         (1<<11)                     // Level GPIO pin 11
#define GPIOLVL0_10         (1<<10)                     // Level GPIO pin 10
#define GPIOLVL0_9          (1<<9)                      // Level GPIO pin 9
#define GPIOLVL0_8          (1<<8)                      // Level GPIO pin 8
#define GPIOLVL0_7          (1<<7)                      // Level GPIO pin 7
#define GPIOLVL0_6          (1<<6)                      // Level GPIO pin 6
#define GPIOLVL0_5          (1<<5)                      // Level GPIO pin 5
#define GPIOLVL0_4          (1<<4)                      // Level GPIO pin 4
#define GPIOLVL0_3          (1<<3)                      // Level GPIO pin 3
#define GPIOLVL0_2          (1<<2)                      // Level GPIO pin 2
#define GPIOLVL0_1          (1<<1)                      // Level GPIO pin 1
#define GPIOLVL0_0          (1<<0)                      // Level GPIO pin 0


#define GPIO_LEV1           (GPIO_BASE+0x38)            // GPIO Pin Level 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOLVL1_53         (1<<21)                     // Level GPIO pin 53
#define GPIOLVL1_52         (1<<20)                     // Level GPIO pin 52
#define GPIOLVL1_51         (1<<19)                     // Level GPIO pin 51
#define GPIOLVL1_50         (1<<18)                     // Level GPIO pin 50
#define GPIOLVL1_49         (1<<17)                     // Level GPIO pin 49
#define GPIOLVL1_48         (1<<16)                     // Level GPIO pin 48
#define GPIOLVL1_47         (1<<15)                     // Level GPIO pin 47
#define GPIOLVL1_46         (1<<14)                     // Level GPIO pin 46
#define GPIOLVL1_45         (1<<13)                     // Level GPIO pin 45
#define GPIOLVL1_44         (1<<12)                     // Level GPIO pin 44
#define GPIOLVL1_43         (1<<11)                     // Level GPIO pin 43
#define GPIOLVL1_42         (1<<10)                     // Level GPIO pin 42
#define GPIOLVL1_41         (1<<9)                      // Level GPIO pin 41
#define GPIOLVL1_40         (1<<8)                      // Level GPIO pin 40
#define GPIOLVL1_39         (1<<7)                      // Level GPIO pin 39
#define GPIOLVL1_38         (1<<6)                      // Level GPIO pin 38
#define GPIOLVL1_37         (1<<5)                      // Level GPIO pin 37
#define GPIOLVL1_36         (1<<4)                      // Level GPIO pin 36
#define GPIOLVL1_35         (1<<3)                      // Level GPIO pin 35
#define GPIOLVL1_34         (1<<2)                      // Level GPIO pin 34
#define GPIOLVL1_33         (1<<1)                      // Level GPIO pin 33
#define GPIOLVL1_32         (1<<0)                      // Level GPIO pin 32


#define GPIO_EDS0           (GPIO_BASE+0x40)            // GPIO Pin Event Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOEDS0_31         (1<<31)                     // Event Detected on GPIO pin 31
#define GPIOEDS0_30         (1<<30)                     // Event Detected on GPIO pin 30
#define GPIOEDS0_29         (1<<29)                     // Event Detected on GPIO pin 29
#define GPIOEDS0_28         (1<<28)                     // Event Detected on GPIO pin 28
#define GPIOEDS0_27         (1<<27)                     // Event Detected on GPIO pin 27
#define GPIOEDS0_26         (1<<26)                     // Event Detected on GPIO pin 26
#define GPIOEDS0_25         (1<<25)                     // Event Detected on GPIO pin 25
#define GPIOEDS0_24         (1<<24)                     // Event Detected on GPIO pin 24
#define GPIOEDS0_23         (1<<23)                     // Event Detected on GPIO pin 23
#define GPIOEDS0_22         (1<<22)                     // Event Detected on GPIO pin 22
#define GPIOEDS0_21         (1<<21)                     // Event Detected on GPIO pin 21
#define GPIOEDS0_20         (1<<20)                     // Event Detected on GPIO pin 20
#define GPIOEDS0_19         (1<<19)                     // Event Detected on GPIO pin 19
#define GPIOEDS0_18         (1<<18)                     // Event Detected on GPIO pin 18
#define GPIOEDS0_17         (1<<17)                     // Event Detected on GPIO pin 17
#define GPIOEDS0_16         (1<<16)                     // Event Detected on GPIO pin 16
#define GPIOEDS0_15         (1<<15)                     // Event Detected on GPIO pin 15
#define GPIOEDS0_14         (1<<14)                     // Event Detected on GPIO pin 14
#define GPIOEDS0_13         (1<<13)                     // Event Detected on GPIO pin 13
#define GPIOEDS0_12         (1<<12)                     // Event Detected on GPIO pin 12
#define GPIOEDS0_11         (1<<11)                     // Event Detected on GPIO pin 11
#define GPIOEDS0_10         (1<<10)                     // Event Detected on GPIO pin 10
#define GPIOEDS0_9          (1<<9)                      // Event Detected on GPIO pin 9
#define GPIOEDS0_8          (1<<8)                      // Event Detected on GPIO pin 8
#define GPIOEDS0_7          (1<<7)                      // Event Detected on GPIO pin 7
#define GPIOEDS0_6          (1<<6)                      // Event Detected on GPIO pin 6
#define GPIOEDS0_5          (1<<5)                      // Event Detected on GPIO pin 5
#define GPIOEDS0_4          (1<<4)                      // Event Detected on GPIO pin 4
#define GPIOEDS0_3          (1<<3)                      // Event Detected on GPIO pin 3
#define GPIOEDS0_2          (1<<2)                      // Event Detected on GPIO pin 2
#define GPIOEDS0_1          (1<<1)                      // Event Detected on GPIO pin 1
#define GPIOEDS0_0          (1<<0)                      // Event Detected on GPIO pin 0


#define GPIO_EDS1           (GPIO_BASE+0x44)            // GPIO Pin Event Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOEDS1_53         (1<<21)                     // Event Detected on GPIO pin 53
#define GPIOEDS1_52         (1<<20)                     // Event Detected on GPIO pin 52
#define GPIOEDS1_51         (1<<19)                     // Event Detected on GPIO pin 51
#define GPIOEDS1_50         (1<<18)                     // Event Detected on GPIO pin 50
#define GPIOEDS1_49         (1<<17)                     // Event Detected on GPIO pin 49
#define GPIOEDS1_48         (1<<16)                     // Event Detected on GPIO pin 48
#define GPIOEDS1_47         (1<<15)                     // Event Detected on GPIO pin 47
#define GPIOEDS1_46         (1<<14)                     // Event Detected on GPIO pin 46
#define GPIOEDS1_45         (1<<13)                     // Event Detected on GPIO pin 45
#define GPIOEDS1_44         (1<<12)                     // Event Detected on GPIO pin 44
#define GPIOEDS1_43         (1<<11)                     // Event Detected on GPIO pin 43
#define GPIOEDS1_42         (1<<10)                     // Event Detected on GPIO pin 42
#define GPIOEDS1_41         (1<<9)                      // Event Detected on GPIO pin 41
#define GPIOEDS1_40         (1<<8)                      // Event Detected on GPIO pin 40
#define GPIOEDS1_39         (1<<7)                      // Event Detected on GPIO pin 39
#define GPIOEDS1_38         (1<<6)                      // Event Detected on GPIO pin 38
#define GPIOEDS1_37         (1<<5)                      // Event Detected on GPIO pin 37
#define GPIOEDS1_36         (1<<4)                      // Event Detected on GPIO pin 36
#define GPIOEDS1_35         (1<<3)                      // Event Detected on GPIO pin 35
#define GPIOEDS1_34         (1<<2)                      // Event Detected on GPIO pin 34
#define GPIOEDS1_33         (1<<1)                      // Event Detected on GPIO pin 33
#define GPIOEDS1_32         (1<<0)                      // Event Detected on GPIO pin 32


#define GPIO_REN0           (GPIO_BASE+0x4c)            // GPIO Pin Rising Edge Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOREN0_31         (1<<31)                     // Set Rising Edge Detect on GPIO pin 31
#define GPIOREN0_30         (1<<30)                     // Set Rising Edge Detect on GPIO pin 30
#define GPIOREN0_29         (1<<29)                     // Set Rising Edge Detect on GPIO pin 29
#define GPIOREN0_28         (1<<28)                     // Set Rising Edge Detect on GPIO pin 28
#define GPIOREN0_27         (1<<27)                     // Set Rising Edge Detect on GPIO pin 27
#define GPIOREN0_26         (1<<26)                     // Set Rising Edge Detect on GPIO pin 26
#define GPIOREN0_25         (1<<25)                     // Set Rising Edge Detect on GPIO pin 25
#define GPIOREN0_24         (1<<24)                     // Set Rising Edge Detect on GPIO pin 24
#define GPIOREN0_23         (1<<23)                     // Set Rising Edge Detect on GPIO pin 23
#define GPIOREN0_22         (1<<22)                     // Set Rising Edge Detect on GPIO pin 22
#define GPIOREN0_21         (1<<21)                     // Set Rising Edge Detect on GPIO pin 21
#define GPIOREN0_20         (1<<20)                     // Set Rising Edge Detect on GPIO pin 20
#define GPIOREN0_19         (1<<19)                     // Set Rising Edge Detect on GPIO pin 19
#define GPIOREN0_18         (1<<18)                     // Set Rising Edge Detect on GPIO pin 18
#define GPIOREN0_17         (1<<17)                     // Set Rising Edge Detect on GPIO pin 17
#define GPIOREN0_16         (1<<16)                     // Set Rising Edge Detect on GPIO pin 16
#define GPIOREN0_15         (1<<15)                     // Set Rising Edge Detect on GPIO pin 15
#define GPIOREN0_14         (1<<14)                     // Set Rising Edge Detect on GPIO pin 14
#define GPIOREN0_13         (1<<13)                     // Set Rising Edge Detect on GPIO pin 13
#define GPIOREN0_12         (1<<12)                     // Set Rising Edge Detect on GPIO pin 12
#define GPIOREN0_11         (1<<11)                     // Set Rising Edge Detect on GPIO pin 11
#define GPIOREN0_10         (1<<10)                     // Set Rising Edge Detect on GPIO pin 10
#define GPIOREN0_9          (1<<9)                      // Set Rising Edge Detect on GPIO pin 9
#define GPIOREN0_8          (1<<8)                      // Set Rising Edge Detect on GPIO pin 8
#define GPIOREN0_7          (1<<7)                      // Set Rising Edge Detect on GPIO pin 7
#define GPIOREN0_6          (1<<6)                      // Set Rising Edge Detect on GPIO pin 6
#define GPIOREN0_5          (1<<5)                      // Set Rising Edge Detect on GPIO pin 5
#define GPIOREN0_4          (1<<4)                      // Set Rising Edge Detect on GPIO pin 4
#define GPIOREN0_3          (1<<3)                      // Set Rising Edge Detect on GPIO pin 3
#define GPIOREN0_2          (1<<2)                      // Set Rising Edge Detect on GPIO pin 2
#define GPIOREN0_1          (1<<1)                      // Set Rising Edge Detect on GPIO pin 1
#define GPIOREN0_0          (1<<0)                      // Set Rising Edge Detect on GPIO pin 0


#define GPIO_REN1           (GPIO_BASE+0x50)            // GPIO Pin Rising Edge Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOREN1_53         (1<<21)                     // Set Rising Edge Detect on GPIO pin 53
#define GPIOREN1_52         (1<<20)                     // Set Rising Edge Detect on GPIO pin 52
#define GPIOREN1_51         (1<<19)                     // Set Rising Edge Detect on GPIO pin 51
#define GPIOREN1_50         (1<<18)                     // Set Rising Edge Detect on GPIO pin 50
#define GPIOREN1_49         (1<<17)                     // Set Rising Edge Detect on GPIO pin 49
#define GPIOREN1_48         (1<<16)                     // Set Rising Edge Detect on GPIO pin 48
#define GPIOREN1_47         (1<<15)                     // Set Rising Edge Detect on GPIO pin 47
#define GPIOREN1_46         (1<<14)                     // Set Rising Edge Detect on GPIO pin 46
#define GPIOREN1_45         (1<<13)                     // Set Rising Edge Detect on GPIO pin 45
#define GPIOREN1_44         (1<<12)                     // Set Rising Edge Detect on GPIO pin 44
#define GPIOREN1_43         (1<<11)                     // Set Rising Edge Detect on GPIO pin 43
#define GPIOREN1_42         (1<<10)                     // Set Rising Edge Detect on GPIO pin 42
#define GPIOREN1_41         (1<<9)                      // Set Rising Edge Detect on GPIO pin 41
#define GPIOREN1_40         (1<<8)                      // Set Rising Edge Detect on GPIO pin 40
#define GPIOREN1_39         (1<<7)                      // Set Rising Edge Detect on GPIO pin 39
#define GPIOREN1_38         (1<<6)                      // Set Rising Edge Detect on GPIO pin 38
#define GPIOREN1_37         (1<<5)                      // Set Rising Edge Detect on GPIO pin 37
#define GPIOREN1_36         (1<<4)                      // Set Rising Edge Detect on GPIO pin 36
#define GPIOREN1_35         (1<<3)                      // Set Rising Edge Detect on GPIO pin 35
#define GPIOREN1_34         (1<<2)                      // Set Rising Edge Detect on GPIO pin 34
#define GPIOREN1_33         (1<<1)                      // Set Rising Edge Detect on GPIO pin 33
#define GPIOREN1_32         (1<<0)                      // Set Rising Edge Detect on GPIO pin 32


#define GPIO_FEN0           (GPIO_BASE+0x58)            // GPIO Pin Falling Edge Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFEN0_31         (1<<31)                     // Set Falling Edge Detect on GPIO pin 31
#define GPIOFEN0_30         (1<<30)                     // Set Falling Edge Detect on GPIO pin 30
#define GPIOFEN0_29         (1<<29)                     // Set Falling Edge Detect on GPIO pin 29
#define GPIOFEN0_28         (1<<28)                     // Set Falling Edge Detect on GPIO pin 28
#define GPIOFEN0_27         (1<<27)                     // Set Falling Edge Detect on GPIO pin 27
#define GPIOFEN0_26         (1<<26)                     // Set Falling Edge Detect on GPIO pin 26
#define GPIOFEN0_25         (1<<25)                     // Set Falling Edge Detect on GPIO pin 25
#define GPIOFEN0_24         (1<<24)                     // Set Falling Edge Detect on GPIO pin 24
#define GPIOFEN0_23         (1<<23)                     // Set Falling Edge Detect on GPIO pin 23
#define GPIOFEN0_22         (1<<22)                     // Set Falling Edge Detect on GPIO pin 22
#define GPIOFEN0_21         (1<<21)                     // Set Falling Edge Detect on GPIO pin 21
#define GPIOFEN0_20         (1<<20)                     // Set Falling Edge Detect on GPIO pin 20
#define GPIOFEN0_19         (1<<19)                     // Set Falling Edge Detect on GPIO pin 19
#define GPIOFEN0_18         (1<<18)                     // Set Falling Edge Detect on GPIO pin 18
#define GPIOFEN0_17         (1<<17)                     // Set Falling Edge Detect on GPIO pin 17
#define GPIOFEN0_16         (1<<16)                     // Set Falling Edge Detect on GPIO pin 16
#define GPIOFEN0_15         (1<<15)                     // Set Falling Edge Detect on GPIO pin 15
#define GPIOFEN0_14         (1<<14)                     // Set Falling Edge Detect on GPIO pin 14
#define GPIOFEN0_13         (1<<13)                     // Set Falling Edge Detect on GPIO pin 13
#define GPIOFEN0_12         (1<<12)                     // Set Falling Edge Detect on GPIO pin 12
#define GPIOFEN0_11         (1<<11)                     // Set Falling Edge Detect on GPIO pin 11
#define GPIOFEN0_10         (1<<10)                     // Set Falling Edge Detect on GPIO pin 10
#define GPIOFEN0_9          (1<<9)                      // Set Falling Edge Detect on GPIO pin 9
#define GPIOFEN0_8          (1<<8)                      // Set Falling Edge Detect on GPIO pin 8
#define GPIOFEN0_7          (1<<7)                      // Set Falling Edge Detect on GPIO pin 7
#define GPIOFEN0_6          (1<<6)                      // Set Falling Edge Detect on GPIO pin 6
#define GPIOFEN0_5          (1<<5)                      // Set Falling Edge Detect on GPIO pin 5
#define GPIOFEN0_4          (1<<4)                      // Set Falling Edge Detect on GPIO pin 4
#define GPIOFEN0_3          (1<<3)                      // Set Falling Edge Detect on GPIO pin 3
#define GPIOFEN0_2          (1<<2)                      // Set Falling Edge Detect on GPIO pin 2
#define GPIOFEN0_1          (1<<1)                      // Set Falling Edge Detect on GPIO pin 1
#define GPIOFEN0_0          (1<<0)                      // Set Falling Edge Detect on GPIO pin 0


#define GPIO_FEN1           (GPIO_BASE+0x5c)            // GPIO Pin Falling Edge Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFEN1_53         (1<<21)                     // Set Falling Edge Detect on GPIO pin 53
#define GPIOFEN1_52         (1<<20)                     // Set Falling Edge Detect on GPIO pin 52
#define GPIOFEN1_51         (1<<19)                     // Set Falling Edge Detect on GPIO pin 51
#define GPIOFEN1_50         (1<<18)                     // Set Falling Edge Detect on GPIO pin 50
#define GPIOFEN1_49         (1<<17)                     // Set Falling Edge Detect on GPIO pin 49
#define GPIOFEN1_48         (1<<16)                     // Set Falling Edge Detect on GPIO pin 48
#define GPIOFEN1_47         (1<<15)                     // Set Falling Edge Detect on GPIO pin 47
#define GPIOFEN1_46         (1<<14)                     // Set Falling Edge Detect on GPIO pin 46
#define GPIOFEN1_45         (1<<13)                     // Set Falling Edge Detect on GPIO pin 45
#define GPIOFEN1_44         (1<<12)                     // Set Falling Edge Detect on GPIO pin 44
#define GPIOFEN1_43         (1<<11)                     // Set Falling Edge Detect on GPIO pin 43
#define GPIOFEN1_42         (1<<10)                     // Set Falling Edge Detect on GPIO pin 42
#define GPIOFEN1_41         (1<<9)                      // Set Falling Edge Detect on GPIO pin 41
#define GPIOFEN1_40         (1<<8)                      // Set Falling Edge Detect on GPIO pin 40
#define GPIOFEN1_39         (1<<7)                      // Set Falling Edge Detect on GPIO pin 39
#define GPIOFEN1_38         (1<<6)                      // Set Falling Edge Detect on GPIO pin 38
#define GPIOFEN1_37         (1<<5)                      // Set Falling Edge Detect on GPIO pin 37
#define GPIOFEN1_36         (1<<4)                      // Set Falling Edge Detect on GPIO pin 36
#define GPIOFEN1_35         (1<<3)                      // Set Falling Edge Detect on GPIO pin 35
#define GPIOFEN1_34         (1<<2)                      // Set Falling Edge Detect on GPIO pin 34
#define GPIOFEN1_33         (1<<1)                      // Set Falling Edge Detect on GPIO pin 33
#define GPIOFEN1_32         (1<<0)                      // Set Falling Edge Detect on GPIO pin 32


#define GPIO_HEN0           (GPIO_BASE+0x64)            // GPIO Pin High Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOHEN0_31         (1<<31)                     // Set High Detect on GPIO pin 31
#define GPIOHEN0_30         (1<<30)                     // Set High Detect on GPIO pin 30
#define GPIOHEN0_29         (1<<29)                     // Set High Detect on GPIO pin 29
#define GPIOHEN0_28         (1<<28)                     // Set High Detect on GPIO pin 28
#define GPIOHEN0_27         (1<<27)                     // Set High Detect on GPIO pin 27
#define GPIOHEN0_26         (1<<26)                     // Set High Detect on GPIO pin 26
#define GPIOHEN0_25         (1<<25)                     // Set High Detect on GPIO pin 25
#define GPIOHEN0_24         (1<<24)                     // Set High Detect on GPIO pin 24
#define GPIOHEN0_23         (1<<23)                     // Set High Detect on GPIO pin 23
#define GPIOHEN0_22         (1<<22)                     // Set High Detect on GPIO pin 22
#define GPIOHEN0_21         (1<<21)                     // Set High Detect on GPIO pin 21
#define GPIOHEN0_20         (1<<20)                     // Set High Detect on GPIO pin 20
#define GPIOHEN0_19         (1<<19)                     // Set High Detect on GPIO pin 19
#define GPIOHEN0_18         (1<<18)                     // Set High Detect on GPIO pin 18
#define GPIOHEN0_17         (1<<17)                     // Set High Detect on GPIO pin 17
#define GPIOHEN0_16         (1<<16)                     // Set High Detect on GPIO pin 16
#define GPIOHEN0_15         (1<<15)                     // Set High Detect on GPIO pin 15
#define GPIOHEN0_14         (1<<14)                     // Set High Detect on GPIO pin 14
#define GPIOHEN0_13         (1<<13)                     // Set High Detect on GPIO pin 13
#define GPIOHEN0_12         (1<<12)                     // Set High Detect on GPIO pin 12
#define GPIOHEN0_11         (1<<11)                     // Set High Detect on GPIO pin 11
#define GPIOHEN0_10         (1<<10)                     // Set High Detect on GPIO pin 10
#define GPIOHEN0_9          (1<<9)                      // Set High Detect on GPIO pin 9
#define GPIOHEN0_8          (1<<8)                      // Set High Detect on GPIO pin 8
#define GPIOHEN0_7          (1<<7)                      // Set High Detect on GPIO pin 7
#define GPIOHEN0_6          (1<<6)                      // Set High Detect on GPIO pin 6
#define GPIOHEN0_5          (1<<5)                      // Set High Detect on GPIO pin 5
#define GPIOHEN0_4          (1<<4)                      // Set High Detect on GPIO pin 4
#define GPIOHEN0_3          (1<<3)                      // Set High Detect on GPIO pin 3
#define GPIOHEN0_2          (1<<2)                      // Set High Detect on GPIO pin 2
#define GPIOHEN0_1          (1<<1)                      // Set High Detect on GPIO pin 1
#define GPIOHEN0_0          (1<<0)                      // Set High Detect on GPIO pin 0


#define GPIO_HEN1           (GPIO_BASE+0x68)            // GPIO Pin High Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOHEN1_53         (1<<21)                     // Set High Detect on GPIO pin 53
#define GPIOHEN1_52         (1<<20)                     // Set High Detect on GPIO pin 52
#define GPIOHEN1_51         (1<<19)                     // Set High Detect on GPIO pin 51
#define GPIOHEN1_50         (1<<18)                     // Set High Detect on GPIO pin 50
#define GPIOHEN1_49         (1<<17)                     // Set High Detect on GPIO pin 49
#define GPIOHEN1_48         (1<<16)                     // Set High Detect on GPIO pin 48
#define GPIOHEN1_47         (1<<15)                     // Set High Detect on GPIO pin 47
#define GPIOHEN1_46         (1<<14)                     // Set High Detect on GPIO pin 46
#define GPIOHEN1_45         (1<<13)                     // Set High Detect on GPIO pin 45
#define GPIOHEN1_44         (1<<12)                     // Set High Detect on GPIO pin 44
#define GPIOHEN1_43         (1<<11)                     // Set High Detect on GPIO pin 43
#define GPIOHEN1_42         (1<<10)                     // Set High Detect on GPIO pin 42
#define GPIOHEN1_41         (1<<9)                      // Set High Detect on GPIO pin 41
#define GPIOHEN1_40         (1<<8)                      // Set High Detect on GPIO pin 40
#define GPIOHEN1_39         (1<<7)                      // Set High Detect on GPIO pin 39
#define GPIOHEN1_38         (1<<6)                      // Set High Detect on GPIO pin 38
#define GPIOHEN1_37         (1<<5)                      // Set High Detect on GPIO pin 37
#define GPIOHEN1_36         (1<<4)                      // Set High Detect on GPIO pin 36
#define GPIOHEN1_35         (1<<3)                      // Set High Detect on GPIO pin 35
#define GPIOHEN1_34         (1<<2)                      // Set High Detect on GPIO pin 34
#define GPIOHEN1_33         (1<<1)                      // Set High Detect on GPIO pin 33
#define GPIOHEN1_32         (1<<0)                      // Set High Detect on GPIO pin 32


#define GPIO_LEN0           (GPIO_BASE+0x70)            // GPIO Pin Low Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOLEN0_31         (1<<31)                     // Set Low Detect on GPIO pin 31
#define GPIOLEN0_30         (1<<30)                     // Set Low Detect on GPIO pin 30
#define GPIOLEN0_29         (1<<29)                     // Set Low Detect on GPIO pin 29
#define GPIOLEN0_28         (1<<28)                     // Set Low Detect on GPIO pin 28
#define GPIOLEN0_27         (1<<27)                     // Set Low Detect on GPIO pin 27
#define GPIOLEN0_26         (1<<26)                     // Set Low Detect on GPIO pin 26
#define GPIOLEN0_25         (1<<25)                     // Set Low Detect on GPIO pin 25
#define GPIOLEN0_24         (1<<24)                     // Set Low Detect on GPIO pin 24
#define GPIOLEN0_23         (1<<23)                     // Set Low Detect on GPIO pin 23
#define GPIOLEN0_22         (1<<22)                     // Set Low Detect on GPIO pin 22
#define GPIOLEN0_21         (1<<21)                     // Set Low Detect on GPIO pin 21
#define GPIOLEN0_20         (1<<20)                     // Set Low Detect on GPIO pin 20
#define GPIOLEN0_19         (1<<19)                     // Set Low Detect on GPIO pin 19
#define GPIOLEN0_18         (1<<18)                     // Set Low Detect on GPIO pin 18
#define GPIOLEN0_17         (1<<17)                     // Set Low Detect on GPIO pin 17
#define GPIOLEN0_16         (1<<16)                     // Set Low Detect on GPIO pin 16
#define GPIOLEN0_15         (1<<15)                     // Set Low Detect on GPIO pin 15
#define GPIOLEN0_14         (1<<14)                     // Set Low Detect on GPIO pin 14
#define GPIOLEN0_13         (1<<13)                     // Set Low Detect on GPIO pin 13
#define GPIOLEN0_12         (1<<12)                     // Set Low Detect on GPIO pin 12
#define GPIOLEN0_11         (1<<11)                     // Set Low Detect on GPIO pin 11
#define GPIOLEN0_10         (1<<10)                     // Set Low Detect on GPIO pin 10
#define GPIOLEN0_9          (1<<9)                      // Set Low Detect on GPIO pin 9
#define GPIOLEN0_8          (1<<8)                      // Set Low Detect on GPIO pin 8
#define GPIOLEN0_7          (1<<7)                      // Set Low Detect on GPIO pin 7
#define GPIOLEN0_6          (1<<6)                      // Set Low Detect on GPIO pin 6
#define GPIOLEN0_5          (1<<5)                      // Set Low Detect on GPIO pin 5
#define GPIOLEN0_4          (1<<4)                      // Set Low Detect on GPIO pin 4
#define GPIOLEN0_3          (1<<3)                      // Set Low Detect on GPIO pin 3
#define GPIOLEN0_2          (1<<2)                      // Set Low Detect on GPIO pin 2
#define GPIOLEN0_1          (1<<1)                      // Set Low Detect on GPIO pin 1
#define GPIOLEN0_0          (1<<0)                      // Set Low Detect on GPIO pin 0


#define GPIO_LEN1           (GPIO_BASE+0x74)            // GPIO Pin Low Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOLEN1_53         (1<<21)                     // Set Low Detect on GPIO pin 53
#define GPIOLEN1_52         (1<<20)                     // Set Low Detect on GPIO pin 52
#define GPIOLEN1_51         (1<<19)                     // Set Low Detect on GPIO pin 51
#define GPIOLEN1_50         (1<<18)                     // Set Low Detect on GPIO pin 50
#define GPIOLEN1_49         (1<<17)                     // Set Low Detect on GPIO pin 49
#define GPIOLEN1_48         (1<<16)                     // Set Low Detect on GPIO pin 48
#define GPIOLEN1_47         (1<<15)                     // Set Low Detect on GPIO pin 47
#define GPIOLEN1_46         (1<<14)                     // Set Low Detect on GPIO pin 46
#define GPIOLEN1_45         (1<<13)                     // Set Low Detect on GPIO pin 45
#define GPIOLEN1_44         (1<<12)                     // Set Low Detect on GPIO pin 44
#define GPIOLEN1_43         (1<<11)                     // Set Low Detect on GPIO pin 43
#define GPIOLEN1_42         (1<<10)                     // Set Low Detect on GPIO pin 42
#define GPIOLEN1_41         (1<<9)                      // Set Low Detect on GPIO pin 41
#define GPIOLEN1_40         (1<<8)                      // Set Low Detect on GPIO pin 40
#define GPIOLEN1_39         (1<<7)                      // Set Low Detect on GPIO pin 39
#define GPIOLEN1_38         (1<<6)                      // Set Low Detect on GPIO pin 38
#define GPIOLEN1_37         (1<<5)                      // Set Low Detect on GPIO pin 37
#define GPIOLEN1_36         (1<<4)                      // Set Low Detect on GPIO pin 36
#define GPIOLEN1_35         (1<<3)                      // Set Low Detect on GPIO pin 35
#define GPIOLEN1_34         (1<<2)                      // Set Low Detect on GPIO pin 34
#define GPIOLEN1_33         (1<<1)                      // Set Low Detect on GPIO pin 33
#define GPIOLEN1_32         (1<<0)                      // Set Low Detect on GPIO pin 32


#define GPIO_AREN0          (GPIO_BASE+0x7c)            // GPIO Pin Async Rising Edge Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOAREN0_31        (1<<31)                     // Set Async Rising Edge Detect on GPIO pin 31
#define GPIOAREN0_30        (1<<30)                     // Set Async Rising Edge Detect on GPIO pin 30
#define GPIOAREN0_29        (1<<29)                     // Set Async Rising Edge Detect on GPIO pin 29
#define GPIOAREN0_28        (1<<28)                     // Set Async Rising Edge Detect on GPIO pin 28
#define GPIOAREN0_27        (1<<27)                     // Set Async Rising Edge Detect on GPIO pin 27
#define GPIOAREN0_26        (1<<26)                     // Set Async Rising Edge Detect on GPIO pin 26
#define GPIOAREN0_25        (1<<25)                     // Set Async Rising Edge Detect on GPIO pin 25
#define GPIOAREN0_24        (1<<24)                     // Set Async Rising Edge Detect on GPIO pin 24
#define GPIOAREN0_23        (1<<23)                     // Set Async Rising Edge Detect on GPIO pin 23
#define GPIOAREN0_22        (1<<22)                     // Set Async Rising Edge Detect on GPIO pin 22
#define GPIOAREN0_21        (1<<21)                     // Set Async Rising Edge Detect on GPIO pin 21
#define GPIOAREN0_20        (1<<20)                     // Set Async Rising Edge Detect on GPIO pin 20
#define GPIOAREN0_19        (1<<19)                     // Set Async Rising Edge Detect on GPIO pin 19
#define GPIOAREN0_18        (1<<18)                     // Set Async Rising Edge Detect on GPIO pin 18
#define GPIOAREN0_17        (1<<17)                     // Set Async Rising Edge Detect on GPIO pin 17
#define GPIOAREN0_16        (1<<16)                     // Set Async Rising Edge Detect on GPIO pin 16
#define GPIOAREN0_15        (1<<15)                     // Set Async Rising Edge Detect on GPIO pin 15
#define GPIOAREN0_14        (1<<14)                     // Set Async Rising Edge Detect on GPIO pin 14
#define GPIOAREN0_13        (1<<13)                     // Set Async Rising Edge Detect on GPIO pin 13
#define GPIOAREN0_12        (1<<12)                     // Set Async Rising Edge Detect on GPIO pin 12
#define GPIOAREN0_11        (1<<11)                     // Set Async Rising Edge Detect on GPIO pin 11
#define GPIOAREN0_10        (1<<10)                     // Set Async Rising Edge Detect on GPIO pin 10
#define GPIOAREN0_9         (1<<9)                      // Set Async Rising Edge Detect on GPIO pin 9
#define GPIOAREN0_8         (1<<8)                      // Set Async Rising Edge Detect on GPIO pin 8
#define GPIOAREN0_7         (1<<7)                      // Set Async Rising Edge Detect on GPIO pin 7
#define GPIOAREN0_6         (1<<6)                      // Set Async Rising Edge Detect on GPIO pin 6
#define GPIOAREN0_5         (1<<5)                      // Set Async Rising Edge Detect on GPIO pin 5
#define GPIOAREN0_4         (1<<4)                      // Set Async Rising Edge Detect on GPIO pin 4
#define GPIOAREN0_3         (1<<3)                      // Set Async Rising Edge Detect on GPIO pin 3
#define GPIOAREN0_2         (1<<2)                      // Set Async Rising Edge Detect on GPIO pin 2
#define GPIOAREN0_1         (1<<1)                      // Set Async Rising Edge Detect on GPIO pin 1
#define GPIOAREN0_0         (1<<0)                      // Set Async Rising Edge Detect on GPIO pin 0


#define GPIO_AREN1          (GPIO_BASE+0x80)            // GPIO Pin Async Rising Edge Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOAREN1_53        (1<<21)                     // Set Async Rising Edge Detect on GPIO pin 53
#define GPIOAREN1_52        (1<<20)                     // Set Async Rising Edge Detect on GPIO pin 52
#define GPIOAREN1_51        (1<<19)                     // Set Async Rising Edge Detect on GPIO pin 51
#define GPIOAREN1_50        (1<<18)                     // Set Async Rising Edge Detect on GPIO pin 50
#define GPIOAREN1_49        (1<<17)                     // Set Async Rising Edge Detect on GPIO pin 49
#define GPIOAREN1_48        (1<<16)                     // Set Async Rising Edge Detect on GPIO pin 48
#define GPIOAREN1_47        (1<<15)                     // Set Async Rising Edge Detect on GPIO pin 47
#define GPIOAREN1_46        (1<<14)                     // Set Async Rising Edge Detect on GPIO pin 46
#define GPIOAREN1_45        (1<<13)                     // Set Async Rising Edge Detect on GPIO pin 45
#define GPIOAREN1_44        (1<<12)                     // Set Async Rising Edge Detect on GPIO pin 44
#define GPIOAREN1_43        (1<<11)                     // Set Async Rising Edge Detect on GPIO pin 43
#define GPIOAREN1_42        (1<<10)                     // Set Async Rising Edge Detect on GPIO pin 42
#define GPIOAREN1_41        (1<<9)                      // Set Async Rising Edge Detect on GPIO pin 41
#define GPIOAREN1_40        (1<<8)                      // Set Async Rising Edge Detect on GPIO pin 40
#define GPIOAREN1_39        (1<<7)                      // Set Async Rising Edge Detect on GPIO pin 39
#define GPIOAREN1_38        (1<<6)                      // Set Async Rising Edge Detect on GPIO pin 38
#define GPIOAREN1_37        (1<<5)                      // Set Async Rising Edge Detect on GPIO pin 37
#define GPIOAREN1_36        (1<<4)                      // Set Async Rising Edge Detect on GPIO pin 36
#define GPIOAREN1_35        (1<<3)                      // Set Async Rising Edge Detect on GPIO pin 35
#define GPIOAREN1_34        (1<<2)                      // Set Async Rising Edge Detect on GPIO pin 34
#define GPIOAREN1_33        (1<<1)                      // Set Async Rising Edge Detect on GPIO pin 33
#define GPIOAREN1_32        (1<<0)                      // Set Async Rising Edge Detect on GPIO pin 32


#define GPIO_AFEN0          (GPIO_BASE+0x88)            // GPIO Pin Async Falling Edge Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOAFEN0_31        (1<<31)                     // Set Async Falling Edge Detect on GPIO pin 31
#define GPIOAFEN0_30        (1<<30)                     // Set Async Falling Edge Detect on GPIO pin 30
#define GPIOAFEN0_29        (1<<29)                     // Set Async Falling Edge Detect on GPIO pin 29
#define GPIOAFEN0_28        (1<<28)                     // Set Async Falling Edge Detect on GPIO pin 28
#define GPIOAFEN0_27        (1<<27)                     // Set Async Falling Edge Detect on GPIO pin 27
#define GPIOAFEN0_26        (1<<26)                     // Set Async Falling Edge Detect on GPIO pin 26
#define GPIOAFEN0_25        (1<<25)                     // Set Async Falling Edge Detect on GPIO pin 25
#define GPIOAFEN0_24        (1<<24)                     // Set Async Falling Edge Detect on GPIO pin 24
#define GPIOAFEN0_23        (1<<23)                     // Set Async Falling Edge Detect on GPIO pin 23
#define GPIOAFEN0_22        (1<<22)                     // Set Async Falling Edge Detect on GPIO pin 22
#define GPIOAFEN0_21        (1<<21)                     // Set Async Falling Edge Detect on GPIO pin 21
#define GPIOAFEN0_20        (1<<20)                     // Set Async Falling Edge Detect on GPIO pin 20
#define GPIOAFEN0_19        (1<<19)                     // Set Async Falling Edge Detect on GPIO pin 19
#define GPIOAFEN0_18        (1<<18)                     // Set Async Falling Edge Detect on GPIO pin 18
#define GPIOAFEN0_17        (1<<17)                     // Set Async Falling Edge Detect on GPIO pin 17
#define GPIOAFEN0_16        (1<<16)                     // Set Async Falling Edge Detect on GPIO pin 16
#define GPIOAFEN0_15        (1<<15)                     // Set Async Falling Edge Detect on GPIO pin 15
#define GPIOAFEN0_14        (1<<14)                     // Set Async Falling Edge Detect on GPIO pin 14
#define GPIOAFEN0_13        (1<<13)                     // Set Async Falling Edge Detect on GPIO pin 13
#define GPIOAFEN0_12        (1<<12)                     // Set Async Falling Edge Detect on GPIO pin 12
#define GPIOAFEN0_11        (1<<11)                     // Set Async Falling Edge Detect on GPIO pin 11
#define GPIOAFEN0_10        (1<<10)                     // Set Async Falling Edge Detect on GPIO pin 10
#define GPIOAFEN0_9         (1<<9)                      // Set Async Falling Edge Detect on GPIO pin 9
#define GPIOAFEN0_8         (1<<8)                      // Set Async Falling Edge Detect on GPIO pin 8
#define GPIOAFEN0_7         (1<<7)                      // Set Async Falling Edge Detect on GPIO pin 7
#define GPIOAFEN0_6         (1<<6)                      // Set Async Falling Edge Detect on GPIO pin 6
#define GPIOAFEN0_5         (1<<5)                      // Set Async Falling Edge Detect on GPIO pin 5
#define GPIOAFEN0_4         (1<<4)                      // Set Async Falling Edge Detect on GPIO pin 4
#define GPIOAFEN0_3         (1<<3)                      // Set Async Falling Edge Detect on GPIO pin 3
#define GPIOAFEN0_2         (1<<2)                      // Set Async Falling Edge Detect on GPIO pin 2
#define GPIOAFEN0_1         (1<<1)                      // Set Async Falling Edge Detect on GPIO pin 1
#define GPIOAFEN0_0         (1<<0)                      // Set Async Falling Edge Detect on GPIO pin 0


#define GPIO_AFEN1          (GPIO_BASE+0x8c)            // GPIO Pin Async Falling Edge Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOAFEN1_53        (1<<21)                     // Set Async Falling Edge Detect on GPIO pin 53
#define GPIOAFEN1_52        (1<<20)                     // Set Async Falling Edge Detect on GPIO pin 52
#define GPIOAFEN1_51        (1<<19)                     // Set Async Falling Edge Detect on GPIO pin 51
#define GPIOAFEN1_50        (1<<18)                     // Set Async Falling Edge Detect on GPIO pin 50
#define GPIOAFEN1_49        (1<<17)                     // Set Async Falling Edge Detect on GPIO pin 49
#define GPIOAFEN1_48        (1<<16)                     // Set Async Falling Edge Detect on GPIO pin 48
#define GPIOAFEN1_47        (1<<15)                     // Set Async Falling Edge Detect on GPIO pin 47
#define GPIOAFEN1_46        (1<<14)                     // Set Async Falling Edge Detect on GPIO pin 46
#define GPIOAFEN1_45        (1<<13)                     // Set Async Falling Edge Detect on GPIO pin 45
#define GPIOAFEN1_44        (1<<12)                     // Set Async Falling Edge Detect on GPIO pin 44
#define GPIOAFEN1_43        (1<<11)                     // Set Async Falling Edge Detect on GPIO pin 43
#define GPIOAFEN1_42        (1<<10)                     // Set Async Falling Edge Detect on GPIO pin 42
#define GPIOAFEN1_41        (1<<9)                      // Set Async Falling Edge Detect on GPIO pin 41
#define GPIOAFEN1_40        (1<<8)                      // Set Async Falling Edge Detect on GPIO pin 40
#define GPIOAFEN1_39        (1<<7)                      // Set Async Falling Edge Detect on GPIO pin 39
#define GPIOAFEN1_38        (1<<6)                      // Set Async Falling Edge Detect on GPIO pin 38
#define GPIOAFEN1_37        (1<<5)                      // Set Async Falling Edge Detect on GPIO pin 37
#define GPIOAFEN1_36        (1<<4)                      // Set Async Falling Edge Detect on GPIO pin 36
#define GPIOAFEN1_35        (1<<3)                      // Set Async Falling Edge Detect on GPIO pin 35
#define GPIOAFEN1_34        (1<<2)                      // Set Async Falling Edge Detect on GPIO pin 34
#define GPIOAFEN1_33        (1<<1)                      // Set Async Falling Edge Detect on GPIO pin 33
#define GPIOAFEN1_32        (1<<0)                      // Set Async Falling Edge Detect on GPIO pin 32


#define GPIO_GPPUD          (GPIO_BASE+0x94)            // GPIO Pin Pull Up/Down Enable
//-------------------------------------------------------------------------------------------------------------------
#define GPIOPUD_OFF         (0b00)                      // Disable Pull Up/Down Control
#define GPIOPUD_DOWN        (0b01)                      // Enable Pull Down
#define GPIOPUD_UP          (0b10)                      // Enable Pull Up


#define GPIO_GPPUDCLK1      (GPIO_BASE+0x98)            // GPIO Pin Pull Up/Down Enable Clock 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOCLK1_31         (1<<31)                     // Assert Clock on GPIO pin 31
#define GPIOCLK1_30         (1<<30)                     // Assert Clock on GPIO pin 30
#define GPIOCLK1_29         (1<<29)                     // Assert Clock on GPIO pin 29
#define GPIOCLK1_28         (1<<28)                     // Assert Clock on GPIO pin 28
#define GPIOCLK1_27         (1<<27)                     // Assert Clock on GPIO pin 27
#define GPIOCLK1_26         (1<<26)                     // Assert Clock on GPIO pin 26
#define GPIOCLK1_25         (1<<25)                     // Assert Clock on GPIO pin 25
#define GPIOCLK1_24         (1<<24)                     // Assert Clock on GPIO pin 24
#define GPIOCLK1_23         (1<<23)                     // Assert Clock on GPIO pin 23
#define GPIOCLK1_22         (1<<22)                     // Assert Clock on GPIO pin 22
#define GPIOCLK1_21         (1<<21)                     // Assert Clock on GPIO pin 21
#define GPIOCLK1_20         (1<<20)                     // Assert Clock on GPIO pin 20
#define GPIOCLK1_19         (1<<19)                     // Assert Clock on GPIO pin 19
#define GPIOCLK1_18         (1<<18)                     // Assert Clock on GPIO pin 18
#define GPIOCLK1_17         (1<<17)                     // Assert Clock on GPIO pin 17
#define GPIOCLK1_16         (1<<16)                     // Assert Clock on GPIO pin 16
#define GPIOCLK1_15         (1<<15)                     // Assert Clock on GPIO pin 15
#define GPIOCLK1_14         (1<<14)                     // Assert Clock on GPIO pin 14
#define GPIOCLK1_13         (1<<13)                     // Assert Clock on GPIO pin 13
#define GPIOCLK1_12         (1<<12)                     // Assert Clock on GPIO pin 12
#define GPIOCLK1_11         (1<<11)                     // Assert Clock on GPIO pin 11
#define GPIOCLK1_10         (1<<10)                     // Assert Clock on GPIO pin 10
#define GPIOCLK1_9          (1<<9)                      // Assert Clock on GPIO pin 9
#define GPIOCLK1_8          (1<<8)                      // Assert Clock on GPIO pin 8
#define GPIOCLK1_7          (1<<7)                      // Assert Clock on GPIO pin 7
#define GPIOCLK1_6          (1<<6)                      // Assert Clock on GPIO pin 6
#define GPIOCLK1_5          (1<<5)                      // Assert Clock on GPIO pin 5
#define GPIOCLK1_4          (1<<4)                      // Assert Clock on GPIO pin 4
#define GPIOCLK1_3          (1<<3)                      // Assert Clock on GPIO pin 3
#define GPIOCLK1_2          (1<<2)                      // Assert Clock on GPIO pin 2
#define GPIOCLK1_1          (1<<1)                      // Assert Clock on GPIO pin 1
#define GPIOCLK1_0          (1<<0)                      // Assert Clock on GPIO pin 0


#define GPIO_GPPUDCLK2      (GPIO_BASE+0x9c)            // GPIO Pin Pull Up/Down Enable CLock 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOCLK2_53         (1<<21)                     // Assert Clock on GPIO pin 53
#define GPIOCLK2_52         (1<<20)                     // Assert Clock on GPIO pin 52
#define GPIOCLK2_51         (1<<19)                     // Assert Clock on GPIO pin 51
#define GPIOCLK2_50         (1<<18)                     // Assert Clock on GPIO pin 50
#define GPIOCLK2_49         (1<<17)                     // Assert Clock on GPIO pin 49
#define GPIOCLK2_48         (1<<16)                     // Assert Clock on GPIO pin 48
#define GPIOCLK2_47         (1<<15)                     // Assert Clock on GPIO pin 47
#define GPIOCLK2_46         (1<<14)                     // Assert Clock on GPIO pin 46
#define GPIOCLK2_45         (1<<13)                     // Assert Clock on GPIO pin 45
#define GPIOCLK2_44         (1<<12)                     // Assert Clock on GPIO pin 44
#define GPIOCLK2_43         (1<<11)                     // Assert Clock on GPIO pin 43
#define GPIOCLK2_42         (1<<10)                     // Assert Clock on GPIO pin 42
#define GPIOCLK2_41         (1<<9)                      // Assert Clock on GPIO pin 41
#define GPIOCLK2_40         (1<<8)                      // Assert Clock on GPIO pin 40
#define GPIOCLK2_39         (1<<7)                      // Assert Clock on GPIO pin 39
#define GPIOCLK2_38         (1<<6)                      // Assert Clock on GPIO pin 38
#define GPIOCLK2_37         (1<<5)                      // Assert Clock on GPIO pin 37
#define GPIOCLK2_36         (1<<4)                      // Assert Clock on GPIO pin 36
#define GPIOCLK2_35         (1<<3)                      // Assert Clock on GPIO pin 35
#define GPIOCLK2_34         (1<<2)                      // Assert Clock on GPIO pin 34
#define GPIOCLK2_33         (1<<1)                      // Assert Clock on GPIO pin 33
#define GPIOCLK2_32         (1<<0)                      // Assert Clock on GPIO pin 32


//-------------------------------------------------------------------------------------------------------------------
// General Purpose Clocks
//-------------------------------------------------------------------------------------------------------------------


//
// -- Different clock sources
//    -----------------------
#define CMSRC_GND0          (0)                         // Ground
#define CMSRC_OSC           (1)                         // Oscilator
#define CMSRC_TESTDBG0      (2)                         // Test Debug 0
#define CMSRC_TESTDBG1      (3)                         // Test Debug 1
#define CMSRC_PLLA          (4)                         // PLLA per
#define CMSRC_PLLC          (5)                         // PLLC per
#define CMSRC_PLLD          (6)                         // PLLD per
#define CMSRC_HDMI          (7)                         // HDMI Auxiliary
#define CMSRC_GND8          (8)                         // Ground
#define CMSRC_GND9          (9)                         // Ground
#define CMSRC_GNDA          (10)                        // Ground
#define CMSRC_GNDB          (11)                        // Ground
#define CMSRC_GNDC          (12)                        // Ground
#define CMSRC_GNDD          (13)                        // Ground
#define CMSRC_GNDE          (14)                        // Ground
#define CMSRC_GNDF          (15)                        // Ground


#define SH_CMGPPWD          (0x5a<<24)                  // This is a safety value


//
// -- The Clock Manager 0 Control
//    ---------------------------
#define CMGP0_CTL           (HW_BASE+0x101070)
//-------------------------------------------------------------------------------------------------------------------
#define CM0CTL_MASH         (3<<9)                      // MASH control
#define CM0CTL_FLIP         (1<<8)                      // Invert output
#define CM0CTL_BUSY         (1<<7)                      // Clock generator is running
#define CM0CTL_KILL         (1<<5)                      // kill teh clock generator
#define CM0CTL_ENAB         (1<<4)                      // enable the clock generator
#define CM0CTL_SRC          (0xf)                       // The Clock Source

#define SH_CMGP0MASHNONE    (0<<9)                      // integer division
#define SH_CMGP0MASH1       (1<<9)                      // 1-stage MASH (same as non-MASH)
#define SH_CMGP0MASH2       (2<<9)                      // 2-stage MASH
#define SH_CMGP0MASH3       (3<<9)                      // 3-stage MASH


#define CMGP0_DIV           (HW_BASE+0x101074)
//-------------------------------------------------------------------------------------------------------------------
#define CM0DIV_DIVI         (0xfff<<12)                 // Integer part of divisor
#define CM0DIV_DIVF         (0xfff<<0)                  // Fractional part of divisor

#define SH_CMGP0DIVI(x)     (((x)&0xfff)<<12)           // Shift the value to the correct bits


//
// -- The Clock Manager 1 Control
//    ---------------------------
#define CMGP1_CTL           (HW_BASE+0x101078)
//-------------------------------------------------------------------------------------------------------------------
#define CM1CTL_MASH         (3<<9)                      // MASH control
#define CM1CTL_FLIP         (1<<8)                      // Invert output
#define CM1CTL_BUSY         (1<<7)                      // Clock generator is running
#define CM1CTL_KILL         (1<<5)                      // kill teh clock generator
#define CM1CTL_ENAB         (1<<4)                      // enable the clock generator
#define CM1CTL_SRC          (0xf)                       // The Clock Source

#define SH_CMGP1MASHNONE    (0<<9)                      // integer division
#define SH_CMGP1MASH1       (1<<9)                      // 1-stage MASH (same as non-MASH)
#define SH_CMGP1MASH2       (2<<9)                      // 2-stage MASH
#define SH_CMGP1MASH3       (3<<9)                      // 3-stage MASH


#define CMGP1_DIV           (HW_BASE+0x10107c)
//-------------------------------------------------------------------------------------------------------------------
#define CM1DIV_DIVI         (0xfff<<12)                 // Integer part of divisor
#define CM1DIV_DIVF         (0xfff<<0)                  // Fractional part of divisor

#define SH_CMGP1DIVI(x)     (((x)&0xfff)<<12)           // Shift the value to the correct bits


//
// -- The Clock Manager 2 Control
//    ---------------------------
#define CMGP2_CTL           (HW_BASE+0x101080)
//-------------------------------------------------------------------------------------------------------------------
#define CM2CTL_MASH         (3<<9)                      // MASH control
#define CM2CTL_FLIP         (1<<8)                      // Invert output
#define CM2CTL_BUSY         (1<<7)                      // Clock generator is running
#define CM2CTL_KILL         (1<<5)                      // kill teh clock generator
#define CM2CTL_ENAB         (1<<4)                      // enable the clock generator
#define CM2CTL_SRC          (0xf)                       // The Clock Source

#define SH_CMGP2MASHNONE    (0<<9)                      // integer division
#define SH_CMGP2MASH1       (1<<9)                      // 1-stage MASH (same as non-MASH)
#define SH_CMGP2MASH2       (2<<9)                      // 2-stage MASH
#define SH_CMGP2MASH3       (3<<9)                      // 3-stage MASH


#define CMGP2_DIV           (HW_BASE+0x101084)
//-------------------------------------------------------------------------------------------------------------------
#define CM2DIV_DIVI         (0xfff<<12)                 // Integer part of divisor
#define CM2DIV_DIVF         (0xfff<<0)                  // Fractional part of divisor

#define SH_CMGP2DIVI(x)     (((x)&0xfff)<<12)           // Shift the value to the correct bits


//-------------------------------------------------------------------------------------------------------------------
// Interrupts
//-------------------------------------------------------------------------------------------------------------------


//
// -- Interrupt Registers
//    -------------------
#define INT_BASE            (HW_BASE+0x00b000)          // The base address for the interrupt registers


#define INT_IRQPEND0        (INT_BASE+0x200)            // The basic interrupt pending register
//-------------------------------------------------------------------------------------------------------------------
#define INTPND0IRQ62        (1<<20)                     // GPU IRQ 62
#define INTPND0IRQ57        (1<<19)                     // GPU IRQ 57
#define INTPND0IRQ56        (1<<18)                     // GPU IRQ 56
#define INTPND0IRQ55        (1<<17)                     // GPU IRQ 55
#define INTPND0IRQ54        (1<<16)                     // GPU IRQ 54
#define INTPND0IRQ53        (1<<15)                     // GPU IRQ 53
#define INTPND0IRQ19        (1<<14)                     // GPU IRQ 19
#define INTPND0IRQ18        (1<<13)                     // GPU IRQ 18
#define INTPND0IRQ10        (1<<12)                     // GPU IRQ 10
#define INTPND0IRQ9         (1<<11)                     // GPU IRQ 9
#define INTPND0IRQ7         (1<<10)                     // GPU IRQ 7
#define INTPND0IRQREG2      (1<<9)                      // Pending Register 0 IRQ
#define INTPND0IRQREG1      (1<<8)                      // Pending Register 1 IRQ
#define INTPND0IRQILL0      (1<<7)                      // Illegal Access type 0 IRQ
#define INTPND0IRQILL1      (1<<6)                      // Illegal Access type 1 IRQ
#define INTPND0IRQGPUH1     (1<<5)                      // GPU1 halted IRQ
#define INTPND0IRQGPUH0     (1<<4)                      // GPU0 halted IRQ
#define INTPND0IRQDOORB1    (1<<3)                      // ARM Doorbell 1
#define INTPND0IRQDOORB0    (1<<2)                      // ARM Doorbell 0
#define INTPND0IRQMAIL      (1<<1)                      // ARM Mailbox IRQ
#define INTPND0IRQTIMER     (1<<0)                      // ARM Timer IRQ


#define INT_IRQPEND1        (INT_BASE+0x204)            // IRQ pending 1
//-------------------------------------------------------------------------------------------------------------------
#define INTPND1IRQ31        (1<<31)                     // IRQ 31 pending
#define INTPND1IRQ30        (1<<30)                     // IRQ 30 pending
#define INTPND1IRQ29        (1<<29)                     // IRQ 29 pending
#define INTPND1IRQ28        (1<<28)                     // IRQ 28 pending
#define INTPND1IRQ27        (1<<27)                     // IRQ 27 pending
#define INTPND1IRQ26        (1<<26)                     // IRQ 26 pending
#define INTPND1IRQ25        (1<<25)                     // IRQ 25 pending
#define INTPND1IRQ24        (1<<24)                     // IRQ 24 pending
#define INTPND1IRQ23        (1<<23)                     // IRQ 23 pending
#define INTPND1IRQ22        (1<<22)                     // IRQ 22 pending
#define INTPND1IRQ21        (1<<21)                     // IRQ 21 pending
#define INTPND1IRQ20        (1<<20)                     // IRQ 20 pending
#define INTPND1IRQ19        (1<<19)                     // IRQ 19 pending
#define INTPND1IRQ18        (1<<18)                     // IRQ 18 pending
#define INTPND1IRQ17        (1<<17)                     // IRQ 17 pending
#define INTPND1IRQ16        (1<<16)                     // IRQ 16 pending
#define INTPND1IRQ15        (1<<15)                     // IRQ 15 pending
#define INTPND1IRQ14        (1<<14)                     // IRQ 14 pending
#define INTPND1IRQ13        (1<<13)                     // IRQ 13 pending
#define INTPND1IRQ12        (1<<12)                     // IRQ 12 pending
#define INTPND1IRQ11        (1<<11)                     // IRQ 11 pending
#define INTPND1IRQ10        (1<<10)                     // IRQ 10 pending
#define INTPND1IRQ9         (1<<9)                      // IRQ 9 pending
#define INTPND1IRQ8         (1<<8)                      // IRQ 8 pending
#define INTPND1IRQ7         (1<<7)                      // IRQ 7 pending
#define INTPND1IRQ6         (1<<6)                      // IRQ 6 pending
#define INTPND1IRQ5         (1<<5)                      // IRQ 5 pending
#define INTPND1IRQ4         (1<<4)                      // IRQ 4 pending
#define INTPND1IRQ3         (1<<3)                      // IRQ 3 pending
#define INTPND1IRQ2         (1<<2)                      // IRQ 2 pending
#define INTPND1IRQ1         (1<<1)                      // IRQ 1 pending
#define INTPND1IRQ0         (1<<0)                      // IRQ 0 pending


#define INT_IRQPEND2        (INT_BASE+0x208)            // IRQ pending 2
//-------------------------------------------------------------------------------------------------------------------
#define INTPND2IRQ63        (1<<31)                     // IRQ 63 pending
#define INTPND2IRQ62        (1<<30)                     // IRQ 62 pending
#define INTPND2IRQ61        (1<<29)                     // IRQ 61 pending
#define INTPND2IRQ60        (1<<28)                     // IRQ 60 pending
#define INTPND2IRQ59        (1<<27)                     // IRQ 59 pending
#define INTPND2IRQ58        (1<<26)                     // IRQ 58 pending
#define INTPND2IRQ57        (1<<25)                     // IRQ 57 pending
#define INTPND2IRQ56        (1<<24)                     // IRQ 56 pending
#define INTPND2IRQ55        (1<<23)                     // IRQ 55 pending
#define INTPND2IRQ54        (1<<22)                     // IRQ 54 pending
#define INTPND2IRQ53        (1<<21)                     // IRQ 53 pending
#define INTPND2IRQ52        (1<<20)                     // IRQ 52 pending
#define INTPND2IRQ51        (1<<19)                     // IRQ 51 pending
#define INTPND2IRQ50        (1<<18)                     // IRQ 50 pending
#define INTPND2IRQ49        (1<<17)                     // IRQ 49 pending
#define INTPND2IRQ48        (1<<16)                     // IRQ 48 pending
#define INTPND2IRQ47        (1<<15)                     // IRQ 47 pending
#define INTPND2IRQ46        (1<<14)                     // IRQ 46 pending
#define INTPND2IRQ45        (1<<13)                     // IRQ 45 pending
#define INTPND2IRQ44        (1<<12)                     // IRQ 44 pending
#define INTPND2IRQ43        (1<<11)                     // IRQ 43 pending
#define INTPND2IRQ42        (1<<10)                     // IRQ 42 pending
#define INTPND2IRQ41        (1<<9)                      // IRQ 41 pending
#define INTPND2IRQ40        (1<<8)                      // IRQ 40 pending
#define INTPND2IRQ39        (1<<7)                      // IRQ 39 pending
#define INTPND2IRQ38        (1<<6)                      // IRQ 38 pending
#define INTPND2IRQ37        (1<<5)                      // IRQ 37 pending
#define INTPND2IRQ36        (1<<4)                      // IRQ 36 pending
#define INTPND2IRQ35        (1<<3)                      // IRQ 35 pending
#define INTPND2IRQ34        (1<<2)                      // IRQ 34 pending
#define INTPND2IRQ33        (1<<1)                      // IRQ 33 pending
#define INTPND2IRQ32        (1<<0)                      // IRQ 32 pending


#define INT_FIQCTL          (INT_BASE+0x20c)            // FIQ Control
//-------------------------------------------------------------------------------------------------------------------
#define INTFIQ_ENB          (1<<7)                      // FIQ enable
#define INTFIQ_SRC          (0x7f)                      // FIQ Source


#define INT_IRQENB1         (INT_BASE+0x210)            // IRQ Enable 1
//-------------------------------------------------------------------------------------------------------------------
#define INTENB1IRQ31        (1<<31)                     // IRQ 31 Enable
#define INTENB1IRQ30        (1<<30)                     // IRQ 30 Enable
#define INTENB1IRQ29        (1<<29)                     // IRQ 29 Enable
#define INTENB1IRQ28        (1<<28)                     // IRQ 28 Enable
#define INTENB1IRQ27        (1<<27)                     // IRQ 27 Enable
#define INTENB1IRQ26        (1<<26)                     // IRQ 26 Enable
#define INTENB1IRQ25        (1<<25)                     // IRQ 25 Enable
#define INTENB1IRQ24        (1<<24)                     // IRQ 24 Enable
#define INTENB1IRQ23        (1<<23)                     // IRQ 23 Enable
#define INTENB1IRQ22        (1<<22)                     // IRQ 22 Enable
#define INTENB1IRQ21        (1<<21)                     // IRQ 21 Enable
#define INTENB1IRQ20        (1<<20)                     // IRQ 20 Enable
#define INTENB1IRQ19        (1<<19)                     // IRQ 19 Enable
#define INTENB1IRQ18        (1<<18)                     // IRQ 18 Enable
#define INTENB1IRQ17        (1<<17)                     // IRQ 17 Enable
#define INTENB1IRQ16        (1<<16)                     // IRQ 16 Enable
#define INTENB1IRQ15        (1<<15)                     // IRQ 15 Enable
#define INTENB1IRQ14        (1<<14)                     // IRQ 14 Enable
#define INTENB1IRQ13        (1<<13)                     // IRQ 13 Enable
#define INTENB1IRQ12        (1<<12)                     // IRQ 12 Enable
#define INTENB1IRQ11        (1<<11)                     // IRQ 11 Enable
#define INTENB1IRQ10        (1<<10)                     // IRQ 10 Enable
#define INTENB1IRQ9         (1<<9)                      // IRQ 9 Enable
#define INTENB1IRQ8         (1<<8)                      // IRQ 8 Enable
#define INTENB1IRQ7         (1<<7)                      // IRQ 7 Enable
#define INTENB1IRQ6         (1<<6)                      // IRQ 6 Enable
#define INTENB1IRQ5         (1<<5)                      // IRQ 5 Enable
#define INTENB1IRQ4         (1<<4)                      // IRQ 4 Enable
#define INTENB1IRQ3         (1<<3)                      // IRQ 3 Enable
#define INTENB1IRQ2         (1<<2)                      // IRQ 2 Enable
#define INTENB1IRQ1         (1<<1)                      // IRQ 1 Enable
#define INTENB1IRQ0         (1<<0)                      // IRQ 0 Enable


#define INT_IRQENB2         (INT_BASE+0x214)            // IRQ Enable 2
//-------------------------------------------------------------------------------------------------------------------
#define INTENB2IRQ63        (1<<31)                     // IRQ 63 Enable
#define INTENB2IRQ62        (1<<30)                     // IRQ 62 Enable
#define INTENB2IRQ61        (1<<29)                     // IRQ 61 Enable
#define INTENB2IRQ60        (1<<28)                     // IRQ 60 Enable
#define INTENB2IRQ59        (1<<27)                     // IRQ 59 Enable
#define INTENB2IRQ58        (1<<26)                     // IRQ 58 Enable
#define INTENB2IRQ57        (1<<25)                     // IRQ 57 Enable
#define INTENB2IRQ56        (1<<24)                     // IRQ 56 Enable
#define INTENB2IRQ55        (1<<23)                     // IRQ 55 Enable
#define INTENB2IRQ54        (1<<22)                     // IRQ 54 Enable
#define INTENB2IRQ53        (1<<21)                     // IRQ 53 Enable
#define INTENB2IRQ52        (1<<20)                     // IRQ 52 Enable
#define INTENB2IRQ51        (1<<19)                     // IRQ 51 Enable
#define INTENB2IRQ50        (1<<18)                     // IRQ 50 Enable
#define INTENB2IRQ49        (1<<17)                     // IRQ 49 Enable
#define INTENB2IRQ48        (1<<16)                     // IRQ 48 Enable
#define INTENB2IRQ47        (1<<15)                     // IRQ 47 Enable
#define INTENB2IRQ46        (1<<14)                     // IRQ 46 Enable
#define INTENB2IRQ45        (1<<13)                     // IRQ 45 Enable
#define INTENB2IRQ44        (1<<12)                     // IRQ 44 Enable
#define INTENB2IRQ43        (1<<11)                     // IRQ 43 Enable
#define INTENB2IRQ42        (1<<10)                     // IRQ 42 Enable
#define INTENB2IRQ41        (1<<9)                      // IRQ 41 Enable
#define INTENB2IRQ40        (1<<8)                      // IRQ 40 Enable
#define INTENB2IRQ39        (1<<7)                      // IRQ 39 Enable
#define INTENB2IRQ38        (1<<6)                      // IRQ 38 Enable
#define INTENB2IRQ37        (1<<5)                      // IRQ 37 Enable
#define INTENB2IRQ36        (1<<4)                      // IRQ 36 Enable
#define INTENB2IRQ35        (1<<3)                      // IRQ 35 Enable
#define INTENB2IRQ34        (1<<2)                      // IRQ 34 Enable
#define INTENB2IRQ33        (1<<1)                      // IRQ 33 Enable
#define INTENB2IRQ32        (1<<0)                      // IRQ 32 Enable


#define INT_IRQENB0         (INT_BASE+0x218)            // Basic IRQ Enable
//-------------------------------------------------------------------------------------------------------------------
#define INTENB0IRQILL0      (1<<7)                      // Illegal Access type 0 IRQ Enable
#define INTENB0IRQILL1      (1<<6)                      // Illegal Access type 1 IRQ Enable
#define INTENB0IRQGPUH1     (1<<5)                      // GPU1 halted IRQ Enable
#define INTENB0IRQGPUH0     (1<<4)                      // GPU0 halted IRQ Enable
#define INTENB0IRQDOORB1    (1<<3)                      // ARM Doorbell 1 Enable
#define INTENB0IRQDOORB0    (1<<2)                      // ARM Doorbell 0 Enable
#define INTENB0IRQMAIL      (1<<1)                      // ARM Mailbox IRQ Enable
#define INTENB0IRQTIMER     (1<<0)                      // ARM Timer IRQ Enable


#define INT_IRQDIS1         (INT_BASE+0x21c)            // IRQ Disable 1
//-------------------------------------------------------------------------------------------------------------------
#define INTDIS1IRQ31        (1<<31)                     // IRQ 31 Disable
#define INTDIS1IRQ30        (1<<30)                     // IRQ 30 Disable
#define INTDIS1IRQ29        (1<<29)                     // IRQ 29 Disable
#define INTDIS1IRQ28        (1<<28)                     // IRQ 28 Disable
#define INTDIS1IRQ27        (1<<27)                     // IRQ 27 Disable
#define INTDIS1IRQ26        (1<<26)                     // IRQ 26 Disable
#define INTDIS1IRQ25        (1<<25)                     // IRQ 25 Disable
#define INTDIS1IRQ24        (1<<24)                     // IRQ 24 Disable
#define INTDIS1IRQ23        (1<<23)                     // IRQ 23 Disable
#define INTDIS1IRQ22        (1<<22)                     // IRQ 22 Disable
#define INTDIS1IRQ21        (1<<21)                     // IRQ 21 Disable
#define INTDIS1IRQ20        (1<<20)                     // IRQ 20 Disable
#define INTDIS1IRQ19        (1<<19)                     // IRQ 19 Disable
#define INTDIS1IRQ18        (1<<18)                     // IRQ 18 Disable
#define INTDIS1IRQ17        (1<<17)                     // IRQ 17 Disable
#define INTDIS1IRQ16        (1<<16)                     // IRQ 16 Disable
#define INTDIS1IRQ15        (1<<15)                     // IRQ 15 Disable
#define INTDIS1IRQ14        (1<<14)                     // IRQ 14 Disable
#define INTDIS1IRQ13        (1<<13)                     // IRQ 13 Disable
#define INTDIS1IRQ12        (1<<12)                     // IRQ 12 Disable
#define INTDIS1IRQ11        (1<<11)                     // IRQ 11 Disable
#define INTDIS1IRQ10        (1<<10)                     // IRQ 10 Disable
#define INTDIS1IRQ9         (1<<9)                      // IRQ 9 Disable
#define INTDIS1IRQ8         (1<<8)                      // IRQ 8 Disable
#define INTDIS1IRQ7         (1<<7)                      // IRQ 7 Disable
#define INTDIS1IRQ6         (1<<6)                      // IRQ 6 Disable
#define INTDIS1IRQ5         (1<<5)                      // IRQ 5 Disable
#define INTDIS1IRQ4         (1<<4)                      // IRQ 4 Disable
#define INTDIS1IRQ3         (1<<3)                      // IRQ 3 Disable
#define INTDIS1IRQ2         (1<<2)                      // IRQ 2 Disable
#define INTDIS1IRQ1         (1<<1)                      // IRQ 1 Disable
#define INTDIS1IRQ0         (1<<0)                      // IRQ 0 Disable


#define INT_IRQDIS2         (INT_BASE+0x220)            // IRQ Disable 2
//-------------------------------------------------------------------------------------------------------------------
#define INTDIS2IRQ63        (1<<31)                     // IRQ 63 Disable
#define INTDIS2IRQ62        (1<<30)                     // IRQ 62 Disable
#define INTDIS2IRQ61        (1<<29)                     // IRQ 61 Disable
#define INTDIS2IRQ60        (1<<28)                     // IRQ 60 Disable
#define INTDIS2IRQ59        (1<<27)                     // IRQ 59 Disable
#define INTDIS2IRQ58        (1<<26)                     // IRQ 58 Disable
#define INTDIS2IRQ57        (1<<25)                     // IRQ 57 Disable
#define INTDIS2IRQ56        (1<<24)                     // IRQ 56 Disable
#define INTDIS2IRQ55        (1<<23)                     // IRQ 55 Disable
#define INTDIS2IRQ54        (1<<22)                     // IRQ 54 Disable
#define INTDIS2IRQ53        (1<<21)                     // IRQ 53 Disable
#define INTDIS2IRQ52        (1<<20)                     // IRQ 52 Disable
#define INTDIS2IRQ51        (1<<19)                     // IRQ 51 Disable
#define INTDIS2IRQ50        (1<<18)                     // IRQ 50 Disable
#define INTDIS2IRQ49        (1<<17)                     // IRQ 49 Disable
#define INTDIS2IRQ48        (1<<16)                     // IRQ 48 Disable
#define INTDIS2IRQ47        (1<<15)                     // IRQ 47 Disable
#define INTDIS2IRQ46        (1<<14)                     // IRQ 46 Disable
#define INTDIS2IRQ45        (1<<13)                     // IRQ 45 Disable
#define INTDIS2IRQ44        (1<<12)                     // IRQ 44 Disable
#define INTDIS2IRQ43        (1<<11)                     // IRQ 43 Disable
#define INTDIS2IRQ42        (1<<10)                     // IRQ 42 Disable
#define INTDIS2IRQ41        (1<<9)                      // IRQ 41 Disable
#define INTDIS2IRQ40        (1<<8)                      // IRQ 40 Disable
#define INTDIS2IRQ39        (1<<7)                      // IRQ 39 Disable
#define INTDIS2IRQ38        (1<<6)                      // IRQ 38 Disable
#define INTDIS2IRQ37        (1<<5)                      // IRQ 37 Disable
#define INTDIS2IRQ36        (1<<4)                      // IRQ 36 Disable
#define INTDIS2IRQ35        (1<<3)                      // IRQ 35 Disable
#define INTDIS2IRQ34        (1<<2)                      // IRQ 34 Disable
#define INTDIS2IRQ33        (1<<1)                      // IRQ 33 Disable
#define INTDIS2IRQ32        (1<<0)                      // IRQ 32 Disable


#define INT_IRQDIS0         (INT_BASE+0x224)            // Basic IRQ Disable
//-------------------------------------------------------------------------------------------------------------------
#define INTDIS0IRQILL0      (1<<7)                      // Illegal Access type 0 IRQ Disable
#define INTDIS0IRQILL1      (1<<6)                      // Illegal Access type 1 IRQ Disable
#define INTDIS0IRQGPUH1     (1<<5)                      // GPU1 halted IRQ Disable
#define INTDIS0IRQGPUH0     (1<<4)                      // GPU0 halted IRQ Disable
#define INTDIS0IRQDOORB1    (1<<3)                      // ARM Doorbell 1 Disable
#define INTDIS0IRQDOORB0    (1<<2)                      // ARM Doorbell 0 Disable
#define INTDIS0IRQMAIL      (1<<1)                      // ARM Mailbox IRQ Disable
#define INTDIS0IRQTIMER     (1<<0)                      // ARM Timer IRQ Disable


//-------------------------------------------------------------------------------------------------------------------
// PCM/I2S Audio
//-------------------------------------------------------------------------------------------------------------------


//
// -- PCM registers
//    -------------

#define PCM_BASE            (HW_BASE+0x203000)          // The base register for the PCM/I2S Aduio controller


#define PCM_CS              (PCM_BASE)                  // PCM Controller and Status
//-------------------------------------------------------------------------------------------------------------------
#define PCMCS_STBY          (1<<25)                     // RAM Standby
#define PCMCS_SYNC          (1<<24)                     // PCM Clock Sync helper
#define PCMCS_RXSEX         (1<<23)                     // RX Sign Extend
#define PCMCS_RXF           (1<<22)                     // RX FIFO Full
#define PCMCS_TXE           (1<<21)                     // TX FIFO Empty
#define PCMCS_RXD           (1<<20)                     // RX FIFO has Data
#define PCMCS_TXD           (1<<19)                     // TX FIFO can accpet Data
#define PCMCS_RXR           (1<<18)                     // RX FIFO needs reading
#define PCMCS_TXW           (1<<17)                     // TX FIFO needs writing
#define PCMCS_RXERR         (1<<16)                     // RX FIFO Error
#define PCMCS_TXERR         (1<<15)                     // TX FIFO Error
#define PCMCS_RXSYNC        (1<<14)                     // RX FIFO Sync
#define PCMCS_TXSYNC        (1<<13)                     // TX FIFO Sync
#define PCMCS_DMAEN         (1<<9)                      // DMA DREQ Enable
#define PCMCS_RXTHR         (3<<7)                      // Sets when RXR flag is set
#define PCMCS_TXTHR         (3<<5)                      // Sets when TXR flag is set
#define PCMCS_RXCLR         (1<<4)                      // Set to clear RX FIFO
#define PCMCS_TXCLR         (1<<3)                      // Set to clear TX FIFO
#define PCMCS_TXON          (1<<2)                      // Transmission Enabled
#define PCMCS_RXON          (1<<1)                      // Reception Enabled
#define PCMCS_EN            (1<<0)                      // Enable the PCM Audio Interface

#define SH_PCMCSRXTHR(x)    (((x)&3)<<7)                // Shift to the correct bit locations
#define SH_PCMCSTXTHR(x)    (((x)&3)<<5)                // Shift to the correct bit locations


#define PCM_FIFO            (PCM_BASE+4)                // PCM FIFO Data
//-------------------------------------------------------------------------------------------------------------------
#define PCMFIFO_DATA        (0xffffffff)                // FIFO data


#define PCM_MODE            (PCM_BASE+8)                // PCM Mode
//-------------------------------------------------------------------------------------------------------------------
#define PCMMODE_CLKDIS      (1<<28)                     // PCM Clock Disable
#define PCMMODE_PDMN        (1<<27)                     // PDM Decimation Factor
#define PCMMODE_PDME        (1<<26)                     // PDM Input Mode Enable
#define PCMMODE_FRXP        (1<<25)                     // Receive Frame Packed Mode
#define PCMMODE_FTXP        (1<<24)                     // Transmit Frame Packed Mode
#define PCMMODE_CLKM        (1<<23)                     // PCM Clock mode
#define PCMMODE_CLKI        (1<<22)                     // PCM Clock Invert
#define PCMMODE_FSM         (1<<21)                     // Frame Sync Invert
#define PCMMODE_FLEN        (0x3ff<<10)                 // Frame Length
#define PCMMODE_FSLEN       (0x3ff)                     // Frame Sync Length

#define SH_PCMMODEFLEN(x)   (((x)&0x3ff)<<10)           // Shift to the correct bits


#define PCM_RXC             (PCM_BASE+0xc)              // PCM Receive configuration
//-------------------------------------------------------------------------------------------------------------------
#define PCMRXC_CH1WEX       (1<<31)                     // Channel 1 Width Extention Bit
#define PCMRXC_CH1EN        (1<<30)                     // Channel 1 Enable
#define PCMRXC_CH1POS       (0x3ff<<20)                 // Channel 1 Position
#define PCMRXC_CH1WID       (0xf<<16)                   // Channel 1 Width
#define PCMRXC_CH2WEX       (1<<15)                     // Channel 2 Width Extension Bit
#define PCMRXC_CH2EN        (1<<14)                     // Channel 2 Enable
#define PCMRXC_CH2POS       (0x3ff<<13)                 // Channel 2 Position
#define PCMRXC_CH2WID       (0xf)                       // Channel 2 Width

#define SH_PCMRXCCH1POS(x)  (((x)&0x3ff)<<20)           // Shift over to the right position
#define SH_PCMRXCCH1WID(x)  (((x)&0xf)<<16)             // Shift over to the right position
#define SH_PCMRXCCH2POS(x)  (((x)&0x3ff)<<13)           // Shift over to the right position


#define PCM_TXC             (PCM_BASE+0x10)             // PCM Transmit configuration
//-------------------------------------------------------------------------------------------------------------------
#define PCMTXC_CH1WEX       (1<<31)                     // Channel 1 Width Extention Bit
#define PCMTXC_CH1EN        (1<<30)                     // Channel 1 Enable
#define PCMTXC_CH1POS       (0x3ff<<20)                 // Channel 1 Position
#define PCMTXC_CH1WID       (0xf<<16)                   // Channel 1 Width
#define PCMTXC_CH2WEX       (1<<15)                     // Channel 2 Width Extension Bit
#define PCMTXC_CH2EN        (1<<14)                     // Channel 2 Enable
#define PCMTXC_CH2POS       (0x3ff<<13)                 // Channel 2 Position
#define PCMTXC_CH2WID       (0xf)                       // Channel 2 Width

#define SH_PCMTXCCH1POS(x)  (((x)&0x3ff)<<20)           // Shift over to the right position
#define SH_PCMTXCCH1WID(x)  (((x)&0xf)<<16)             // Shift over to the right position
#define SH_PCMTXCCH2POS(x)  (((x)&0x3ff)<<13)           // Shift over to the right position


#define PCM_DREQ            (PCM_BASE+0x14)             // PCM DMA Request Level
//-------------------------------------------------------------------------------------------------------------------
#define PCMDREQ_TXPANIC     (0x7f<<24)                  // TX Panic Level
#define PCMDREQ_RXPANIC     (0x7f<<16)                  // RX Panic Level
#define PCMDREQ_TX          (0x7f<<8)                   // TX Request Level
#define PCMDREQ_RX          (0x7f)                      // RX Request Level

#define SH_PCMDRECTXPAN(x)  (((x)&0x7f)<<24)            // Shift over to the right bits
#define SH_PCMDRECRXPAN(x)  (((x)&0x7f)<<16)            // Shift over to the right bits
#define SH_PCMDRECTX(x)     (((x)&0x7f)<<8)             // Shift over to the right bits


#define PCM_INTEN           (PCM_BASE+0x18)             // PCM Interrupt Enable
//-------------------------------------------------------------------------------------------------------------------
#define PCMINTEN_RXERR      (1<<3)                      // RX Error Interrupt
#define PCMINTEN_TXERR      (1<<2)                      // TX Error Interrupt
#define PCMINTEN_RXR        (1<<1)                      // RX Read Interrupt Enable
#define PCMINTEN_TXW        (1<<0)                      // TX Write Interrupt Enable


#define PCM_INTSTC          (PCM_BASE+0x1c)             // PCM Interrupt Status & Clear
//-------------------------------------------------------------------------------------------------------------------
#define PCMINTSTC_RXERR     (1<<3)                      // RX Error Interrupt Clear
#define PCMINTSTC_TXERR     (1<<2)                      // TX Error Interrupt Clear
#define PCMINTSTC_RXR       (1<<1)                      // RX Read Interrupt Enable Clear
#define PCMINTSTC_TXW       (1<<0)                      // TX Write Interrupt Enable Clear


#define PCM_GRAY            (PCM_BASE+0x20)             // PCM Gray Mode Control
//-------------------------------------------------------------------------------------------------------------------
#define PCMGRAY_RXFIFOLVL   (0x3f<<16)                  // Current level of the RX FIFO
#define PCMGRAY_FLUSHED     (0x3f<<10)                  // The number of bits that were flushed into RX FIFO
#define PCMGRAY_RXLEVEL     (0x3f<<4)                   // The current fill level of the RX buffer
#define PCMGRAY_FLUSH       (1<<2)                      // Flush RX buffer into RX FIFO
#define PCMGRAY_CLR         (1<<1)                      // Clear the GRAY Mode Logic
#define PCMGRAY_EN          (1<<0)                      // Enable GRAY mode


//-------------------------------------------------------------------------------------------------------------------
// Pulse Width Modulator
//-------------------------------------------------------------------------------------------------------------------


//
// -- The PWM Base address
//    --------------------
#define PWM_BASE            (HW_BASE+0x20c000)          // The base address of the PWM


#define PWM_CTL             (PWM_BASE)                  // PWM Control
//-------------------------------------------------------------------------------------------------------------------
#define PWMCTL_MSEN2        (1<<15)                     // Channel 2 M/S Enable
#define PWMCTL_USEF2        (1<<13)                     // Channel 2 Use FIFO
#define PWMCTL_POLA2        (1<<12)                     // Channel 2 Polarity
#define PWMCTL_SBIT2        (1<<11)                     // Channel 2 Silence Bit
#define PWMCTL_RPTL2        (1<<10)                     // Channel 2 Repeat Last Data
#define PWMCTL_MODE2        (1<<9)                      // Channel 2 Mode
#define PWMCTL_PWEN2        (1<<8)                      // Channel 2 Enable
#define PWMCTL_MSEN1        (1<<7)                      // Channel 1 M/S Enable
#define PWMCTL_CLRF1        (1<<6)                      // Clear FIFO
#define PWMCTL_USEF1        (1<<5)                      // Channel 1 Use FIFO
#define PWMCTL_POLA1        (1<<4)                      // Channel 1 Polarity
#define PWMCTL_SBIT1        (1<<3)                      // Channel 1 Silence Bit
#define PWMCTL_RPTL1        (1<<2)                      // Channel 1 Repeat Last Data
#define PWMCTL_MODE1        (1<<1)                      // Channel 1 Mode
#define PWMCTL_PWEN1        (1<<0)                      // Channel 1 Enable


#define PWM_STA             (PWM_BASE+4)                // PWM Status
//-------------------------------------------------------------------------------------------------------------------
#define PWMSTA_STA4         (1<<12)                     // Channel 4 state
#define PWMSTA_STA3         (1<<11)                     // Channel 3 state
#define PWMSTA_STA2         (1<<10)                     // Channel 2 state
#define PWMSTA_STA1         (1<<9)                      // Channel 1 state
#define PWMSTA_BERR         (1<<8)                      // Bus Error Flag
#define PWMSTA_GAPO4        (1<<7)                      // Channel 4 Gap Occurred Flag
#define PWMSTA_GAPO3        (1<<6)                      // Channel 3 Gap Occurred Flag
#define PWMSTA_GAPO2        (1<<5)                      // Channel 2 Gap Occurred Flag
#define PWMSTA_GAPO1        (1<<4)                      // Channel 1 Gap Occurred Flag
#define PWMSTA_RERR1        (1<<3)                      // FIFO Read Error Flag
#define PWMSTA_WERR1        (1<<2)                      // FIFO Write Error Flag
#define PWMSTA_EMPT1        (1<<1)                      // FIFO Empty Flag
#define PWMSTA_FULL1        (1<<0)                      // FIFO Full Flag


#define PWM_DMAC            (PWM_BASE+8)                // PWM DMA Configuration
//-------------------------------------------------------------------------------------------------------------------
#define PWMDMAC_ENAB        (1<<31)                     // DMA Enable
#define PWMDMAC_PANIC       (0xff<<8)                   // DMA Threshold for PANIC
#define PWMDMAC_DREQ        (0xff)                      // DMA Threshold for DREQ

#define SH_PWMDMACPANIC(x)  (((x)&0xff)<<8)             // Shift over to the correct bits


#define PWM_RNG1            (PWM_BASE+0x10)             // PWM Channel 1 Range
//-------------------------------------------------------------------------------------------------------------------
#define PWMRNG1_RANGE       (0xffffffff)                // Channel 1 Range


#define PWM_DAT1            (PWM_BASE+0x14)             // PWM Channel 1 Data
//-------------------------------------------------------------------------------------------------------------------
#define PWMDAT1_DATA        (0xffffffff)                // Channel 1 Data


#define PWM_FIF1            (PWM_BASE+0x18)             // PWM FIFO Input
//-------------------------------------------------------------------------------------------------------------------
#define PWMFIF1_FIFO        (0xffffffff)                // FIFO Input


#define PWM_RNG2            (PWM_BASE+0x20)             // PWM Channel 2 Range
//-------------------------------------------------------------------------------------------------------------------
#define PWMRNG2_RANGE       (0xffffffff)                // Channel 2 Range


#define PWM_DAT2            (PWM_BASE+0x24)             // PWM Channel 2 Data
//-------------------------------------------------------------------------------------------------------------------
#define PWMDAT2_DATA        (0xffffffff)                // Channel 2 Data


//-------------------------------------------------------------------------------------------------------------------
// The Serial Peripheral Interface (SPI)
//-------------------------------------------------------------------------------------------------------------------


//
// -- SPI0 Interface
//    --------------
#define SPIO_BASE           (HW_BASE+0x204000)          // The base address of the SPI0

#define SPI0_CS             (SPI0_BASE)                 // SPI Master Control and Status
//-------------------------------------------------------------------------------------------------------------------
#define SPI0CS_LENLONG      (1<<25)                     // Enable Long data word in LoSSI mode
#define SPI0CS_DMALEN       (1<<24)                     // ENable DMA mode in LoSSI mode
#define SPI0CS_CSPOL2       (1<<23)                     // Chip Select 2 polarity
#define SPI0CS_CSPOL1       (1<<22)                     // Chip Select 1 polarity
#define SPI0CS_CSPOL0       (1<<21)                     // Chip Select 0 polarity
#define SPI0CS_RXF          (1<<20)                     // RX FIFO Full
#define SPI0CS_RXR          (1<<19)                     // RX FIFO needs reading
#define SPI0CS_TXD          (1<<18)                     // TX FIFO can accept data
#define SPI0CS_RXD          (1<<17)                     // RX FIFO contains data
#define SPI0CS_DONE         (1<<16)                     // Data transfer done
#define SPI0CS_LEN          (1<<13)                     // LoSSI Enable
#define SPI0CS_REN          (1<<12)                     // Read Enable
#define SPI0CS_ADCS         (1<<11)                     // Automatically Deassert Chip Select
#define SPI0CS_INTR         (1<<10)                     // Interrupt RXR
#define SPI0CS_INTD         (1<<9)                      // Interrupt Done
#define SPI0CS_DMAEN        (1<<8)                      // DMA Enable
#define SPI0CS_TA           (1<<7)                      // Transfer Active
#define SPI0CS_CSPOL        (1<<6)                      // Chip Select Polarity
#define SPI0CS_CLEAR        (3<<4)                      // Clear FIFO
#define SPI0CS_CPOL         (1<<3)                      // Clock Polarity
#define SPI0CS_CPHA         (1<<2)                      // Clock Phase
#define SPI0CS_CS           (3<<0)                      // Chip Select

#define SH_CSI0CSCLEARNO    (0b00<<4)                   // No action
#define SH_CSI0CSCLEARTX    (0b01<<4)                   // Clear TX FIFO
#define SH_CSI0CSCLEARRX    (0b10<<4)                   // Clear RX FIFO
#define SH_CSI0CSCLEARBOTH  (0x11<<4)                   // Clear TX/RX FIFO
#define SH_CSI0CS0          (0b00)                      // Chip Select 0
#define SH_CSI0CS1          (0b01)                      // Chip Select 1
#define SH_CSI0CS2          (0b10)                      // Chip Select 2


#define SPI0_FIFO           (SPI0_BASE+4)               // SPI Master TX and RX FIFOs
//-------------------------------------------------------------------------------------------------------------------
#define SPI0FIFO_DATA       (0xffffffff)                // Depends on DMAEN.  See documentaiton


#define SPI0_CLK            (SPI0_BASE+8)               // SPI Master Clock Divider
//-------------------------------------------------------------------------------------------------------------------
#define SPI0CLK_CDIV        (0xffff)                    // Clock Divider


#define SPI0_DLEN           (SPI0_BASE+0xc)             // SPI Master Data Length
//-------------------------------------------------------------------------------------------------------------------
#define SPI0DLEN_LEN        (0xffff)                    // Data Length


#define SPI0_LTOH           (SPI0_BASE+0x10)            // SPI LoSSI mode TOH
//-------------------------------------------------------------------------------------------------------------------
#define SPI0LTOH_TOH        (0x7)                       // Output Hold Delay


#define SPI0_DC             (SPI0_BASE+0x14)            // SPU DMA DREQ Controls
//-------------------------------------------------------------------------------------------------------------------
#define SPI0DC_RPANIC       (0xff<<24)                  // DMA Read Panic threshold
#define SPI0DC_RDREQ        (0xff<<16)                  // DMA Read Request threshold
#define SPI0DC_TPANIC       (0xff<<8)                   // DMA Write Panic threshold
#define SPI0DC_TDREQ        (0xff)                      // DMA Write Request threshold

#define SH_SPI0DCRPANIC(x)  (((x)&0xff)<<24)            // Shift to the correct bits
#define SH_SPI0DCRDREQ(x)   (((x)&0xff)<<16)            // Shift to the correct bits
#define SH_SPI0DCTPANIC(x)  (((x)&0xff)<<8)             // Shift to the correct bits


//-------------------------------------------------------------------------------------------------------------------
// The SPI/BSC slave
//-------------------------------------------------------------------------------------------------------------------


//
// -- SPI/BSC slave interface (shortened to SLV)
//    ------------------------------------------
#define SLV_BASE            (HW_BASE+0x214000)          // The SPI/BSC Slave base register


#define SLV_DR              (SLV_BASE)                  // Data register
//-------------------------------------------------------------------------------------------------------------------
#define SLVDR_RXFLEVEL      (0x1f<<27)                  // RX FIFO Level
#define SLVDR_TXFLEVEL      (0x1f<<22)                  // TX FIFO Level
#define SLVDR_RXBUSY        (1<<21)                     // Receive Busy
#define SLVDR_TXFE          (1<<20)                     // TX FIFO Empty
#define SLVDR_RXFF          (1<<19)                     // RX FIFO Full
#define SLVDR_TXFF          (1<<18)                     // TX FIFO Full
#define SLVDR_RXFE          (1<<17)                     // RX FIFO Empty
#define SLVDR_TXBUSY        (1<<16)                     // Transmit Busy
#define SLVDR_TXUE          (1<<9)                      // TX Underrun Error
#define SLVDR_RXOE          (1<<8)                      // RX Overrun Error
#define SLVDR_DATA          (0xff)                      // Data


#define SLV_RSR             (SLV_BASE+4)                // operation status and error clear register
//-------------------------------------------------------------------------------------------------------------------
#define SLVRSR_TXUE         (1<<1)                      // TX Underrun Error
#define SLVRSR_RXOE         (1<<0)                      // RX Overrun Error


#define SLV_SLV             (SLV_BASE+8)                // I2C slave address value
//-------------------------------------------------------------------------------------------------------------------
#define SLVSLV_ADDR         (0x7f)                      // I2C Slave Address


#define SLV_CR              (SLV_BASE+0xc)              // Control Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVCR_INVTXF        (1<<13)                     // TX Status Flag
#define SLVCR_HOSTCTRLEN    (1<<12)                     // Enable control for host
#define SLVCR_TESTFIFO      (1<<11)                     // TEST FIFO
#define SLVCR_INVRXF        (1<<10)                     // RX Status Flag
#define SLVCR_RXE           (1<<9)                      // Receive Enable
#define SLVCR_TXE           (1<<8)                      // Transmit Enable
#define SLVCR_BRK           (1<<7)                      // Break current operation
#define SLVCR_ENCTRL        (1<<6)                      // Enable Control 8bit register
#define SLVCR_ENSTAT        (1<<5)                      // Enable Status 8bit register
#define SLVCR_CPOL          (1<<4)                      // Clock Polarity
#define SLVCR_CPHA          (1<<3)                      // Clock Phase
#define SLVCR_I2C           (1<<2)                      // I2C Mode
#define SLVCR_SPI           (1<<1)                      // SPI Mode
#define SLVCR_EN            (1<<0)                      // Enable Device


#define SLV_FR              (SLV_BASE+0x10)             // Flag Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVFR_RXFLEVEL      (0x1f<<11)                  // RX FIFO Level
#define SLVFR_TXFLEVEL      (0x1f<<6)                   // TX FIFO Level
#define SLVFR_RXBUSY        (1<<5)                      // Receive Busy
#define SLVFR_TXFE          (1<<4)                      // TX FIFO Empty
#define SLVFR_RXFF          (1<<3)                      // RX FIFO Full
#define SLVFR_TXFF          (1<<2)                      // TX FIFO Full
#define SLVFR_RXFE          (1<<1)                      // RX FIFO Empty
#define SLVFR_TXBUSY        (1<<0)                      // Transmit Busy

#define SH_SLVFRRXFLEVEL(x) (((x)&0x1f)<<11)            // RX FIFO Level
#define SH_SLVFRTXFLEVEL(x) (((x)&0x1f)<<6)             // TX FIFO Level


#define SLV_IFLS            (SLV_BASE+0x14)             // interrupt FIFO Select register
//-------------------------------------------------------------------------------------------------------------------
#define SLVIFLS_RXIFLSEL    (0x7<<3)                    // RX Interrupt FIFO Level Select
#define SLVIFLS_TXIFLSEL    (0x7)                       // TX Interrupt FIFO Level Select

#define SLVIFLS_1_8         (0b000)                     // Interrupt when 1/8 full
#define SLVIFLS_1_4         (0b001)                     // Interrupt when 1/4 full
#define SLVIFLS_1_2         (0b010)                     // Interrupt when 1/2 full
#define SLVIFLS_3_4         (0b011)                     // Interrupt when 3/4 full
#define SLVIFLS_7_8         (0b100)                     // Interrupt when 7/8 full

#define SH_SLVIFLSRXIFLSEL(x) (((x)&0x7)<<3)            // shift to the correct position
#define SH_SLVIFLSTXIFLSEL(x) ((x)&0x7)                 // shift to the correct position


#define SLV_IMSC            (SLV_BASE+0x18)             // Interrupt Mask Clear Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVIMSC_OEIM        (1<<3)                      // Overrun error interrupt mask
#define SLVIMSC_BEIM        (1<<2)                      // Break error interrupt mask
#define SLVIMSC_TXIM        (1<<1)                      // Transmit interrupt mask
#define SLVIMSC_RXIM        (1<<0)                      // Receive interrupt mask


#define SLV_RIS             (SLV_BASE+0x1c)             // Raw Interrupt Status Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVRIS_OERIS        (1<<3)                      // Overrun error interrupt status
#define SLVRIS_BERIS        (1<<2)                      // Break error interrupt status
#define SLVRIS_TXRIS        (1<<1)                      // Transmit interrupt status
#define SLVRIS_RXRIS        (1<<0)                      // Receive interrupt status


#define SLV_MIS             (SLV_BASE+0x20)             // Masked Interrupt Status Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVMIS_OEMIS        (1<<3)                      // Overrun error interrupt masked interrupt status
#define SLVMIS_BEMIS        (1<<2)                      // Break error interrupt masked interrupt status
#define SLVMIS_TXMIS        (1<<1)                      // Transmit interrupt masked interrupt status
#define SLVMIS_RXMIS        (1<<0)                      // Receive interrupt masked interrupt status


#define SLV_ICR             (SLV_BASE+0x24)             // Interrupt Clear Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVIMSC_OEIC        (1<<3)                      // Overrun error interrupt clear
#define SLVIMSC_BEIC        (1<<2)                      // Break error interrupt clear
#define SLVIMSC_TXIC        (1<<1)                      // Transmit interrupt clear
#define SLVIMSC_RXIC        (1<<0)                      // Receive interrupt clear


#define SLV_DMACR           (SLV_BASE+0x28)             // DMA Control Register
//-------------------------------------------------------------------------------------------------------------------
// This is unsupported


#define SLV_TDR             (SLV_BASE+0x2c)             // FIFO Test Data
//-------------------------------------------------------------------------------------------------------------------
#define SLVTDR_DATA         (0xff)                      // Test Data


#define SLV_GPUSTAT         (SLV_BASE+0x30)             // GPU Status Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVGPUSTAT_DATA     (0xff)                      // GPU to Host status register


#define SLV_HCTRL           (SLV_BASE+0x34)             // Host Control Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVHCTRL_DATA       (0xff)                      // Host Control Register


#define SLV_DEBUG1          (SLV_BASE+0x38)             // I2C Debug Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVDEBUG1_DATA      (0x3ffffff)                 // Debug Register


#define SLV_DEBUG2          (SLV_BASE+0x3c)             // SPI Debug Register
//-------------------------------------------------------------------------------------------------------------------
#define SLVDEBUG2_DATA      (0x3ffffff)                 // Debug Register


//-------------------------------------------------------------------------------------------------------------------
// The System Timers
//-------------------------------------------------------------------------------------------------------------------


//
// -- System Timer Interface
//    ----------------------
#define ST_BASE             (HW_BASE+0x3000)            // System Timer base address


#define ST_CS               (ST_BASE)                   // Control/Status register
//-------------------------------------------------------------------------------------------------------------------
#define STCS_M3             (1<<3)                      // System Timer Match 3
#define STCS_M2             (1<<2)                      // System Timer Match 2
#define STCS_M1             (1<<1)                      // System Timer Match 1
#define STCS_M0             (1<<0)                      // System Timer Match 0


#define ST_CLO              (ST_BASE+4)                 // Counter Lower 32 bits
//-------------------------------------------------------------------------------------------------------------------
#define STCLO_CNT           (0xffffffff)                // lower bits of the running counter


#define ST_CHI              (ST_BASE+8)                 // Counter Upper 32 bits
//-------------------------------------------------------------------------------------------------------------------
#define STCHI_CNT           (0xffffffff)                // higher bits of the running counter


#define ST_C0               (ST_BASE+0xc)               // Compare 0
//-------------------------------------------------------------------------------------------------------------------
#define STC0_CMP            (0xffffffff)                // Compare value


#define ST_C1               (ST_BASE+0x10)              // Compare 1
//-------------------------------------------------------------------------------------------------------------------
#define STC1_CMP            (0xffffffff)                // Compare value


#define ST_C2               (ST_BASE+0x14)              // Compare 2
//-------------------------------------------------------------------------------------------------------------------
#define STC2_CMP            (0xffffffff)                // Compare value


#define ST_C3               (ST_BASE+0x18)              // Compare 3
//-------------------------------------------------------------------------------------------------------------------
#define STC3_CMP            (0xffffffff)                // Compare value


//-------------------------------------------------------------------------------------------------------------------
// The UART
//-------------------------------------------------------------------------------------------------------------------


//
// -- UART Interface
//    --------------
#define UART_BASE           (HW_BASE+0x201000)          // The UART base register


#define UART_DR             (0x00)                      // Data register
//-------------------------------------------------------------------------------------------------------------------
#define UARTDR_OE           (1<<11)                     // Overrun Error
#define UARTDR_BE           (1<<10)                     // Break Error
#define UARTDR_PE           (1<<9)                      // Parity Error
#define UARTDR_FE           (1<<8)                      // Framing Error
#define UARTDR_DATA         (0xff)                      // Data


#define UART_RSRECR         (0x04)                      // operation status and error clear register
//-------------------------------------------------------------------------------------------------------------------
#define UARTRSRECR_OE       (1<<3)                      // Overrun Error
#define UARTRSRECR_BE       (1<<2)                      // Break Error
#define UARTRSRECR_PE       (1<<1)                      // Parity Error
#define UARTRSRECR_FE       (1<<0)                      // Framing Error


#define UART_FR             (0x10)                      // Flag Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTFR_TXFE         (1<<7)                      // TX FIFO Empty
#define UARTFR_RXFF         (1<<6)                      // RX FIFO Full
#define UARTFR_TXFF         (1<<5)                      // TX FIFO Full
#define UARTFR_RXFE         (1<<4)                      // RX FIFO Empty
#define UARTFR_BUSY         (1<<3)                      // Transmit Busy
#define UARTFR_CTS          (1<<0)                      // Clear to send


#define UART_IBRD           (0x24)                      // Integer Baud Rate Divisor
//-------------------------------------------------------------------------------------------------------------------
#define UARTIBRD_IBRD       (0xffff)                    // Integer Baud Rate Divisor


#define UART_FBRD           (0x28)                      // Fractional Baud Rate Divisor
//-------------------------------------------------------------------------------------------------------------------
#define UARTFBRD_FBRD       (0x3f)                      // Fractional Baud Rate Divisor


#define UART_LCRH           (0x2c)                      // Line Control Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTLCRH_SPS        (1<<7)                      // Stick Parity Select
#define UARTLCRH_WLEN       (3<<5)                      // Word Length
#define UARTLCRH_FEN        (1<<4)                      // Enable FIFO
#define UARTLCRH_STP2       (1<<3)                      // 2 Stop bit enable
#define UARTLCRH_EPS        (1<<2)                      // Even Parity bit select
#define UARTLCRH_PEN        (1<<1)                      // Parity Enable
#define UARTLCRH_BRK        (1<<0)                      // Send break


#define SH_UARTLCRHWLEN_8   ((0b11)<<5)                 // 8 bit word length
#define SH_UARTLCRHWLEN_7   ((0b10)<<5)                 // 7 bit word length
#define SH_UARTLCRHWLEN_6   ((0b01)<<5)                 // 6 bit word length
#define SH_UARTLCRHWLEN_5   ((0b00)<<5)                 // 5 bit word length


#define UART_CR             (0x30)                      // Control Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTCR_CTSEN        (1<<15)                     // Enable CTS flow control
#define UARTCR_RTSEN        (1<<14)                     // Enable RTS flow control
#define UARTCR_RTS          (1<<11)                     // Request to send
#define UARTCR_RXE          (1<<9)                      // Receive Enable
#define UARTCR_TXE          (1<<8)                      // Transmit Enable
#define UARTCR_LBE          (1<<7)                      // Loopback enable
#define UARTCR_EN           (1<<0)                      // Enable Device


#define UART_IFLS           (0x34)                      // interrupt FIFO Select register
//-------------------------------------------------------------------------------------------------------------------
#define UARTIFLS_RXIFLSEL   (0x7<<3)                    // RX Interrupt FIFO Level Select
#define UARTIFLS_TXIFLSEL   (0x7)                       // TX Interrupt FIFO Level Select

#define UARTIFLS_1_8        (0b000)                     // Interrupt when 1/8 full
#define UARTIFLS_1_4        (0b001)                     // Interrupt when 1/4 full
#define UARTIFLS_1_2        (0b010)                     // Interrupt when 1/2 full
#define UARTIFLS_3_4        (0b011)                     // Interrupt when 3/4 full
#define UARTIFLS_7_8        (0b100)                     // Interrupt when 7/8 full

#define SH_UARTIFLSRXIFLSEL(x) (((x)&0x7)<<3)           // shift to the correct position
#define SH_UARTIFLSTXIFLSEL(x) ((x)&0x7)                // shift to the correct position


#define UART_IMSC           (0x38)                      // Interrupt Mask Clear Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTIMSC_OEIM       (1<<10)                     // Overrun error interrupt mask
#define UARTIMSC_BEIM       (1<<9)                      // Break error interrupt mask
#define UARTIMSC_PEIM       (1<<8)                      // Parity error interrups mask
#define UARTIMSC_FEIM       (1<<7)                      // Framing error interrupt mask
#define UARTIMSC_RTIM       (1<<6)                      // Receive Timeout interrupt mask
#define UARTIMSC_TXIM       (1<<5)                      // Transmit interrupt mask
#define UARTIMSC_RXIM       (1<<4)                      // Receive interrupt mask
#define UARTIMSC_CTSMIM     (1<<1)                      // UARTCTR Modem interrupt mask


#define UART_RIS            (0x3c)                      // Raw Interrupt Status Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTRIS_OERIS       (1<<10)                     // Overrun error interrupt status
#define UARTRIS_BERIS       (1<<9)                      // Break error interrupt status
#define UARTRIS_PERIS       (1<<8)                      // Parity error interrups status
#define UARTRIS_FERIS       (1<<7)                      // Framing error interrupt status
#define UARTRIS_RTRIS       (1<<6)                      // Receive Timeout interrupt status
#define UARTRIS_TXRIS       (1<<5)                      // Transmit interrupt status
#define UARTRIS_RXRIS       (1<<4)                      // Receive interrupt status
#define UARTRIS_CTSRMIS     (1<<1)                      // UARTCTR Modem interrupt status


#define UART_MIS            (0x40)                      // Masked Interrupt Status Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTMIS_OEMIS       (1<<10)                     // Overrun error interrupt status
#define UARTMIS_BEMIS       (1<<9)                      // Break error interrupt status
#define UARTMIS_PEMIS       (1<<8)                      // Parity error interrups status
#define UARTMIS_FEMIS       (1<<7)                      // Framing error interrupt status
#define UARTMIS_RTMIS       (1<<6)                      // Receive Timeout interrupt status
#define UARTMIS_TXMIS       (1<<5)                      // Transmit interrupt status
#define UARTMIS_RXMIS       (1<<4)                      // Receive interrupt status
#define UARTMIS_CTSMMIS     (1<<1)                      // UARTCTR Modem interrupt status


#define UART_ICR            (0x44)                      // Interrupt Clear Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTMIS_OEIC        (1<<10)                     // Overrun error interrupt status
#define UARTMIS_BEIC        (1<<9)                      // Break error interrupt status
#define UARTMIS_PEIC        (1<<8)                      // Parity error interrups status
#define UARTMIS_FEIC        (1<<7)                      // Framing error interrupt status
#define UARTMIS_RTIC        (1<<6)                      // Receive Timeout interrupt status
#define UARTMIS_TXIC        (1<<5)                      // Transmit interrupt status
#define UARTMIS_RXIC        (1<<4)                      // Receive interrupt status
#define UARTMIS_CTSMIC      (1<<1)                      // UARTCTR Modem interrupt status


#define UART_DMACR          (0x48)                      // DMA Control Register
//-------------------------------------------------------------------------------------------------------------------
// -- unsupported


#define UART_ITCR           (0x80)                      // Test Control Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTITCR_ITCR1      (1<<1)                      // Test FIFO Enable
#define UARTITCR_ITCR0      (1<<0)                      // Integration test enable


#define UART_ITIP           (0x84)                      // Integration Test Input Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTITIP_ITIP3      (1<<3)                      // reads return the value ot the nUARTCTS primary input
#define UARTITIP_ITIP0      (1<<0)                      // reads return the value ot the UARTRXD primary input


#define UART_ITOP           (0x88)                      // Integration Test Output Register
//-------------------------------------------------------------------------------------------------------------------
#define UARTITOP_ITOP11     (1<<11)                     // output from UARTMSINTR
#define UARTITOP_ITOP10     (1<<10)                     // output from UARTRXINTR
#define UARTITOP_ITOP9      (1<<9)                      // output from UARTTXINTR
#define UARTITOP_ITOP8      (1<<8)                      // output from UARTRTINTR
#define UARTITOP_ITOP7      (1<<7)                      // output from UARTEIINTR
#define UARTITOP_ITOP6      (1<<6)                      // output from UARTINTR
#define UARTITOP_ITOP3      (1<<3)                      // output from nUARTCTS
#define UARTITOP_ITOP0      (1<<0)                      // output from UARTTXD


#define UART_TDR            (0x8c)                      // FIFO Test Data
//-------------------------------------------------------------------------------------------------------------------
#define UARTTDR_TDR         (0x7ff)                     // Written to FIFO when ITCR1=1


//-------------------------------------------------------------------------------------------------------------------
// The ARM Timer -- not recommended to be used to accurately time anything, so will not be used.
//-------------------------------------------------------------------------------------------------------------------
