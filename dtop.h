#ifndef DTOP_H_
#define DTOP_H_

/* Files containing statistics */
#define STAT			"/proc/stat"
#define NET_DEV			"/proc/net/dev"
#define NET_SOCKSTAT	"/proc/net/sockstat"
#define VMSTAT			"/proc/vmstat"
#define CPU_INFO		"/proc/cpuinfo"
#define UPTIME			"/proc/uptime"
#define DISKSTATS		"/proc/diskstats"
#define MEMINFO			"/proc/meminfo"

/* Update rate for pulling data about memory and cpu from /proc/ */
#define UPDATE_RATE 2
#define BUFFER_SIZE 50

/* Structure to hold data about CPU */
struct cpu_info {
	unsigned long long cpu_user;
	unsigned long long cpu_nice;
	unsigned long long cpu_sys;
	unsigned long long cpu_idle;
	unsigned long long cpu_processes;
	unsigned long long cpu_proc_running;
};

/* Structure to hold data about RAM Memory */
struct mem_info {
	unsigned long long mem_total;
	unsigned long long mem_free;
	unsigned long long mem_available;
};

/* Prototypes */
void list_process_openfiles(int pid);
void list_processes();
void show_system_usage();
void print_menu(char** choices, int highlight, int num_choices);
void show_main_menu();
struct mem_info read_stat_memory();
struct cpu_info read_stat_cpu();

#endif
