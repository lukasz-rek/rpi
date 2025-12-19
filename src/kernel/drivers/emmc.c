#include "drivers/emmc.h"
#include "printf.h"
#include "drivers/mb.h"
#include "arm/util.h"
#include "drivers/io.h"

// int emmc_disable_18v() {
//     mbox[0] = 8 * 4;
//     mbox[1] = MBOX_REQUEST;
//     mbox[2] = MBOX_TAG_SET_GPIO;   
//     mbox[3] = 8;            
//     mbox[4] = 0;            
//     mbox[5] = 132;            
//     mbox[6] = 0;            
//     mbox[7] = MBOX_TAG_LAST;

//     if (!mbox_call(MBOX_CH_PROP)) {
//         printf("Failed to disable 1.8V for sd card\n");
//         return -1;
//     }    

//     // Did they process the bad boi?

//     if(!mbox[4])
    
//     return 0;
    
// }
int call_mailbox(uint32_t tag, uint32_t data_len, uint32_t* values) {
    mbox[0] = (6 + data_len) * 4; // Total size of msg
    mbox[1] = MBOX_REQUEST;
    mbox[2] = tag;
    mbox[3] = data_len * 4; // bytes we're passing 
    mbox[4] = 0; // Stores response code
    int index = 5;
    for(int i = 0; i < data_len; i++) {
        mbox[index++] = values[i];
    }
    mbox[index] = MBOX_TAG_LAST;

    // printf("mbox buffer before:\n");
    // for(int i = 0; i <= index; i++) {
    //     printf("  [%d] = 0x%x\n", i, mbox[i]);
    // }
    
    // Check if message as a whole went ok
    if(!mbox_call(MBOX_CH_PROP)) {
        printf("Entire message with tag %x failed to be processed\n", tag);
        return -1;
    }

    // printf("mbox buffer after:\n");
    // for(int i = 0; i <= index; i++) {
    //     printf("  [%d] = 0x%x\n", i, mbox[i]);
    // }

    // Check if given tag went through
    if(!(mbox[4] & 0x80000000)) {
        printf("Tag %x failed\n", tag);
        return -2;
    }
    return 0;
}

int emmc_read_block(uint32_t block, uint8_t* buffer) {
    // Wait for CMD and DAT lines to be free
    while (mmio_read(EMMC_STATUS) & 0x3) {}
    
    // Clear interrupts
    mmio_write(EMMC_INTERRUPT, 0xFFFFFFFF);
    
    // Set block size (512) and count (1)
    mmio_write(EMMC_BLKSIZECNT, (1 << 16) | 512);
    
    // Set argument (block number)
    mmio_write(EMMC_ARG1, block);
    
    // CMD17: Read single block
    // Bits: cmd(24-29), response R1(16-17), CRC(19), index(20), data present(21), direction read(4)
    uint32_t cmdtm = (17 << 24) | (2 << 16) | (1 << 19) | (1 << 20) | (1 << 21) | (1 << 4);
    mmio_write(EMMC_CMDTM, cmdtm);
    
    // Wait for command complete
    int timeout = 1000000;
    while (--timeout) {
        uint32_t irq = mmio_read(EMMC_INTERRUPT);
        if (irq & (1 << 15)) {
            printf("CMD17 error: %08X\n", irq);
            mmio_write(EMMC_INTERRUPT, irq);
            return -1;
        }
        if (irq & (1 << 0)) {
            mmio_write(EMMC_INTERRUPT, 1 << 0);
            break;
        }
    }
    if (timeout == 0) return -2;
    
    // Wait for read ready (bit 5)
    timeout = 1000000;
    while (--timeout) {
        uint32_t irq = mmio_read(EMMC_INTERRUPT);
        if (irq & (1 << 15)) {
            printf("Read error: %08X\n", irq);
            return -3;
        }
        if (irq & (1 << 5)) {
            break;
        }
    }
    if (timeout == 0) return -4;
    
    // Read 512 bytes (128 words)
    uint32_t* buf32 = (uint32_t*)buffer;
    for (int i = 0; i < 128; i++) {
        buf32[i] = mmio_read(EMMC_DATA);
    }
    
    // Wait for transfer complete (bit 1)
    timeout = 1000000;
    while (--timeout) {
        uint32_t irq = mmio_read(EMMC_INTERRUPT);
        if (irq & (1 << 1)) {
            mmio_write(EMMC_INTERRUPT, 0xFFFF);
            break;
        }
    }
    
    return 0;
}

