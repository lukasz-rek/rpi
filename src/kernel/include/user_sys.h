#ifndef	_USER_SYS_H
#define	_USER_SYS_H
#include <stdarg.h>
#include <stdint.h>
#include "fs/fat32_files.h"




// Usage:


void call_sys_write(char * fmt, ...);
int call_sys_fork();
char call_sys_uart_read_char();
void call_sys_exit();
void call_sys_delay_ticks(long ticks);
void call_sys_set_prio(int prio);
void call_sys_write_char(void *p, char character);
void call_sys_register_for_isr(int isr_num);
void call_sys_clear_screen();
void call_sys_write_screen_buffer(char *buf, uint8_t* needs_update, int size_x, int size_y);

// Filesystem stuff
/**
 * @brief Gives back file handler (virt_node) for requested path if possible
 * @param path pointer to path string
 * @param path_len length of the path
 * @return Node of the file, invalid if cluster_number == 0
 */
virt_node_t call_sys_get_file_handle_by_path(char* path, int path_len);
/**
 * @brief Lists files under the given file handler, if it is a directory
 * @param parent_dir requested parent directory
 * @param retrieved_paths pointer to pre-allocated spot for resulting paths
 * @param idx amount of returned paths
 * @return 0 on success, -1 on failure
 */
int call_sys_list_files(virt_node_t parent_dir, virt_node_t retrieved_paths[64], int* idx);
/**
 * @brief Does a single 512 byte read from file, why 512? I can't be bothered to decouple this from emmc read
 * @param buffer has to have len 512
 */
int call_sys_read_from_file(virt_node_t file, uint8_t* buffer, uint32_t offset);


extern void user_delay ( unsigned long);
extern unsigned long get_sp ( void );
extern unsigned long get_pc ( void );

#endif  /*_USER_SYS_H */