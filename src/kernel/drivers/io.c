// GPIO
#include "arm/util.h"
#include "drivers/io.h"
#include "printf.h"

    // Why this peripheral base, you might ask?
    // in manual register base is 0x7e20_0000, which is legacy address, but wait a minute, mister

    // our core is now operating in low peripheral mode, where main peripherals
    // are from 0x0_FC00_0000
    // in legacy address, start of periph is from 0x7c00_0000 (also legacy are the ones used in manual)
    // This is difference of 200_0000
    // so we add it to our to get our peripheral base at 0xFe00_0000
    // then from gpfsel0 we always add at least 20_0000 as you can see below
#define GPFSEL0         PERIPHERAL_BASE_VA + 0x200000
#define GPSET0          PERIPHERAL_BASE_VA + 0x20001C
#define GPCLR0          PERIPHERAL_BASE_VA + 0x200028
#define GPPUPPDN0       PERIPHERAL_BASE_VA + 0x2000E4

enum {
    GPIO_MAX_PIN       = 53, 
    GPIO_FUNCTION_ALT5 = 2,
};

enum {
    Pull_None = 0,
};

static char buffer[RX_FIFO_LEN];
static int buf_len = 0; 


unsigned int gpio_call(unsigned int pin_number, unsigned int value, unsigned long base, unsigned int field_size, unsigned int field_max) {
    unsigned int field_mask = (1 << field_size) - 1;
  
    if (pin_number > field_max) return 0;
    if (value > field_mask) return 0; 

    unsigned int num_fields = 32 / field_size;
    unsigned long reg = base + ((pin_number / num_fields) * 4);
    unsigned int shift = (pin_number % num_fields) * field_size;

    unsigned int curval = mmio_read(reg);
    curval &= ~(field_mask << shift);
    curval |= value << shift;
    mmio_write(reg, curval);

    return 1;
}

unsigned int gpio_set     (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPSET0, 1, GPIO_MAX_PIN); }
unsigned int gpio_clear   (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPCLR0, 1, GPIO_MAX_PIN); }
unsigned int gpio_pull    (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPPUPPDN0, 2, GPIO_MAX_PIN); }
unsigned int gpio_function(unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPFSEL0, 3, GPIO_MAX_PIN); }

void gpio_useAsAlt5(unsigned int pin_number) {
    gpio_pull(pin_number, Pull_None);
    gpio_function(pin_number, GPIO_FUNCTION_ALT5);
}

// UART

void uart_debug_fifo_status(void) {
    int stat = mmio_read(AUX_MU_STAT_REG);
    int rx_fifo_level = (stat >> 16) & 0x0F;  // Bits 19:16
    int tx_fifo_level = (stat >> 24) & 0x0F;  // Bits 27:24
    int iir = mmio_read(AUX_MU_IIR_REG);
    int aux_irq = mmio_read(AUX_IRQ) & 0x01;
    
    printf("STAT_REG: 0x%08x\n", stat);
    printf("RX FIFO level: %d\n", rx_fifo_level);
    printf("TX FIFO level: %d\n", tx_fifo_level);
    printf("IIR pending bit: %d\n", iir & 1);
    printf("AUX IRQ: %d\n", aux_irq);
}

void uart_init() {
    mmio_write(AUX_ENABLES, 1); //enable UART1
    mmio_write(AUX_MU_IER_REG, 0); // clear interrupt register
    mmio_write(AUX_MU_CNTL_REG, 0); // clear cntl register, set it later
    mmio_write(AUX_MU_LCR_REG, 3); //8 bits
    mmio_write(AUX_MU_MCR_REG, 0);  // sets rts to high?
    mmio_write(AUX_MU_IIR_REG, 0x05); // clear & enable FIFO
    mmio_write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200));
    gpio_useAsAlt5(14);
    gpio_useAsAlt5(15);
    mmio_write(AUX_MU_IER_REG, 1); //RX isr ON, TX isr OFF
    // Their docs are wrong, it seems bit 0 switches RX isr, instead of bit 1 as they say
    mmio_write(AUX_MU_CNTL_REG, 3); //enable RX/TX
    


}

unsigned int uart_isWriteByteReady() { return mmio_read(AUX_MU_LSR_REG) & 0x20; }

unsigned int uart_isReadByteReady() {return mmio_read(AUX_MU_LSR_REG) & 0x01; }

void uart_writeByteBlockingActual(unsigned char ch) {
    while (!uart_isWriteByteReady()); 
    mmio_write(AUX_MU_IO_REG, (unsigned int)ch);
}

void uart_writeText(char *buffer) {
    while (*buffer) {
       if (*buffer == '\n') uart_writeByteBlockingActual('\r');
       uart_writeByteBlockingActual(*buffer++);
    }
}

void uart_writeChar(void* p, char character) {
    if (character == '\n') uart_writeByteBlockingActual('\r');
    uart_writeByteBlockingActual(character);
}

void uart_readChar() {
    if ( uart_isReadByteReady()) {
        buffer[buf_len++] = (char) mmio_read(AUX_MU_IO_REG); // upper bits, should be clear 
    } else {
        return; // Do nothing
    }
}

int uart_read_from_fifo(char* buf) {
    // Remember buf has to be atleast RX_BUF_LEN size
    disable_irq();
    for(int i = 0; i < buf_len; i++) {
        buf[i] = buffer[i];
    }
    int ret = buf_len;
    buf_len = 0;
    enable_irq();
    return ret;
}

