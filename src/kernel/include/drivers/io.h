#include "mm.h"

#define PERIPHERAL_BASE_VA (PERIPHERAL_BASE + VA_START)
enum {
    AUX_BASE        = PERIPHERAL_BASE_VA + 0x215000,
    AUX_IRQ         = AUX_BASE,
    AUX_ENABLES     = AUX_BASE + 4,
    AUX_MU_IO_REG   = AUX_BASE + 64,
    AUX_MU_IER_REG  = AUX_BASE + 68,
    AUX_MU_IIR_REG  = AUX_BASE + 72,
    AUX_MU_LCR_REG  = AUX_BASE + 76,
    AUX_MU_MCR_REG  = AUX_BASE + 80,
    AUX_MU_LSR_REG  = AUX_BASE + 84,
    AUX_MU_STAT_REG = AUX_BASE + 0x64,
    // LSR shows data status, 5th bit is whether fifo can accept at least one char
    // 0th bit is set to 1 if at least 1 symbol in
    AUX_MU_CNTL_REG = AUX_BASE + 96,
    AUX_MU_BAUD_REG = AUX_BASE + 104,
    AUX_UART_CLOCK  = 500000000, // cpu freq
};

#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK/(baud*8))-1)
#define RX_FIFO_LEN 50
void uart_debug_fifo_status();
void uart_init();
void uart_writeText(char *buffer);

unsigned int gpio_pull(unsigned int pin_number, unsigned int value);
unsigned int gpio_function(unsigned int pin_number, unsigned int value);

// pointer unused but added so it works with printf lib
void uart_writeChar(void* p, char character);
void uart_readChar();
int uart_read_from_fifo(char* buf);

void mmio_write(unsigned long reg, unsigned int val);
unsigned int mmio_read(unsigned long reg);