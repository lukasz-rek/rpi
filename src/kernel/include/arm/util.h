#ifndef	_UTILS_H
#define	_UTILS_H
#include "stdint.h"

extern int get_el (void );
void mmio_write(unsigned long reg, unsigned int val);
unsigned int mmio_read(unsigned long reg);
extern void enable_irq( void);
extern void disable_irq(void);
void delay(int ms);
uint32_t str_compare(char * buffer1, char * buffer2, uint32_t size);
extern void set_pgd(unsigned long pgd);
extern unsigned long get_pgd();

#endif  /*_UTILS_H */