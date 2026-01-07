#include "printf.h"
#include "sched.h"
#include "mm.h"
#include "printf.h"
#include "drivers/io.h"
#include "drivers/fb.h"
#include <stdint.h>
#include "fs/fat32.h"


void sys_write(char * fmt, ...){
	// Yeah, bit goofy, but works and this is LEARNING OS
	va_list va;
	va_start(va, fmt);
	tfp_format(0, uart_writeChar, fmt, va);
	va_end(va);
}

int sys_fork(){
	return copy_process(0, 0, 0, 0);
}

void sys_exit(){
	exit_process();
}

void sys_delay_ticks(long ticks){
	delay_ticks(ticks);
}

char sys_uart_read_char(char * buf) {
	return uart_read_from_fifo(buf);
}

void sys_set_prio(int prio){
	set_task_prio(prio);
}


void sys_register_for_isr(int isr_num) {
	register_for_isr(isr_num);
}

void sys_write_screen_buffer(char *buf, uint8_t* needs_update, int size_x, int size_y) {
    for (int x = 0; x < size_x; x++) {
        for (int y = 0; y < size_y; y++) {
			if (needs_update[x * size_y + y]){
            	char c = buf[x * size_y + y];    
            	drawChar(c, x * 8, y * 8, 0x0f);
				needs_update[x * size_y + y] = 0;
			}
        }
    }
}

void sys_clear_screen() {
	drawRect(0, 0, 1920, 1080, 0, 1);
}

// Returning structs no bueno :/
int get_file_handle_by_path(char* path, int path_len, virt_node_t* ret) {
	*ret = get_cluster_by_path(path, path_len);
	return 1;
}

int list_files(virt_node_t *parent_dir, virt_node_t retrieved_paths[64], int* idx) {
	*idx = 0;
	return parse_cluster(parent_dir->cluster_number, retrieved_paths, idx);
}

int read_from_file(virt_node_t* file, uint8_t* buffer, uint32_t offset) {
	// TODO: both here and in list_files I could add some checks if file is file and dir a dir
	// buuuut nah
	return read_from_fat32_file(file, buffer, offset);
}

unsigned long get_time_in_ticks() {
	return get_ticks_since_start();
}

void * const sys_call_table[] = {	sys_write, sys_fork, sys_exit, sys_delay_ticks, 
									sys_set_prio, sys_uart_read_char, sys_register_for_isr, sys_write_screen_buffer, sys_clear_screen,
									get_file_handle_by_path, list_files, read_from_file, get_time_in_ticks};