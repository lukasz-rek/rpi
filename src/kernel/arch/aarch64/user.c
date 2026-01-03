#include "user_sys.h"
#include "printf.h"
#include "term.h"
#include "fs/fat32_files.h"

#define CMD_LEN 72



void loop()
{
	call_sys_set_prio(100);

	while (1){
		refresh_screen();	
		call_sys_delay_ticks(128);
	}
}

void get_command(char* command) {
	int cmd_cursor = 0;
	char read[CMD_LEN];

	while(1) {
		// Get some chars and process them
		int len = call_sys_uart_read_char(read);
		for(int i = 0; i < len; i++) {
			switch (read[i])
			{
			case 127:
				term_printf("%c", 127);
				command[cmd_cursor--] = '\n';
				break;

			case '\r':
				// We have the whole command
				return;
			default:
				command[cmd_cursor++] = read[i];
				term_printf("%c",read[i]);
				break;
			}
		}
	}
}


void terminal() {
	char current_path[64] = "/";
	
	while(1) {
		char command[CMD_LEN] = "";

		term_printf("%s>", current_path);
		
		get_command(command);


		// process command
		// For now we only support up to 2 args, 0th arg is cmd
		char args[3][CMD_LEN/3];
		int input_idx = 0;
		int copy_idx = 0;
		int arg_id = 0;
		uint8_t parenthesis = 0;
		while(input_idx < CMD_LEN && arg_id < 3) {
			if (command[input_idx] == '"' && parenthesis) {
				input_idx++;
				parenthesis = 0;
			} else if (command[input_idx] == '"' && !parenthesis) {
				parenthesis = 1;
				input_idx++;
			} else if(command[input_idx] == ' ' && !parenthesis) {
				args[arg_id][copy_idx] = '\0';
				arg_id++;
				input_idx++;
				copy_idx = 0;
			} else {
				args[arg_id][copy_idx++] = command[input_idx++]; 
			}
		}

		term_printf("\r");

		// Stupid matching but oh well
		if (!user_str_compare(args[0], "echo", 4)) {
			term_printf("%s", args[1]);
		} else if (!user_str_compare(args[0], "ls", 2)) {

		} else if (!user_str_compare(args[0], "cd", 2)) {
			// Check if parent directory

		} else if (!user_str_compare(args[0], "cat", 3)) {

		} else {
			term_printf("Failed to parse command!");
			call_sys_write("Failed to recognize command!\n");
		}

		term_printf("\r");

	}
}




void user_process() 
{
	call_sys_write("First user process\n\r");
	term_init(&call_sys_clear_screen, &call_sys_write_screen_buffer);
	int pid = call_sys_fork();
	if (pid < 0) {
		call_sys_write("Error during fork\n\r");
		call_sys_exit();
		return;
	}
	if (pid == 0)
		terminal();
	loop();
	
}