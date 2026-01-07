#include "drivers/io.h"

// The buffer must be 16-byte aligned as only the upper 28 bits of the address can be passed via the mailbox
volatile unsigned int __attribute__((aligned(16))) mbox[36];

 
#define VIDEOCORE_MBOX  (PERIPHERAL_BASE_VA + 0x0000B880)
#define MBOX_READ       (VIDEOCORE_MBOX + 0x0)
#define MBOX_POLL       (VIDEOCORE_MBOX + 0x10)
#define MBOX_SENDER     (VIDEOCORE_MBOX + 0x14)
#define MBOX_STATUS     (VIDEOCORE_MBOX + 0x18)
#define MBOX_CONFIG     (VIDEOCORE_MBOX + 0x1C)
#define MBOX_WRITE      (VIDEOCORE_MBOX + 0x20)
#define MBOX_RESPONSE   0x80000000 
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000


unsigned int mbox_call(unsigned char ch)
{
    // 28-bit address (MSB) and 4-bit value (LSB)
    unsigned int r = ((unsigned int)((long) &mbox) &~ 0xF) | (ch & 0xF);

    // Wait until we can write
    while (mmio_read(MBOX_STATUS) & MBOX_FULL);
    
    // Write the address of our buffer to the mailbox with the channel appended
    mmio_write(MBOX_WRITE, r);

    while (1) {
        // Is there a reply?
        while (mmio_read(MBOX_STATUS) & MBOX_EMPTY);

        // Is it a reply to our message?
        if (r == mmio_read(MBOX_READ)) return mbox[1]==MBOX_RESPONSE; // Is it successful?
           
    }
    return 0;
}