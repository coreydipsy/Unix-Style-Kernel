#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H

#include "types.h"
#include "lib.h"
#include "filesys.h"

#define FD_TABLE_SIZE       8
#define MAX_PROCESSES       18
#define MAX_PCB             6
#define EIGHT_MB            0x800000
#define FOUR_MB             0x400000
#define EXC_OFFSET          0x48000
#define ONE_TWO_EIGHT_MB    0x08000000
#define EIGHT_KB            8192
#define ONE_THREE_TWO_MB    ONE_TWO_EIGHT_MB + FOUR_MB
#define STDIN_IDX           0
#define STDOUT_IDX          1  
#define ACTIVE              1
#define INACTIVE            0 


int32_t terminal_0_active;
int32_t terminal_1_active;
int32_t terminal_2_active;
int32_t global_process_count;


typedef struct file_operations {
    int32_t (*open)(const uint8_t* filename, int32_t fd);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} __attribute__ ((packed)) file_operations;

typedef struct fd_t {
    uint32_t status;
    file_operations* op_jmp;  /* used in jumptable later? */
    uint32_t inode;
    uint32_t offset;
    uint32_t flags;
    dentry_t fd_dentry;
} __attribute__ ((packed)) fd_t;

typedef struct pcb_t {
    uint32_t pid;
    uint32_t parent_pid;
    fd_t fd_table[FD_TABLE_SIZE];
    uint32_t ebp;
    uint32_t eip;
    uint32_t esp0;
    uint32_t ebp_for_scheduling;
    uint32_t esp_for_scheduling;
} __attribute__((packed)) pcb_t;


typedef struct ternimal_t {
    uint32_t rtc_interval;
    uint32_t rtc_counter;
    uint32_t rtc_flag;
    uint32_t status;
    uint32_t cursor_xpos;
    uint32_t cursor_ypos;
    int numchar_for_this_terminal;
    char terminal_buffer[128];
    int enter_flag;
    int over_80_flag;

} __attribute__((packed)) ternimal_t;

extern void init_processes();
extern void init_pcb(uint32_t process_id, uint32_t eip_magic_bytes);
extern void init_pcb_start(uint32_t process_id);
extern void terminal_swap(int terminal);
extern uint32_t PID_STACK(uint32_t process_id);
extern uint32_t PCB_ADDR(uint32_t process_id);

extern int32_t sys_halt(uint8_t status);
extern int32_t sys_execute(const uint8_t* command);
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t sys_open(const uint8_t* filename);
extern int32_t sys_close(int32_t fd);
extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes);
extern int32_t sys_vidmap(uint8_t** screen_start);
extern int32_t sys_set_handler(int32_t signum, void* handler_address);
extern int32_t sys_sigreturn(void);
extern void init_terminal_struct();


extern int32_t terminal_PID[18];    // 18 b/c 3 terminals w/ 6 processes cp5 stuff, ignore
extern int current_pid;
extern int current_terminal;
extern int terminal_switch_in_progress;
extern pcb_t* curr_pcb;
extern int32_t terminal_shown;
extern int base_shell_initialized;
extern ternimal_t terminal_array[3];
extern int LebronAI_flag;

#endif