int emmc_write_block(uint32_t block, uint8_t* buffer) {
    // Wait for CMD and DAT lines to be free
    while (mmio_read(EMMC_STATUS) & 0x3) {}
    
    // Clear interrupts
    mmio_write(EMMC_INTERRUPT, 0xFFFFFFFF);
    
    // Set block size (512) and count (1)
    mmio_write(EMMC_BLKSIZECNT, (1 << 16) | 512);
    
    // Set argument (block number)
    mmio_write(EMMC_ARG1, block);
    
    // CMD24: Write single block
    // Same as read but direction bit (4) is 0 for write
    uint32_t cmdtm = (24 << 24) | (2 << 16) | (1 << 19) | (1 << 20) | (1 << 21);
    mmio_write(EMMC_CMDTM, cmdtm);
    
    // Wait for command complete
    int timeout = 1000000;
    while (--timeout) {
        uint32_t irq = mmio_read(EMMC_INTERRUPT);
        if (irq & (1 << 15)) {
            printf("CMD24 error: %08X\n", irq);
            mmio_write(EMMC_INTERRUPT, irq);
            return -1;
        }
        if (irq & (1 << 0)) {
            mmio_write(EMMC_INTERRUPT, 1 << 0);
            break;
        }
    }
    if (timeout == 0) return -2;
    
    // Wait for write ready (bit 4)
    timeout = 1000000;
    while (--timeout) {
        uint32_t irq = mmio_read(EMMC_INTERRUPT);
        if (irq & (1 << 15)) {
            printf("Write error: %08X\n", irq);
            return -3;
        }
        if (irq & (1 << 4)) {
            break;
        }
    }
    if (timeout == 0) return -4;
    
    // Write 512 bytes (128 words)
    uint32_t* buf32 = (uint32_t*)buffer;
    for (int i = 0; i < 128; i++) {
        mmio_write(EMMC_DATA, buf32[i]);
    }
    
    // Wait for transfer complete (bit 1)
    timeout = 1000000;
    while (--timeout) {
        uint32_t irq = mmio_read(EMMC_INTERRUPT);
        if (irq & (1 << 1)) {
            mmio_write(EMMC_INTERRUPT, 0xFFFF);
            break;
        }
    }
    
    return 0;
}


