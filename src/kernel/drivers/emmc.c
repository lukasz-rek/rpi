#include "drivers/emmc.h"
#include "printf.h"
#include "drivers/mb.h"
#include "arm/util.h"
#include "drivers/io.h"


int emmc_disable_18v() {
    mbox[0] = 8 * 4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = 0x00038041;   // SET GPIO
    mbox[3] = 8;            // Buffer size
    mbox[4] = 8;            // Request size
    mbox[5] = 132;            // PIN ID
    mbox[6] = 0;            // Requested state
    mbox[7] = MBOX_TAG_LAST;

    if (!mbox_call(MBOX_CH_PROP)) {
        printf("Failed to disable 1.8V for sd card\n");
        return -1;
    }
    printf("Response: mbox[4]=%08X mbox[5]=%d mbox[6]=%d\n", 
           mbox[4], mbox[5], mbox[6]);
    
    // mbox[4] should have bit 31 set (response flag) on success
    if (!(mbox[4] & 0x80000000)) {
        printf("Tag not processed\n");
        return -1;
    }
    
    return 0;
    
}

int emmc_init() {

    // Disable 1.8V -> send via mailbox

    // Power on -> Mailbox to SD CARD device

        // Possibly read slotisr

    // == Begin Card reset (might fail randomly)

    // Write to EMMC CONTROL 1

    // Set 3.3V via CONTROL 0

    // Check for valid card via EMMC STATUS

    // Clear control2

    // Get clock rate

    // Set divisor to sth slow 400kHz

    // Wait for it to stabilise

    // Enable SD Clock and mask off isr's

    // Send CMD0 to go idle 

    // Send CMD8 to check supported voltage, might fail if lower than v2

        // Coould send CMD5 to check but I know my card ain't it
        
    //  Send ACMD41 

        // Potentially increase clock, switch voltage to 1.8V

    // Send CMD2 to get card info

    // Profit
   

    return 0;
}







// int emmc_send_command(uint32_t cmd, uint32_t arg) {
//     int timeout = 1000000;

//     printf("  CMD%d: waiting for CMD_INHIBIT clear\n", cmd);
//     while ((mmio_read(EMMC_STATUS) & (1 << 1)) && --timeout) {}
//     if (timeout == 0) {
//         printf("  CMD%d: CMD_INHIBIT timeout\n", cmd);
//         return -1;
//     }
//     printf("  CMD%d: CMD_INHIBIT clear, STATUS=%08X\n", cmd, mmio_read(EMMC_STATUS));

//     mmio_write(EMMC_INTERRUPT, 0xFFFFFFFF);
//     mmio_write(EMMC_ARG1, arg);

//     uint32_t cmdtm = (cmd << 24);
//     switch (cmd) {
//         case 0:
//             break;
//         case 2:
//         case 9:
//         case 10:
//             cmdtm |= (1 << 16);
//             break;
//         case 41:
//             cmdtm |= (2 << 16);
//             break;
//         default:
//             cmdtm |= (2 << 16);
//             break;
//     }

//     printf("  CMD%d: writing CMDTM=%08X\n", cmd, cmdtm);
//     mmio_write(EMMC_CMDTM, cmdtm);

//     timeout = 1000000;
//     int loops = 0;
//     while (--timeout) {
//         uint32_t irq = mmio_read(EMMC_INTERRUPT);
//         loops++;

//         if (loops == 1 || loops == 1000 || loops == 100000) {
//             printf("  CMD%d: loop %d, IRQ=%08X\n", cmd, loops, irq);
//         }

//         if (irq & (1 << 15)) {
//             printf("  CMD%d error, INTERRUPT: %08X\n", cmd, irq);
//             mmio_write(EMMC_INTERRUPT, irq);
//             return -2;
//         }
//         if (irq & (1 << 0)) {
//             printf("  CMD%d: complete after %d loops\n", cmd, loops);
//             mmio_write(EMMC_INTERRUPT, 1 << 0);
//             return 0;
//         }
//     }

//     printf("  CMD%d timeout after %d loops, final IRQ=%08X\n", cmd, loops, mmio_read(EMMC_INTERRUPT));
//     return -3;
// }

// int emmc_reset() {
//     mmio_write(EMMC_CONTROL1, 0x01000000);  // SRST_HC

//     int timeout = 10000;
//     while ((mmio_read(EMMC_CONTROL1) & 0x01000000) && timeout--) {}
//     if (timeout <= 0) return -1;

//     mmio_write(EMMC_INTERRUPT, 0xFFFFFFFF);
//     mmio_write(EMMC_IRPT_MASK, 0xFFFFFFFF);
//     mmio_write(EMMC_IRPT_EN, 0xFFFFFFFF);

//     return 0;
// }

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

// uint32_t emmc_get_base_clock() {
//     mbox[0] = 8 * 4;
//     mbox[1] = MBOX_REQUEST;
//     mbox[2] = MBOX_TAG_GETCLKRATE;
//     mbox[3] = 8;
//     mbox[4] = 4;
//     mbox[5] = 0x0c;  // Clock ID 1 = EMMC
//     mbox[6] = 0;
//     mbox[7] = MBOX_TAG_LAST;

//     if (mbox_call(MBOX_CH_PROP)) {
//         printf("Base clock from mailbox: %d Hz\n", mbox[6]);
//         return mbox[6];
//     }
//     return 0;
// }

// int emmc_send_cmd2(sd_cid_t *cid) {
//     while (mmio_read(EMMC_STATUS) & (1 << 1)) {}

//     mmio_write(EMMC_INTERRUPT, 0xFFFFFFFF);
//     mmio_write(EMMC_ARG1, 0);
//     mmio_write(EMMC_CMDTM, (2 << 24) | (1 << 16));

//     int timeout = 1000000;
//     while (--timeout) {
//         uint32_t irq = mmio_read(EMMC_INTERRUPT);
//         if (irq & (1 << 15)) {
//             printf("CMD2 error: %08X\n", irq);
//             return -1;
//         }
//         if (irq & (1 << 0)) {
//             mmio_write(EMMC_INTERRUPT, 1 << 0);
//             break;
//         }
//     }
//     if (timeout == 0) return -2;

//     printf("RESP0: %08X\n", mmio_read(EMMC_RESP0));
//     printf("RESP1: %08X\n", mmio_read(EMMC_RESP0 + 4));
//     printf("RESP2: %08X\n", mmio_read(EMMC_RESP0 + 8));
//     printf("RESP3: %08X\n", mmio_read(EMMC_RESP0 + 12));

//     return 0;
// }

// void print_cid(sd_cid_t *cid) {
//     printf("SD Card CID:\n");
//     printf("  Manufacturer ID: %02X\n", cid->mid);
//     printf("  OEM ID: %04X\n", cid->oid);
//     printf("  Product Name: %.*s\n", 5, cid->pnm);
//     printf("  Revision: %d.%d\n", (cid->prv >> 4), (cid->prv & 0xF));
//     printf("  Serial Number: %08X\n", cid->psn);
//     printf("  Manufacturing Date: %02d/%04d\n", cid->mdt & 0xF, 2000 + ((cid->mdt >> 4) & 0xFFF));
// }

// void print_cid_info(sd_cid_t *cid) {
//     print_cid(cid);
// }