#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ncurses.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>

#include "dtop.h"

int main(int argc, char** argv) {	
	// initialize new screen
	initscr();
	// disable buffering
	raw();
	// do not show input
	noecho();
	// no cursor
	curs_set(FALSE);
	// initialize and set background color to dark green
	start_color();
	init_color(0, 155, 255, 244);
	// enable scrolling
	scrollok(stdscr, true);
	
	show_main_menu();
	
	endwin();
	
	return 0;
}

void show_main_menu() {
	int option = -1, highlight = 1;
	char c;
	
	char *choices[] = {
		"1. List Processes with open file descriptors",
		"2. Show system usage",
		"0. Exit"
	};
	
	mvprintw(0, 0, "Select an option:");
	int num_choices = sizeof(choices)/sizeof(char*);
	
	// display main menu and use up and down arrows to go trough the menu items
	while(option == -1) {
		print_menu(choices, highlight, num_choices);
		c = getch();
		switch(c) {
			case 65:
				if(highlight == 1)
					highlight = num_choices;
				else
					--highlight;
				break;
			case 66:
				if(highlight == num_choices)
					highlight = 1;
				else 
					++highlight;
				break;
			case 10:
				option = highlight;
				break;
			default:
				refresh();
				break;
		}
		print_menu(choices, highlight, num_choices);
	}
	
	switch(option) {
		case 1:
			list_processes();
			break;
		case 2:
			show_system_usage();
			break;
	}
}

void print_menu(char** choices, int highlight, int num_choices) {
	int x, y;
	for(int i = 0; i < num_choices; i++) {
		x = 2;
		y = 2;
		if(highlight == i + 1) {
			// reverse colors of the selected choice
			attron(A_REVERSE);
			mvprintw(i+2, x, choices[i]);
			attroff(A_REVERSE);
		} else {
			mvprintw(i+2, x, choices[i]);
		}
		++y;
		// refresh screen
		refresh();
	}
}

/* Display list of processes from /proc/ file */
void list_processes() {
	// clear standard screen
	clear();
	mvprintw(1, 0, "List of processes");
	mvprintw(3, 0, "PID \tName");
	mvprintw(4, 0, "--------------------------------------------");
	// dirent structure holds inode number, type, and name of the directory
	struct dirent *entry;
	// structure to hold PID and name
	struct proc {
		int id;
		char name[30];
	};
	// directory object
	DIR *dir = opendir("/proc");
	char p_name[20];
	int x=5, y=0;
	
	// read directories
	while((entry = readdir(dir)) != NULL) {
		int pid;
		// get directory name as process ID
		sscanf(entry->d_name, "%d", &pid);
		// if it is directory and PID 
		if(entry->d_type == DT_DIR && pid > 0 && isdigit(*entry->d_name)) {
			FILE *pipe_procstatus;
			char command[50];
			sprintf(command, "awk -F ':' '{print $2}' /proc/%d/status  | head -1", pid);
			pipe_procstatus = popen(command, "r");
			
			if(pipe_procstatus) {
				fgets(p_name, sizeof(p_name), pipe_procstatus);
				mvprintw(x++,y, "%d %s ", pid, p_name);
			}
			pclose(pipe_procstatus);
		}
	}
	
	// display everything we write on display
	noraw();
	echo();
	mvprintw(LINES-1, 0, "Enter CPU ID to open file descriptors: ");
	refresh();
	
	int pid_int = 1;
	mvscanw(LINES-1, 40, "%d", &pid_int);
	refresh();
	list_process_openfiles(pid_int);
}

void list_process_openfiles(int pid) {
	clear();	
	char fd_path[30];
	char fd_path_new[30];
	char fd_file_path[50];
	char fd_file_link_path[50];
	
	mvprintw(0, 0, "Open files of PID %d", pid);
	mvprintw(1, 0, "-----------------------------");
	
	// create new path from the PID
	sprintf(fd_path, "/proc/%d/fd/", pid);
	sprintf(fd_path_new, "/proc/%d/fd/", pid);
	
	DIR *dir = opendir(fd_path);
	if(!dir) {
		mvprintw(5,5,"PID does not exists.");
		getch();
	}
	
	int x=2,y=0;
	struct dirent *entry;
	while((entry = readdir(dir)) != NULL) {
		// display only link files, since every file in fd is symbolic link to somewhere
		if(entry->d_type == DT_LNK) {
			strcpy(fd_file_path, fd_path_new);
			strcat(fd_file_path, entry->d_name);
			readlink(fd_file_path, fd_file_link_path, BUFFER_SIZE);
			mvprintw(x++, y, "%s", fd_file_link_path);
		}
	}
	
	getch();
}

