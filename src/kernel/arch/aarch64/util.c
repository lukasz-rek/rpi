#include "arm/util.h"
#include "printf.h"


void mmio_write(unsigned long reg, unsigned int val) { 
    *(volatile unsigned int *)reg = val;  // Use unsigned int (32-bit)
    asm volatile("dsb sy");  // Memory barrier for Device memory
}

unsigned int mmio_read(unsigned long reg) { 
    asm volatile("dsb sy");  // Memory barrier before read
    return *(volatile unsigned int *)reg;  // Use unsigned int (32-bit)
}
void delay(volatile int ms) {
        // Yes, it's this stupid for now. lmao
        while (ms > 0) {ms--;}
}

void set_pgd(unsigned long pgd) {
        __asm__ volatile (
                "msr ttbr0_el1, %0\n\t"
                "tlbi vmalle1is\n\t" // We need to clear this buffer
                "dsb ish\n\t"
                "isb"
                :
                : "r" (pgd)
                : "memory"
        );
}

unsigned long get_pgd(void) {
        unsigned long result;
        __asm__ volatile (
                "mov x1, #0\n\t"
                "ldr %0, [x1]\n\t"
                "mov %0, #0x1000\n\t"
                "msr ttbr0_el1, %0\n\t"
                "ldr %0, [x1]"
                : "=r" (result)
                :
                : "x1", "memory"
        );
        return result;
}

uint32_t str_compare(char * buffer1, char * buffer2, uint32_t size) {
        for(int i = 0; i < size; i++) {
                // if (buffer1[i] == '\0' && buffer2[i] == '\0') 
                //         return 0;
                if(buffer1[i] != buffer2[i]) {
                        // printf("Mismatch on %d with chars %c and %c\n", i, buffer1[i], buffer2[i]);
                        return 1;
                }
        }
        return 0;
}

