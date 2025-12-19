#include "mm.h"

#include "stdint.h"

typedef struct {
    uint8_t mid;        // Manufacturer ID
    uint16_t oid;       // OEM/Application ID
    char pnm[6];        // Product name (5 chars + null terminator)
    uint8_t prv;        // Product revision
    uint32_t psn;       // Product serial number
    uint16_t mdt;       // Manufacturing date
} sd_cid_t;


// Values taken from circle repository https://github.com/rsta2/circle
// as sadly there ain't much good enough docs :'(

// We're using EMMC2
#define EMMC_BASE ( PERIPHERAL_BASE + VA_START + 0x340000)
#define EMMC_CONTROL1   (EMMC_BASE + 0x2C)
#define EMMC_INTERRUPT  (EMMC_BASE + 0x30)
#define EMMC_STATUS     (EMMC_BASE + 0x24)
#define EMMC_ARG1		(EMMC_BASE + 0x08)
#define EMMC_CMDTM		(EMMC_BASE + 0x0C)
#define EMMC_RESP0		(EMMC_BASE + 0x10)
#define EMMC_IRPT_MASK  (EMMC_BASE + 0x34)
#define EMMC_IRPT_EN    (EMMC_BASE + 0x38)


int emmc_init();
int emmc_power_on();
int emmc_reset();
int emmc_set_clock(uint32_t clock_hz);
int emmc_print_card_info();
int emmc_send_command(uint32_t cmd, uint32_t arg);

uint32_t emmc_get_base_clock();

void print_cid(sd_cid_t* cid);
int emmc_send_cmd2(sd_cid_t *cid);