#include "term.h"
#include "user_printf.h"
// 0 , 0 is top left corner
static char buffer[CHAR_X_LEN][CHAR_Y_LEN];
static uint8_t needs_update[CHAR_X_LEN][CHAR_Y_LEN];

static int cursor = 0;

static void (*clear_screen_ptr)(void);
static void (*write_buffer_ptr)(char*, uint8_t*, int , int );

void term_init(void (*clear_screen)(void), void (*write_buffer)(char* data, uint8_t* needs_update, int size_x, int size_y)){
    clear_screen_ptr = clear_screen;
    write_buffer_ptr = write_buffer;
    for(int i = 0; i < CHAR_X_LEN; i++) {
        for (int j = 0; j < CHAR_Y_LEN; j++) {
            buffer[i][j] = ' ';
        }
    }
}

void refresh_screen() {
    write_buffer_ptr((char*)buffer, (uint8_t*)needs_update, CHAR_X_LEN, CHAR_Y_LEN);
}

void write_char_to_buffer(void *p, char character) {
    if (character == '\r') {
        cursor = ((cursor / CHAR_X_LEN) + 1) * CHAR_X_LEN;
        return;
    }
    
    if (character == 127) {
        cursor--;
        int pos_x = cursor % CHAR_X_LEN;
        int pos_y = cursor / CHAR_X_LEN;
        buffer[pos_x][pos_y] = ' ';
        needs_update[pos_x][pos_y] = 1;
        return;
    }
    int pos_x = cursor % CHAR_X_LEN;
    int pos_y = cursor / CHAR_X_LEN;
    buffer[pos_x][pos_y] = character;
    needs_update[pos_x][pos_y] = 1;
    cursor++;
}

void term_printf(char * fmt, ...) {
    va_list va;
    va_start(va, fmt);
    tfp_format_user(0, write_char_to_buffer, fmt, va);
    va_end(va);
}

void clear_screen() {
    cursor = 0;
    for (int x = 0; x < CHAR_X_LEN; x++) {
        for (int y = 0; y < CHAR_Y_LEN; y++) {
            if (buffer[x][y] != ' ') { 
                buffer[x][y] = ' ';
                needs_update[x][y] = 1;
            }
        }
    }
}

uint32_t user_str_compare(char * buffer1, char * buffer2, uint32_t size) {
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

uint32_t append(char* dst, char* src) {
    int i = 0;
    int src_i = 0;
    while(dst[i] != '\0'){
        i++;
    }
    while(src[src_i] != '\0') {
        dst[i++] = src[src_i++];
    }
    return 0;
}
