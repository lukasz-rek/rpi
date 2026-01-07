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
				command[cmd_cursor] = '\0';
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
	int path_end = 1;
	virt_node_t current_folder;
	call_sys_get_file_handle_by_path(current_path, path_end, &current_folder);
	
	while(1) {
		char command[CMD_LEN] = "";

		term_printf("%s>", current_path);
		
		get_command(command);


		// process command
		// For now we only support up to 2 args, 0th arg is cmd
		char args[3][CMD_LEN/3] = {0};
		int arg_lens[3] = {0};
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
				args[arg_id][copy_idx] = '\0';  // Null terminate!
				arg_lens[arg_id] = copy_idx;
				arg_id++;
				input_idx++;
				copy_idx = 0;
			} else if (command[input_idx] == '\0' || command[input_idx] == '\r') {
				// End of input - null terminate current arg
				arg_lens[arg_id] = copy_idx;
				args[arg_id][copy_idx] = '\0';
				break;
			} else {
				args[arg_id][copy_idx++] = command[input_idx++]; 
			}
		}

		term_printf("\r");

		// Stupid matching but oh well
		if (!user_str_compare(args[0], "echo", 4)) {
			term_printf("%s", args[1]);
		} else if (!user_str_compare(args[0], "ls", 2)) {
			virt_node_t retrieved_paths[64];
			int count = 0;
			call_sys_list_files(&current_folder, retrieved_paths, &count);
			for (int i = 0; i < count; i++) {
				term_printf("%s ", retrieved_paths[i].name);
				if (!(i % 4)) {
					term_printf("\r");
				}
			}
		} else if (!user_str_compare(args[0], "cd", 2)) {
			// This is a stupid addition but I want it here, code is messy as is
			if (!user_str_compare(args[1], "..", 2)  && path_end > 1) {
				// Remove last element of path
				while(current_path[path_end] != '/') {
					current_path[path_end--] = '\0';
				}
				call_sys_get_file_handle_by_path(current_path, path_end, &current_folder);
			} else {
				// Check if parent directory
				virt_node_t retrieved_paths[64];
				int count = 0;
				int found = 0;
				call_sys_list_files(&current_folder, retrieved_paths, &count);
				for (int i = 0; i < count; i++) {
					retrieved_paths[i].name[63] = '\0';
					if(!user_str_compare(args[1], retrieved_paths[i].name, arg_lens[1])) {
						// We have a match
						if(path_end != 1) {
							current_path[path_end++] = '/';
						}
						int retrieved_id = 0;
						while(retrieved_paths[i].name[retrieved_id] != '\0') {
							current_path[path_end++] = retrieved_paths[i].name[retrieved_id++];
						}
						found = 1;
						current_folder.cluster_number = retrieved_paths[i].cluster_number;
						current_folder.is_folder = retrieved_paths[i].is_folder;
						current_folder.size = retrieved_paths[i].is_folder;
						// We can skip names, not used, also I should have properly handled memcpy here to just do =
					}
				}
				if (!found) {
					term_printf("Failed to find folder!\r");
				}
			}
		} else if (!user_str_compare(args[0], "cat", 3)) {
			char data[512];
			char new_path[64];
			int new_path_id = 0;
			int arg_id = 0;
			// Copy cur path
			while(new_path_id < path_end) {
				new_path[new_path_id] = current_path[new_path_id];
				new_path_id++;
			}
			new_path[new_path_id++] = '/';
			// Append passed arg
			while(arg_id < CMD_LEN/3) {
				new_path[new_path_id++] = args[1][arg_id++];
			}
			new_path[new_path_id] = '\0'; 
			call_sys_write("Trying to read path %s\n", new_path);
			virt_node_t file_handle;
			call_sys_write("Len is %d\n", new_path_id);
			call_sys_get_file_handle_by_path(new_path, new_path_id, &file_handle);
			call_sys_write("Got file handle %s\n", file_handle.name);
			call_sys_write("Cluster num %d\n", file_handle.cluster_number);
			call_sys_read_from_file(&file_handle, data, 0);
			term_printf("%s", data);

		} else if (!user_str_compare(args[0], "clear", 5)) {
			clear_screen();
		} else if (!user_str_compare(args[0], "uptime", 6)) {
			term_printf("Uptime in seconds: %d\r", call_sys_get_time_in_ticks()/1024);
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