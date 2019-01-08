//===================================================================================================================
//
//  FrameBufferInit.cc -- Frame buffer initialization for the console (rpi2b version)
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function interacts with the mailbox.  For this to work, it requires a 16-byte aligned chunck of memory.
//  I want to be able to allocate this in the .bss section, so here is how this works.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Jan-05  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "hw-loader.h"
#include "serial-loader.h"
#include "fb.h"


//
// -- this is the buffer for the mailbox
//    ----------------------------------
uint32_t mbBuf[64] __attribute__((aligned(16)));


//
// -- Initialize the additional frame buffer info
//    -------------------------------------------
void FrameBufferInit(void)
{
    SerialPutS("Setting up the frame buffer\n");
    uint16_t *fb;

    mbBuf[0] = 32 * 4;
    mbBuf[1] = 0;           // This is a request code
    mbBuf[2] = 0x00048003;  // Set the physical width/height
    mbBuf[3] = 8;           // 8 byte request/reply
    mbBuf[4] = 0;           // indicate this is a request
    mbBuf[5] = 800;         // 800 pixels wide
    mbBuf[6] = 400;         // 400 pixels high
    mbBuf[7] = 0x00048004;  // Set the virtual width/height
    mbBuf[8] = 8;           // 8 byte request/reply
    mbBuf[9] = 0;           // indicate this is a request
    mbBuf[10] = 800;        // 800 pixels wide
    mbBuf[11] = 400;        // 400 pixels high
    mbBuf[12] = 0x00048005; // Set the color depth
    mbBuf[13] = 4;          // 4 byte request/reply
    mbBuf[14] = 0;          // indicate this is a request
    mbBuf[15] = 16;         // 32-bit color
    mbBuf[16] = 0x00048009; // Set the virtual offset
    mbBuf[17] = 8;          // 8 byte request/reply
    mbBuf[18] = 0;          // indicate this is a request
    mbBuf[19] = 0;          // offset at 0,0
    mbBuf[20] = 0;
    mbBuf[21] = 0x00040001; // Allocate the frame buffer
    mbBuf[22] = 8;          // 8 byte request/reply
    mbBuf[23] = 0;          // indicate this is a request
    mbBuf[24] = 0;          // fb addr
    mbBuf[25] = 0;          // fb size
    mbBuf[26] = 0x00040008; // Get the pitch
    mbBuf[27] = 4;          // 4 byte request/reply
    mbBuf[28] = 0;          // indicate this is a request
    mbBuf[29] = 0;          // pitch returned here
    mbBuf[30] = 0;          // last tag
    mbBuf[31] = 0;          // clear one more anyway

    MailboxSend((uint32_t)mbBuf, 8);
    MailboxReceive(8);


    fb = (uint16_t *)(mbBuf[24] + 0x40000000);
    SetFrameBufferAddr(fb);
    SetFrameBufferHeight(400);
    SetFrameBufferWidth(800);
    SetFrameBufferBpp(16);
    SetFrameBufferPitch(mbBuf[29]);

    SerialPutS(".. Framebuffer located at: "); SerialPutHex((uint32_t)GetFrameBufferAddr()); SerialPutChar('\n');
    SerialPutS(".. Framebuffer size: "); SerialPutHex(mbBuf[25]); SerialPutChar('\n');

    SetFgColor(0xffff);
    SetBgColor(0x1234);
    FrameBufferClear();
}
