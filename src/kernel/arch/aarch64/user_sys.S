.section ".text.user"



.macro SYSCALL_WRAPPER name, syscall_num
.globl call_sys_\name
call_sys_\name:
    mov w8, #\syscall_num
    svc #0
    ret
.endm


.globl user_delay
user_delay:
	subs x0, x0, #1
	bne user_delay
	ret


SYSCALL_WRAPPER write, 0
SYSCALL_WRAPPER fork, 1
SYSCALL_WRAPPER exit, 2
SYSCALL_WRAPPER delay_ticks, 3
SYSCALL_WRAPPER set_prio, 4
SYSCALL_WRAPPER uart_read_char, 5
SYSCALL_WRAPPER register_for_isr, 6
SYSCALL_WRAPPER write_screen_buffer, 7
SYSCALL_WRAPPER clear_screen, 8
SYSCALL_WRAPPER get_file_handle_by_path, 9
SYSCALL_WRAPPER list_files, 10
SYSCALL_WRAPPER read_from_file, 11
SYSCALL_WRAPPER get_time_in_ticks, 12