int emmc_init() {

    uint32_t buffer[4];

    // Maybe turn on 34 - 39 and 48 - 53, in case bootloader fricks up 

    // Disable 1.8V -> send via mailbox
    buffer[0] = 132;
    buffer[1] = 0;

    printf("Setting power to 1.8V\n");
    if (call_mailbox(MBOX_TAG_SET_GPIO, 2, buffer)) {
        // When setting led 42 (data read led), it also returns it failed, but then lights up ??
        printf("Failed to set 1.8V power, however from testing it seems this just happens???\n");
    }

    for (volatile int i = 0; i < 1000000; i++);

    // Power on -> Mailbox to SD CARD device
    buffer[0] = 0; // SD CARD ID
    buffer[1] = 3; // 0b11, turn on and wait
    printf("Turning on sd card\n");
    if (call_mailbox(MBOX_TAG_SETPOWER, 2, buffer)) {
        printf("Failed to turn on SD CARD\n");
    }

    // Possibly read slotisr
    uint32_t ver = mmio_read(EMMC_SLOTISR_VER);
    uint32_t sdversion = (ver >> 16) & 0xff;
    uint32_t vendor = ver >> 24;
    uint32_t slot_status = ver & 0xff;
    printf("Vendor %x, SD version %x, slot status %x\n", vendor, sdversion, slot_status);
    // Seems slot status is 0??, read emmc status in case
    

    // == Begin Card reset (might fail randomly)

    // Disable clock via EMMC control 1
    uint32_t control1 = mmio_read(EMMC_CONTROL1);
    control1 |= (1 << 24);
	control1 &= ~(1 << 2);
	control1 &= ~(1 << 0);
    mmio_write(EMMC_CONTROL1, control1);

    int timeout = 1000000;
    while ((mmio_read(EMMC_CONTROL1) & (7 << 24)) != 0) {
        if (--timeout <= 0) {
            printf("Controller did not reset properly\n");
            return -1;
        }
    }   
    // Set 3.3V via CONTROL 0
    uint32_t control0 = mmio_read(EMMC_CONTROL0);
    control0 |= 0x0F << 8;
    mmio_write(EMMC_CONTROL0, control0);

    for (volatile int i = 0; i < 1000000; i++);


    // Check for valid card via EMMC STATUS

    uint32_t present = mmio_read(EMMC_STATUS);
    printf("Present state: 0x%x\n", present);
    // According to SD HOST spec simplified
    printf("Card inserted: %d\n", (present >> 16) & 1);

    // Clear control2

    mmio_write(EMMC_CONTROL2, 0);

    // Get clock rate

    uint32_t base_clock = emmc_get_base_clock();

    // Set divisor to sth slow 400kHz

    uint32_t clock_divider = base_clock / (2 * 400000) ;

    control1 = mmio_read(EMMC_CONTROL1);
    control1 |= 1; // clock enable

    control1 &= ~(0x3FF << 6);
	control1 |= clock_divider;
	control1 &= ~(0xF << 16);
    control1 |= (11 << 16);

    mmio_write(EMMC_CONTROL1, control1);
    // Wait for it to stabilise
    timeout = 1000000;
    while (!(mmio_read(EMMC_CONTROL1) & 2)) {
        if (--timeout <= 0) {
            printf("Clock did not stabilise\n");
            return -1;
        }
    }   
    // Enable SD Clock and mask off isr's
    for (volatile int i = 0; i < 1000000; i++);
    control1 = mmio_read(EMMC_CONTROL1);
    control1 |= 4;
    mmio_write(EMMC_CONTROL1, control1);
    for (volatile int i = 0; i < 1000000; i++);

    mmio_write(EMMC_IRPT_EN, 0);           // Disable interrupts to CPU
    mmio_write(EMMC_INTERRUPT, 0xffffffff); // Clear any pending
    uint32_t irpt_mask = 0xffffffff & ~(1 << 8);  // All except card interrupt
    mmio_write(EMMC_IRPT_MASK, irpt_mask);

    for (volatile int i = 0; i < 1000000; i++);

    // Send CMD0 to go idle 
    emmc_send_command(0, 0);

    // Send CMD8 to check supported voltage, might fail if lower than v2

    emmc_send_command(8, 0x1aa);

    // Coould send CMD5 to check but I know my card ain't it
    emmc_send_command(55, 0); // We need those before sending 41
    emmc_send_command(41, 0x40FF8000);
    //  Send ACMD41 
    while(!((mmio_read(EMMC_RESP0) >> 31) & 1)) {
        emmc_send_command(55, 0); // We need those before sending 41
        emmc_send_command(41, 0x40FF8000);
    }
    // We should read capabilites, but I'm just gonna send some stuff

    // Send CMD2 to get card info

    sd_cid_t info;
    emmc_send_cmd2(&info);
    print_cid(&info);
    // Profit

    // Get relative card access
    emmc_send_command(3, 0);
    uint32_t rca = mmio_read(EMMC_RESP0) & 0xFFFF0000;  // Upper 16 bits
    printf("RCA: %08X\n", rca);

    emmc_send_command(7, rca);

    emmc_send_command(16, 512);


    uint8_t test_buf[512];
    uint8_t buf[512];


    if (emmc_read_block(0, test_buf) == 0) {
    printf("Read success! First bytes: %02X %02X %02X %02X\n",
           test_buf[0], test_buf[1], test_buf[2], test_buf[3]);
    
    // Check for MBR signature at bytes 510-511
    printf("MBR signature: %02X %02X\n", test_buf[510], test_buf[511]);
    // Should be 0x55 0xAA for valid MBR
}   
    uint32_t test_block = 100000;

// First read - see what's there
printf("Reading block %d...\n", test_block);
emmc_read_block(test_block, buf);
printf("Before: %02X %02X %02X %02X\n", buf[0], buf[1], buf[2], buf[3]);

// Write something recognizable
buf[0] = 0xDE;
buf[1] = 0xAD;
buf[2] = 0xBE;
buf[3] = 0xEF;

printf("Writing...\n");
emmc_write_block(test_block, buf);
printf("Done!\n");
   

    return 0;
}