/* Thread for generating and refreshing data */
void *thread_refresh_data() {
	struct mem_info memory_info;
	struct cpu_info cpu_info;
	
	while(true) {
		// read memory and cpu stats
		memory_info = read_stat_memory();
		cpu_info = read_stat_cpu();
		
		/* SHOW MEMORY INFO */
		mvprintw(2,0,"Memory Total %lu", memory_info.mem_total);
		mvprintw(3,0,"Memory Available %lu", memory_info.mem_available);
		mvprintw(4,0,"Memory Free %lu", memory_info.mem_free);
		mvprintw(5,0,"Memory Used %lu", memory_info.mem_total - memory_info.mem_free);
	
		/* SHOW CPU INFO */
		mvprintw(7,0,"CPU User %lu", cpu_info.cpu_user);
		mvprintw(8,0,"CPU Nice %lu", cpu_info.cpu_nice);
		mvprintw(9,0,"CPU Idle %lu", cpu_info.cpu_idle);
		mvprintw(9,0,"CPU System %lu", cpu_info.cpu_sys);
		mvprintw(10,0,"CPU Processes %lu", cpu_info.cpu_processes);
		mvprintw(11,0,"CPU Processes Running %lu", cpu_info.cpu_proc_running);
		
		/* Refresh screen */
		refresh();
		/* wait 2s */
		sleep(UPDATE_RATE);
	}
}

void show_system_usage() {
	clear();
	
	mvprintw(0, 0, "Data is updated every %d seconds.", UPDATE_RATE);
	mvprintw(1, 0, "-------------------------------------");
	
	// create and start new thread
	pthread_t thread;
	int res_code = pthread_create(&thread, NULL, thread_refresh_data, 0);
	
	if(res_code == -1) {
		perror("error");
	}
	
	// wait for the thread to finish before exiting
	pthread_join(thread, NULL);

	getch();
}

struct mem_info read_stat_memory() {
	FILE *fd;
	struct mem_info mem_info;
	
	// strings for data as are in the /proc/meminfo file
	char *mem_prop_strings[] = {
		"MemTotal",
		"MemFree",
		"MemAvailable"
	};
	char *command = "grep '^%s' %s | awk '{print $2}'";
	char temp_command[BUFFER_SIZE];
	
	const int n_mem_prop = sizeof(mem_prop_strings) / sizeof(char*);
	char buffer[BUFFER_SIZE];
	// list of strings for storing data from the file
	char data_array[n_mem_prop][BUFFER_SIZE];
	
	for(int i=0; i<n_mem_prop; i++) {
		// create new command to send
		sprintf(temp_command, command, mem_prop_strings[i], MEMINFO);
		// open new pipe for reading
		fd = popen(temp_command, "r");
		if(fd) {
			// get data from the pipe into buffer
			fgets(buffer, sizeof(buffer), fd);
			// push data to the data_array
			sprintf(data_array[i], buffer, sizeof(buffer));
		}
	}
	fclose(fd);
	
	char *ptr;
	// parse string to unsigned long, and store it in the mem_info structure
	mem_info.mem_total = strtoul(data_array[0], &ptr, 10);
	mem_info.mem_free = strtoul(data_array[1], &ptr, 10);
	mem_info.mem_available = strtoul(data_array[2], &ptr, 10);
	
	return mem_info;
}

struct cpu_info read_stat_cpu() {
	FILE *fd;
	struct cpu_info cpu_info;
	
	char *cpu_prop_strings[] = {
		"processes",
		"procs_running"
	};
	
	int cpu_cpu_prop = 7;
	char *command = "awk '/%s /{print $%c}' /proc/stat";
	char temp_command[BUFFER_SIZE];
	
	const int n_cpu_prop = sizeof(cpu_prop_strings) / sizeof(char*);
	char buffer[BUFFER_SIZE];
	char data_array[n_cpu_prop + cpu_cpu_prop][BUFFER_SIZE];
	
	// get data for processes and procs_runnning
	for(int i=0; i<n_cpu_prop; i++) {
		// create new command
		sprintf(temp_command, command, cpu_prop_strings[i], '2');
		// send command to the pipe and open for reading
		fd = popen(temp_command, "r");
		if(fd) {
			// get and push data from pipe to data_array
			fgets(buffer, sizeof(buffer), fd);
			sprintf(data_array[i], buffer, sizeof(buffer));
		}
	}
	
	/* Continue counting from the second index, for the first line
	 * from the stat file */
	for(int i=0+n_cpu_prop; i<cpu_cpu_prop+n_cpu_prop; i++) {
		/* Convert integer to char - i + '0' */
		sprintf(temp_command, command, "cpu", (char)i + '0');
		/* Using pipe, since we want to control over the process IO */
		fd = popen(temp_command, "r");
		if(fd) {
			fgets(buffer, sizeof(buffer), fd);
			sprintf(data_array[i], buffer, sizeof(buffer));
		}
	}
	fclose(fd);
	
	char *ptr;
	// parse string to unsigned long, and store it in the mem_info structure
	cpu_info.cpu_processes = strtoul(data_array[0], &ptr, 10);
	cpu_info.cpu_proc_running = strtoul(data_array[1], &ptr, 10);
	cpu_info.cpu_user = strtoul(data_array[2], &ptr, 10);
	cpu_info.cpu_nice = strtoul(data_array[3], &ptr, 10);
	cpu_info.cpu_sys = strtoul(data_array[4], &ptr, 10);
	cpu_info.cpu_idle = strtoul(data_array[5], &ptr, 10);
	
	return cpu_info;
}
