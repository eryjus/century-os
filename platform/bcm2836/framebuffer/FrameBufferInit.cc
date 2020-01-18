//===================================================================================================================
//
//  FrameBufferInit.cc -- Frame buffer initialization for the console (rpi2b version)
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
//  2019-Feb-15  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "printf.h"
#include "fb.h"
#include "mmu.h"
#include "hardware.h"


//
// -- this is the buffer for the mailbox
//    ----------------------------------
EXPORT LOADER_BSS
uint32_t mbBuf[64] __attribute__((aligned(16)));


//
// -- Initialize the additional frame buffer info
//    -------------------------------------------
EXTERN_C EXPORT LOADER
void FrameBufferInit(void)
{
    kprintf("Setting up the frame buffer\n");

    uint16_t *fb;

    kMemSetB(mbBuf, 0, sizeof(mbBuf));

    mbBuf[0] = 32 * 4;
    mbBuf[1] = 0;           // This is a request code
    mbBuf[2] = 0x00048003;  // Set the physical width/height
    mbBuf[3] = 8;           // 8 byte request/reply
    mbBuf[4] = 0;           // indicate this is a request
    mbBuf[5] = WIDTH;       // 800 pixels wide
    mbBuf[6] = HEIGHT;      // 400 pixels high
    mbBuf[7] = 0x00048004;  // Set the virtual width/height
    mbBuf[8] = 8;           // 8 byte request/reply
    mbBuf[9] = 0;           // indicate this is a request
    mbBuf[10] = WIDTH;      // 800 pixels wide
    mbBuf[11] = HEIGHT;     // 400 pixels high
    mbBuf[12] = 0x00048005; // Set the color depth
    mbBuf[13] = 4;          // 4 byte request/reply
    mbBuf[14] = 0;          // indicate this is a request
    mbBuf[15] = DEPTH;      // 16-bit color
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


    kprintf("The physical address of the buffer at %p is %p\n", mbBuf, MmuVirtToPhys(mbBuf));
    CLEAN_CACHE(mbBuf, sizeof(mbBuf));
    MailboxSend(&kernelMailbox, 8, MmuVirtToPhys(mbBuf));
    MailboxReceive(&kernelMailbox, 8);
    INVALIDATE_CACHE(mbBuf, sizeof(mbBuf));


    fb = (uint16_t *)(mbBuf[24] + ARM_MAILBOX_OFFSET);
    SetFrameBufferAddr(fb);
    SetFrameBufferHeight(HEIGHT);
    SetFrameBufferWidth(WIDTH);
    SetFrameBufferBpp(DEPTH);
    SetFrameBufferPitch(mbBuf[29]?mbBuf[29]:WIDTH*DEPTH);

    kprintf(".. Framebuffer located at: %p\n", GetFrameBufferAddr());
    kprintf(".. Framebuffer size: %p\n", GetFrameBufferPitch() * GetFrameBufferHeight());


    //
    // -- Map the frame buffer to its final location in virtual memory
    //    ------------------------------------------------------------
    kprintf("Mapping the Frame Buffer\n");
    for (archsize_t fbVirt = MMU_FRAMEBUFFER, fbFrame = ((archsize_t)GetFrameBufferAddr()) >> 12,
                    fbEnd = fbVirt + (GetFrameBufferPitch() * GetFrameBufferHeight());
            fbVirt < fbEnd; fbVirt += PAGE_SIZE, fbFrame ++) {
        MmuMapToFrame(fbVirt, fbFrame, PG_KRN | PG_WRT | PG_DEVICE);
    }

    // -- goose the config to the correct fb address
    SetFrameBufferAddr((uint16_t *)MMU_FRAMEBUFFER);
    SetFgColor(0xffff);
    SetBgColor(0x1234);
    FrameBufferClear();
}