int emmc_send_command(uint32_t cmd, uint32_t arg) {
    int timeout = 1000000;

    printf("  CMD%d: waiting for CMD_INHIBIT clear\n", cmd);
    while ((mmio_read(EMMC_STATUS) & 1) && --timeout) {}
    if (timeout == 0) {
        printf("  CMD%d: CMD_INHIBIT timeout\n", cmd);
        return -1;
    }
    printf("  CMD%d: CMD_INHIBIT clear, STATUS=%08X\n", cmd, mmio_read(EMMC_STATUS));

    mmio_write(EMMC_INTERRUPT, 0xFFFFFFFF);
    mmio_write(EMMC_ARG1, arg);

    uint32_t cmdtm = (cmd << 24);
    switch (cmd) {
        case 0:  // GO_IDLE - no response
            break;
        case 2:  // ALL_SEND_CID - R2 (136-bit)
        case 9:  // SEND_CSD
        case 10: // SEND_CID
            cmdtm |= (1 << 16);  // Response type 01
            break;
        case 7:  // SELECT_CARD - R1b (48-bit with busy)
            cmdtm |= (3 << 16);  // Response type 11
            cmdtm |= (1 << 19);  // CRC check
            cmdtm |= (1 << 20);  // Index check
            break;
        case 8:  // SEND_IF_COND - R7 (48-bit)
        case 55: // APP_CMD - R1
            cmdtm |= (2 << 16);  // Response type 10
            cmdtm |= (1 << 19);  // CRC check
            cmdtm |= (1 << 20);  // Index check
            break;
        case 41: // SD_SEND_OP_COND - R3 (48-bit, no CRC/index)
            cmdtm |= (2 << 16);  // Response type 10
            // No CRC or index check for R3
            break;
        default:
            cmdtm |= (2 << 16);  // Default to 48-bit response
            break;
    }

    printf("  CMD%d: writing CMDTM=%08X\n", cmd, cmdtm);
    mmio_write(EMMC_CMDTM, cmdtm);

    timeout = 1000000;
    int loops = 0;
    while (--timeout) {
        uint32_t irq = mmio_read(EMMC_INTERRUPT);
        loops++;

        if (loops == 1 || loops == 1000 || loops == 100000) {
            // printf("  CMD%d: loop %d, IRQ=%08X\n", cmd, loops, irq);
        }

        if (irq & (1 << 15)) {
            printf("  CMD%d error, INTERRUPT: %08X\n", cmd, irq);
            mmio_write(EMMC_INTERRUPT, irq);
            return -2;
        }
        if (irq & (1 << 0)) {
            printf("  CMD%d: complete after %d loops\n", cmd, loops);
            mmio_write(EMMC_INTERRUPT, 1 << 0);
            return 0;
        }
    }

    printf("  CMD%d timeout after %d loops, final IRQ=%08X\n", cmd, loops, mmio_read(EMMC_INTERRUPT));
    return -3;
}

int emmc_reset() {
    mmio_write(EMMC_CONTROL1, 0x01000000);  // SRST_HC

    int timeout = 10000;
    while ((mmio_read(EMMC_CONTROL1) & 0x01000000) && timeout--) {}
    if (timeout <= 0) return -1;

    mmio_write(EMMC_INTERRUPT, 0xFFFFFFFF);
    mmio_write(EMMC_IRPT_MASK, 0xFFFFFFFF);
    mmio_write(EMMC_IRPT_EN, 0xFFFFFFFF);

    return 0;
}

int emmc_power_on() {
    mbox[0] = 8 * 4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETPOWER;
    mbox[3] = 8;
    mbox[4] = 0;
    mbox[5] = MBOX_TAG_ID_SD;
    mbox[6] = 3;  // ON(1) | WAIT(2)
    mbox[7] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP)) {
        if (mbox[6] & 0x2) {
            return -1;
        }
        if (!(mbox[6] & 0x1)) {
            return -2;
        }
        return 0;
    }

    return -3;
}

// int emmc_set_clock(uint32_t target_hz) {
//     uint32_t ctrl1 = mmio_read(EMMC_CONTROL1);
//     ctrl1 &= ~(1 << 2);  // CLK_EN off
//     mmio_write(EMMC_CONTROL1, ctrl1);

//     uint32_t base_clock = emmc_get_base_clock();
//     uint32_t divider = base_clock / (2 * target_hz);

//     printf("Clock: base=%d, target=%d, divider=%d\n", base_clock, target_hz, divider);

