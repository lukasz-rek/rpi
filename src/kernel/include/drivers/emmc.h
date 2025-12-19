#include "mm.h"

#include "stdint.h"

typedef struct __attribute__((packed)) {
    uint8_t mid;
    uint16_t oid;
    char pnm[6];
    uint8_t prv;
    uint32_t psn;
    uint16_t mdt;
} sd_cid_t;


// Values taken from circle repository https://github.com/rsta2/circle
// as sadly there ain't much good enough docs :'(

// We're using EMMC2
#define EMMC_BASE ( PERIPHERAL_BASE + VA_START + 0x340000)
#define EMMC_CONTROL0       (EMMC_BASE + 0x28)
#define EMMC_CONTROL1       (EMMC_BASE + 0x2C)
#define EMMC_CONTROL2       (EMMC_BASE + 0x3C)
#define EMMC_INTERRUPT      (EMMC_BASE + 0x30)
#define EMMC_STATUS         (EMMC_BASE + 0x24)
#define EMMC_ARG1		    (EMMC_BASE + 0x08)
#define EMMC_CMDTM		    (EMMC_BASE + 0x0C)
#define EMMC_RESP0		    (EMMC_BASE + 0x10)
#define EMMC_RESP1		    (EMMC_BASE + 0x14)
#define EMMC_RESP2	    	(EMMC_BASE + 0x18)
#define EMMC_RESP3		    (EMMC_BASE + 0x1C)
#define EMMC_IRPT_MASK      (EMMC_BASE + 0x34)
#define EMMC_IRPT_EN        (EMMC_BASE + 0x38)
#define EMMC_SLOTISR_VER    (EMMC_BASE + 0xFC)
#define EMMC_BLKSIZECNT		(EMMC_BASE + 0x04)
#define EMMC_DATA		    (EMMC_BASE + 0x20)



int emmc_init();
int emmc_power_on();
int emmc_reset();
int emmc_set_clock(uint32_t clock_hz);
int emmc_print_card_info();
int emmc_send_command(uint32_t cmd, uint32_t arg);

uint32_t emmc_get_base_clock();

void print_cid(sd_cid_t* cid);
int emmc_send_cmd2(sd_cid_t *cid);