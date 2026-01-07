#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#define CHAR_X_LEN (1920/8)
#define CHAR_Y_LEN (1080/8)


uint32_t user_str_compare(char * buffer1, char * buffer2, uint32_t size);
uint32_t append(char* buffer1, char* buffer2);

void term_printf(char * fmt, ...);
void term_init(void (*clear_screen) (void), void (*write_buffer)(char *, uint8_t*, int, int ));
void refresh_screen();
void clear_screen();