//     ctrl1 &= ~0xFFE0;
//     ctrl1 |= (divider & 0xFF) << 8;
//     ctrl1 |= ((divider >> 8) & 0x3) << 6;
//     ctrl1 |= (1 << 0);  // Enable internal clock
//     mmio_write(EMMC_CONTROL1, ctrl1);

//     int timeout = 10000;
//     while (!(mmio_read(EMMC_CONTROL1) & (1 << 1)) && --timeout) {}
//     if (timeout == 0) {
//         printf("Clock stable timeout\n");
//         return -1;
//     }

//     ctrl1 = mmio_read(EMMC_CONTROL1);
//     ctrl1 |= (1 << 2);  // CLK_EN on
//     mmio_write(EMMC_CONTROL1, ctrl1);

//     return 0;
// }

uint32_t emmc_get_base_clock() {
    mbox[0] = 8 * 4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_GETCLKRATE;
    mbox[3] = 8;
    mbox[4] = 4;
    mbox[5] = 0x0c;  // Clock ID 1 = EMMC
    mbox[6] = 0;
    mbox[7] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP)) {
        printf("Base clock from mailbox: %d Hz\n", mbox[6]);
        return mbox[6];
    }
    return 0;
}

int emmc_send_cmd2(sd_cid_t *cid) {
    while (mmio_read(EMMC_STATUS) & 1) {}

    mmio_write(EMMC_INTERRUPT, 0xFFFFFFFF);
    mmio_write(EMMC_ARG1, 0);
    mmio_write(EMMC_CMDTM, (2 << 24) | (1 << 16));

    int timeout = 1000000;
    while (--timeout) {
        uint32_t irq = mmio_read(EMMC_INTERRUPT);
        if (irq & (1 << 15)) {
            printf("CMD2 error: %08X\n", irq);
            return -1;
        }
        if (irq & (1 << 0)) {
            mmio_write(EMMC_INTERRUPT, 1 << 0);
            break;
        }
    }
    if (timeout == 0) return -2;

    uint32_t resp0 = mmio_read(EMMC_RESP0);
    uint32_t resp1 = mmio_read(EMMC_RESP1);
    uint32_t resp2 = mmio_read(EMMC_RESP2);
    uint32_t resp3 = mmio_read(EMMC_RESP3);
    
    // printf("RESP0: %08X\n", resp0);
    // printf("RESP1: %08X\n", resp1);
    // printf("RESP2: %08X\n", resp2);
    // printf("RESP3: %08X\n", resp3);

    printf("Direct chars: %c %c %c %c %c\n",
    (resp2 >> 24) & 0xFF,
    (resp2 >> 16) & 0xFF,
    (resp2 >> 8) & 0xFF,
    resp2 & 0xFF,
    (resp1 >> 24) & 0xFF);


    cid->mid = (resp3 >> 16) & 0xFF;
    cid->oid = ((resp3 >> 8) & 0xFF) << 8 | (resp3 & 0xFF);
    cid->pnm[0] = (resp2 >> 24) & 0xFF;
    cid->pnm[1] = (resp2 >> 16) & 0xFF;
    cid->pnm[2] = (resp2 >> 8) & 0xFF;
    cid->pnm[3] = resp2 & 0xFF;
    cid->pnm[4] = (resp1 >> 24) & 0xFF;
    cid->pnm[5] = '\0';  // Add null terminator!
    cid->prv = (resp1 >> 16) & 0xFF;
    cid->psn = ((resp1 & 0xFFFF) << 16) | ((resp0 >> 16) & 0xFFFF);
    cid->mdt = (resp0 >> 4) & 0xFFF;

    return 0;
}

void print_cid(sd_cid_t *cid) {
    printf("SD Card CID:\n");
    printf("  Manufacturer ID: %02X\n", cid->mid);
    printf("  OEM ID: %04X\n", cid->oid);
    printf("  Product Name: %.*s\n", 5, cid->pnm);
    printf("  Revision: %d.%d\n", (cid->prv >> 4), (cid->prv & 0xF));
    printf("  Serial Number: %08X\n", cid->psn);
    printf("  Manufacturing Date: %02d/%04d\n", cid->mdt & 0xF, 2000 + ((cid->mdt >> 4) & 0xFFF));
}

// void print_cid_info(sd_cid_t *cid) {
//     print_cid(cid);
// }