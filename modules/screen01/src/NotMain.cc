


#include "screen01.h"


extern "C" void NotMain(void)
{
//    SerialInit();

    uint32_t *mbBuf = (uint32_t *)0x5000;

    SerialPutS("Getting the current width/height of the frame buffer\n");
    mbBuf[0] = 32 * 4;
    mbBuf[1] = 0;           // This is a request code
    mbBuf[2] = 0x00040003;  // allocate a frame buffer
    mbBuf[3] = 8;           // room for the buffer and response -- bytes in the value buffer
    mbBuf[4] = 0;           // indicate a request; size will be delivered back here
    mbBuf[5] = 0;           // will be the width in pixels
    mbBuf[6] = 0;           // will be the height in pixels
    mbBuf[7] = 0;           // this is the end tag; no more to process

    // clear the rest of the buffer
    for (int i = 8; i < 32; i ++) mbBuf[i] = 0;

    SerialPutS("Sending...\n");
    MailboxSend((uint32_t)mbBuf - 0x40000000, 8);
    SerialPutS("Receiving...\n");

    SerialPutS(".. The width is reported "); SerialPutHex(mbBuf[5]); SerialPutChar('\n');
    SerialPutS(".. The height is reported "); SerialPutHex(mbBuf[6]); SerialPutChar('\n');


    SerialPutS("Getting the current depth of the frame buffer\n");
    mbBuf[0] = 32 * 4;
    mbBuf[1] = 0;           // This is a request code
    mbBuf[2] = 0x00040005;  // get the depth
    mbBuf[3] = 4;           // room for the buffer and response -- bytes in the value buffer
    mbBuf[4] = 0;           // indicate a request; size will be delivered back here
    mbBuf[5] = 0;           // will be the depth
    mbBuf[6] = 0;           // this is the end tag; no more to process

    // clear the rest of the buffer
    for (int i = 7; i < 32; i ++) mbBuf[i] = 0;

    SerialPutS("Sending...\n");
    MailboxSend((uint32_t)mbBuf - 0x40000000, 8);
    SerialPutS("Receiving...\n");

    SerialPutS(".. The depth is reported "); SerialPutHex(mbBuf[5]); SerialPutChar('\n');



    SerialPutS("Getting the current pixel order of the frame buffer\n");
    mbBuf[0] = 32 * 4;
    mbBuf[1] = 0;           // This is a request code
    mbBuf[2] = 0x00040006;  // get the order
    mbBuf[3] = 4;           // room for the buffer and response -- bytes in the value buffer
    mbBuf[4] = 0;           // indicate a request; size will be delivered back here
    mbBuf[5] = 0;           // will be the depth
    mbBuf[6] = 0;           // this is the end tag; no more to process

    // clear the rest of the buffer
    for (int i = 7; i < 32; i ++) mbBuf[i] = 0;

    SerialPutS("Sending...\n");
    MailboxSend((uint32_t)mbBuf - 0x40000000, 8);
    SerialPutS("Receiving...\n");

    SerialPutS(".. The pixel order is reported "); SerialPutHex(mbBuf[5]); SerialPutChar('\n');



    SerialPutS("Getting the current alpha mode of the frame buffer\n");
    mbBuf[0] = 32 * 4;
    mbBuf[1] = 0;           // This is a request code
    mbBuf[2] = 0x00040007;  // get the mode
    mbBuf[3] = 4;           // room for the buffer and response -- bytes in the value buffer
    mbBuf[4] = 0;           // indicate a request; size will be delivered back here
    mbBuf[5] = 0;           // will be the depth
    mbBuf[6] = 0;           // this is the end tag; no more to process

    // clear the rest of the buffer
    for (int i = 7; i < 32; i ++) mbBuf[i] = 0;

    SerialPutS("Sending...\n");
    MailboxSend((uint32_t)mbBuf - 0x40000000, 8);
    SerialPutS("Receiving...\n");

    SerialPutS(".. The alpha mode is reported "); SerialPutHex(mbBuf[5]); SerialPutChar('\n');



    SerialPutS("Getting the current pitch of the frame buffer\n");
    mbBuf[0] = 32 * 4;
    mbBuf[1] = 0;           // This is a request code
    mbBuf[2] = 0x00040008;  // get the pitch
    mbBuf[3] = 4;           // room for the buffer and response -- bytes in the value buffer
    mbBuf[4] = 0;           // indicate a request; size will be delivered back here
    mbBuf[5] = 0;           // will be the depth
    mbBuf[6] = 0;           // this is the end tag; no more to process

    // clear the rest of the buffer
    for (int i = 7; i < 32; i ++) mbBuf[i] = 0;

    SerialPutS("Sending...\n");
    MailboxSend((uint32_t)mbBuf - 0x40000000, 8);
    SerialPutS("Receiving...\n");

    SerialPutS(".. The pitch is reported "); SerialPutHex(mbBuf[5]); SerialPutChar('\n');



    SerialPutS("Setting up message to get the virtual FB size\n");
    mbBuf[0] = 32 * 4;
    mbBuf[1] = 0;           // This is a request code
    mbBuf[2] = 0x00048003;  // Set the physical width/height
    mbBuf[3] = 8;           // 8 byte request/reply
    mbBuf[4] = 0;           // indicate this is a request
    mbBuf[5] = 1024;         // 800 pixels wide
    mbBuf[6] = 768;         // 400 pixels high
    mbBuf[7] = 0x00048004;  // Set the virtual width/height
    mbBuf[8] = 8;           // 8 byte request/reply
    mbBuf[9] = 0;           // indicate this is a request
    mbBuf[10] = 1024;        // 800 pixels wide
    mbBuf[11] = 768;        // 400 pixels high
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

    SerialPutS("Sending...\n");
    MailboxSend((uint32_t)mbBuf - 0x40000000, 8);
    SerialPutS("Receiving...\n");


    SerialPutS(".. The frame buffer is reported to be at "); SerialPutHex(mbBuf[24]); SerialPutChar('\n');
    SerialPutS(".. The frame buffer is "); SerialPutHex(mbBuf[25]); SerialPutS(" bytes long\n");

    uint16_t *fb = (uint16_t *)(mbBuf[24] + 0x40000000);
    for (int i = 10240 + 50; i < 20480 + 50; i ++) fb[i] = 0xffff;

    while(1);
}