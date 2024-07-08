#include "system_call.h"
#include "filesys.h"
#include "rtc.h"
#include "terminal.h"
#include "system_call_asm.h"
#include "paging.h"
#include "x86_desc.h"

#define VID_MEM_START_ADDR  0x88B8000

#define A_4KB_page_woooo  0x1000

#define MAX_PROCESS_IN_ONE_TERMINAL  6

/* jump tables */
static file_operations rtc_ops;
static file_operations file_ops;
static file_operations dir_ops;
static file_operations stdout_ops;
static file_operations stdin_ops;

/* static variables */
static dentry_t syscall_open_dentry;
// again, not sure of the size but longest file name is 32, 150 will work
uint8_t arg_command[150];
int cursor_x[3]; // index 0 is terminal 0
int cursor_y[3]; // 3 because 3 terminals
ternimal_t terminal_array[3];

int32_t terminal_PID[MAX_PROCESSES];
int current_pid;
int current_terminal = 0;
int32_t terminal_shown = 0;
int terminal_switch_in_progress = 0;
int is_base_shell = 0;
pcb_t* curr_pcb;
int arg_flag;
int base_shell_initialized = 0;

int LebronAI_flag = 0;

int32_t init_fd_entry(file_operations* jmptable, uint32_t inode, uint32_t flags, int32_t idx, pcb_t* pcb);

// This is a glorified macro that returns the PCB ADDR on the stack
uint32_t PCB_ADDR(uint32_t process_id) {
    return ((EIGHT_MB) - ((EIGHT_KB) * ((process_id+1))));
}

// This is a glorified macro that returns the start of the PID on the stack
uint32_t PID_STACK(uint32_t process_id) {
    return ((EIGHT_MB) - ((EIGHT_KB) * (process_id)));
} 

/*
 * init_terminal_struct
 *  DESC: initializes the 3 terminal structs
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN VAL: none
 *  SIDE EFFECTS: none
*/
void init_terminal_struct(){
    int i;
    for(i = 0; i < 3; i++){
        terminal_array[i].status = 0;
        terminal_array[i].numchar_for_this_terminal = 0;
        terminal_array[i].over_80_flag = 0;
        terminal_array[i].enter_flag = 0;
        terminal_array[i].rtc_interval = 0;
        terminal_array[i].rtc_counter = 0;
        terminal_array[i].rtc_flag = 0;
        
        memset(terminal_array[i].terminal_buffer, 0, sizeof(terminal_array[i].terminal_buffer));
    }
}

/*
 * terminal_swap
 *  DESC: switches the shown terminal
 *  INPUT: terminal - terminal number to go to
 *  OUTPUT: none
 *  RETURN VAL: none
 *  SIDE EFFECTS: changes paging
*/
void terminal_swap(int terminal) {
    /*
    a. Copy current video memory from the transparent region to the respective non-transparent
    region of the TB, and save state of current cursor position.
    b. Update internal state representing what terminal is shown.
    c. Copy the non-transparent memory of TP into the transparent region, and move the cursor
    position to the correct location.
    d. Depending on implementation think about TLB (flushes).

    // screenx, screeny is where the cursor is, need to extend it to global variable we can access here
    
    void* memcpy(void* dest, const void* src, uint32_t n)
    
    */
    // B8: screen
    // B9: Terminal 0
    // BA: Terminal 1
    // BB: Terminal 2
    cli();
    int old_terminal;
    old_terminal = terminal_shown;
    terminal_shown = terminal;
    if(terminal_shown == old_terminal){
        return;
    }
    
    // copy the current thing on the screen to the terminal's own memory
    // +1 because when it is 0 we want it to store at B9
    void* dest = memcpy((uint8_t*)(0xB8000+(old_terminal+1)*A_4KB_page_woooo), (uint8_t*)(0xBC000), A_4KB_page_woooo);

    /* we use 0xBC000 as dest because it always points to physical 0xB8000*/
    dest =  memcpy((uint8_t*)(0xBC000), (uint8_t*)(0xB8000+(terminal_shown+1)*A_4KB_page_woooo), A_4KB_page_woooo);
    sti();   
}



/*
 * sys_halt
 *  DESC: halt system call, halts an open process
 *  INPUT: status (not used)
 *  OUTPUT: int for error checking
 *  RETURN VAL: Despite having a return value, a value is never returned
 *              from this function on success and rather jumps into execute which
 *              started this process. If we are at the base process then we return
 *              an error.
 *  SIDE EFFECTS: Descrements the PID, closes fd_table, modifies paging
*/
int32_t sys_halt(uint8_t status){
    cli();
    memset(terminal_array[current_terminal].terminal_buffer, 0, sizeof(terminal_array[current_terminal].terminal_buffer));
    terminal_array[current_terminal].numchar_for_this_terminal = 0;
    terminal_array[current_terminal].over_80_flag = 0;
    terminal_array[current_terminal].enter_flag = 0;

    /* if base shell don't exit */
    if ((current_pid%MAX_PROCESS_IN_ONE_TERMINAL) == 0){
        is_base_shell = 1; 
        global_process_count--;
        sti();
        sys_execute((uint8_t*)"shell"); 
        return -1;
    }

    /* reset paging */
    set_user_page((current_pid-1));
    flush_tlb();

    /* restore parent data */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PID_STACK((current_pid-1));

    /* clear relevant FD's*/
    int i;
    for (i = 0; i < FD_TABLE_SIZE; i++) {
        if (curr_pcb->fd_table[i].status == ACTIVE) {
            sys_close(i);
        }
    }
    /* mark PID to unused */
    terminal_PID[current_pid] = -1;

    /* restore the saved ebp*/
    uint32_t ebp_that_is_saved = curr_pcb-> ebp;

    /* change PID/PCB to parent before returning */
    curr_pcb = (pcb_t*)PCB_ADDR((current_pid - 1));
    current_pid = current_pid - 1;

    /* jump to execute lable (U_R_MY_SUNSHINE) */
    __asm__ volatile (
        "movl %0, %%ebp\n\t"
        "movl %%ebp, %%esp\n\t"
        "jmp U_R_MY_SUNSHINE\n\t"
        :
        : "r"(ebp_that_is_saved)
        : "%ebp", "%esp"
    );

    /* we should never reach this spot */
    return 0;
}

/*
 * sys_execute
 *  DESC: starts a process by loading an executable into physical memory
 *        and by context switching to the user program. read the code
 *  INPUT: command - an executable to run
 *  OUTPUT: int for error checking
 *  RETURN VAL: 0 for success, -1 for error
 *  SIDE EFFECTS: occupies a new PID, changes PCB, modifies paging, writes to memory
*/
int32_t sys_execute(const uint8_t* command){
   cli();
   int i;
   int command_length;
   int argument_offset;
   // i put it as 30 but i am not sure what is the longest executable name
   // the is the name of the file, first argument
   uint8_t execute_name[150];
   if(strncmp((int8_t*)command,(int8_t*)"Lebron AI", sizeof(command) ) == 0 ){
    return 0;
   }
   if(LebronAI_flag == 1){
    return 0;
   }

   // check if current PCB is full, if not, assigned one to current PID
    if (base_shell_initialized == 0) {
        for (i = 0; i < MAX_PCB; i++) {
            if (terminal_PID[current_terminal*MAX_PROCESS_IN_ONE_TERMINAL + i] == -1) {   // we have this current_terminal*6 line for checkpoint 5
                break;                                          // it is hard set to 0 right now
            } 
        }
    } else {
        for (i = 0; i < MAX_PCB; i++) {
            if (terminal_PID[terminal_shown*MAX_PROCESS_IN_ONE_TERMINAL + i] == -1) {   // we have this current_terminal*6 line for checkpoint 5
                break;                                          // it is hard set to 0 right now
            } 
        }
    }
    if (i == MAX_PCB || global_process_count == MAX_PCB) {
        char arr[] = "MAX PROCESSES OPEN\n";
        terminal_write(2, (void*)arr, 20);  // 20 for size of the error message
        return -1;
    }

    /* parse magic bytes and name */
    uint8_t elf_magic_bytes[4];             /* bytes 0-3 (magic string) */
    uint32_t eip_magic_bytes;               /* bytes 24-27 (addr of executable)*/
    uint8_t buf[28];                        /* buffer to get bytes 0-27*/
    // --------------------------------------------------------------------------
    //parse the command 
    arg_flag = 0;
    command_length = strlen((int8_t*)command);
    // this will not include the '/0' dont know if i should + 1
    memset(execute_name,'\0',sizeof(execute_name));
    memset(arg_command,'\0',sizeof(arg_command));
    for(i = 0; i < command_length+1 ; i++){
        // +1 to get the null
        if (command[i] == ' ' && arg_flag == 0) {
            arg_flag = 1;
            // when we encounter space, null terminate the neam
            execute_name[i] = '\0';
            argument_offset = i;
        }
        
        if(arg_flag == 0){
            // if it is not an argument
            // execute name will always start from 0 so it doesnt matter
            execute_name[i] = command[i];
        }else{
            // we need to store the i - offset so it starts from 0
            arg_command[i - argument_offset] = command[i+1];
        
        }

    }

    int open = open_file_execute(execute_name);
    if (open) return -1;
    int read = read_file_execute(buf, 28);
	if (read) return -1;

    /* we need to get the 4 magic bytes associated with the EIP and ELF*/
    eip_magic_bytes = 0;
    for (i = 0; i < 4; i++) {
        elf_magic_bytes[i] = buf[i];
        eip_magic_bytes += buf[i+24] << (8*i);  // get bytes 24-27, left shift to get bytes in correct order
    }

    /* executables begin with these characters, if the file doesn't have them return error */
    if (elf_magic_bytes[0] != 0x7f || elf_magic_bytes[1] != 'E' || elf_magic_bytes[2] != 'L' || elf_magic_bytes[3] != 'F'){
        return -1;
    }

    /* find the pid for this new process, need to consider base shell */
    if (base_shell_initialized == 0) {
        if (is_base_shell == 1) {
            current_pid = current_terminal*MAX_PROCESS_IN_ONE_TERMINAL + 0;
            terminal_PID[current_terminal*MAX_PROCESS_IN_ONE_TERMINAL + 0] = current_pid;
            is_base_shell = 0;
        } else {
            for (i = 0; i < MAX_PROCESS_IN_ONE_TERMINAL; i++) {
                if(terminal_PID[current_terminal*MAX_PROCESS_IN_ONE_TERMINAL + i] == -1){
                    current_pid = current_terminal*MAX_PROCESS_IN_ONE_TERMINAL + i;
                    terminal_PID[current_terminal*MAX_PROCESS_IN_ONE_TERMINAL + i] = current_pid;
                    break;
                }
            }
        }
    } else {
        if (is_base_shell == 1) {
            current_pid = terminal_shown*MAX_PROCESS_IN_ONE_TERMINAL + 0;
            terminal_PID[terminal_shown*MAX_PROCESS_IN_ONE_TERMINAL + 0] = current_pid;
            is_base_shell = 0;
        } else {
            for (i = 0; i < MAX_PROCESS_IN_ONE_TERMINAL; i++) {
                if(terminal_PID[terminal_shown*MAX_PROCESS_IN_ONE_TERMINAL + i] == -1){
                    current_pid = terminal_shown*MAX_PROCESS_IN_ONE_TERMINAL + i;
                    terminal_PID[terminal_shown*MAX_PROCESS_IN_ONE_TERMINAL + i] = current_pid;
                    break;
                }
            }
        }
    }
        
    open = open_file_execute(execute_name);
    if (open) return -1;

    /* modify paging & flush tlb */
    set_user_page(current_pid);
    flush_tlb();

    /* load the executable into physical memory */
    read = read_file_execute((void*)(ONE_TWO_EIGHT_MB + EXC_OFFSET), (int32_t)(FOUR_MB-EXC_OFFSET));
	if (read) return -1;

    /* write the pcb onto the kernel stack */
    init_pcb(current_pid, eip_magic_bytes);

    /* save the ebp */
    __asm__("movl %%ebp, %0" : "=r" (curr_pcb->ebp));

    tss.ss0 = KERNEL_DS;
    tss.esp0 = PID_STACK(current_pid);
    global_process_count++;
    sti();
    /* asm linkage to the user code, subtract 4 for page bounds with 4-byte values */
    execute_cleanup(ONE_THREE_TWO_MB - 4, eip_magic_bytes);
    //sti();
    __asm__ volatile ("U_R_MY_SUNSHINE: nop");  /* halt jumps here */
    global_process_count--;
    return 0;

}

/*
 * sys_read
 *  DESC: calls read fn for fd table entry
 *  INPUT: fd - index into fd table
 *         buf - buffer to read to
 *         nbytes - number of bytes to read
 *  OUTPUT: int for error checking
 *  RETURN VAL: -1 for error,  return val of respective read fn (read the code)
 *  SIDE EFFECTS: none
*/
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes){
    if(fd == 1){
        return -1;
    }
    if(fd < 0){
        return -1;
    }
    pcb_t* PCB_pointer = (pcb_t*)PCB_ADDR(current_pid);

    int32_t status_ = PCB_pointer -> fd_table[fd].status;
    file_operations* op_jump_ = PCB_pointer -> fd_table[fd].op_jmp;

    if(status_ == INACTIVE || !op_jump_)
        return -1;

    return (op_jump_)->read(fd, buf, nbytes);
}

/*
 * sys_write
 *  DESC: calls write fn for fd table entry
 *  INPUT: fd - index into fd table
 *         buf - buffer to read to
 *         nbytes - number of bytes to read
 *  OUTPUT: int for error checking
 *  RETURN VAL: -1 for error,  return val of respective write fn (read the code)
 *  SIDE EFFECTS: none
*/
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes){
    // negative fd shouldn't work
    if(fd < 0){
        return -1;
    }
    if(fd == 0){
        return -1;
    }
    pcb_t* PCB_pointer = (pcb_t*)PCB_ADDR(current_pid);

    int32_t status_ = PCB_pointer -> fd_table[fd].status;
    file_operations* op_jump_ = PCB_pointer -> fd_table[fd].op_jmp;

    if(status_ == INACTIVE || !op_jump_)
        return -1;

    return (op_jump_)->write(fd, buf, nbytes);

}
/*
 * sys_open
 *  DESC: calls open fn for fd table entry
 *  INPUT: filename - name of the file to open
 *  OUTPUT: int for error checking, return val of fd open fn (read the code)
 *  RETURN VAL: -1 for error, idx in fd_table on success
 *  SIDE EFFECTS: none
*/
int32_t sys_open(const uint8_t* filename){
    int no_such_file;
    // edge case if nothing
    if(strncmp((int8_t*)filename,(int8_t*)"", sizeof(filename) ) == 0){
        return -1;
    }
    no_such_file = read_dentry_by_name(filename, &syscall_open_dentry);
    if(no_such_file == -1){
        return -1;
    }

    uint32_t file_type = syscall_open_dentry.type;
    int i, ret;
    int file_opened_flag;
    file_opened_flag = 0;
    for (i = 0; i < FD_TABLE_SIZE; i++) {
        if (curr_pcb->fd_table[i].status == INACTIVE) {
            if (file_type == 0) {   // rtc file has type 0
                ret = init_fd_entry(&rtc_ops, NULL, NULL, i, curr_pcb);
                file_opened_flag = 1;
                break;
            } else if (file_type == 1) { // dir file has type 1
                ret = init_fd_entry(&dir_ops, NULL, NULL, i, curr_pcb);
                file_opened_flag = 1;
                break;
            } else if (file_type == 2) { // regular file has type 2
                ret = init_fd_entry(&file_ops, NULL, NULL, i, curr_pcb);
                file_opened_flag = 1;
                break;
            }
        }
    }
    if(file_opened_flag == 0){
        return -1;
    }
    if (ret == -1) return -1;
    curr_pcb->fd_table[i].op_jmp->open(filename, i);
    return i;
}

/*
 * sys_close
 *  DESC: calls close fn for fd table entry
 *  INPUT: fd - index into the table
 *  OUTPUT: int for error checking
 *  RETURN VAL: -1 for error, return val of respective close fn (read the code)
 *  SIDE EFFECTS: none
*/
int32_t sys_close(int32_t fd){
    // smaller than 2 means trying to close stdin stdout
    if(fd < 2){
        return -1;
    }
    int32_t status_ = curr_pcb -> fd_table[fd].status;
    file_operations* op_jump_ = curr_pcb -> fd_table[fd].op_jmp;

    /* inactive & null check */
    if(status_ == INACTIVE || !op_jump_)
        return -1;

    curr_pcb -> fd_table[fd].status = INACTIVE;
    if (op_jump_->close == NULL) {
        return -1;  // no close function associated with this entry (stdin/stdout)
    }
    return (op_jump_)->close(fd);
}

/*
 * sys_getargs
 *  DESC: gets arguments for user programs
 *  INPUT: buf - a buffer to write to
 *         nbytes - number of bytes/characters to read
 *  OUTPUT: int for error checking
 *  RETURN VAL: -1 for error, 0 for success
 *  SIDE EFFECTS: none
*/
int32_t sys_getargs(uint8_t* buf, int32_t nbytes){
    if(buf == NULL){
        return -1;
    }
    memset(buf, '\0', nbytes);
    if(arg_flag == 0 || arg_command[0] == '\0'){
        // no arguments
        return -1;
    }
    uint8_t* yourmum = arg_command;
    // int open = open_file(arg_command);
    // if (open) return -1;
    // int read = read_file_execute(buf, nbytes);
    // if (read) return -1;
    memcpy(buf, yourmum, nbytes);
    return 0;
}

/*
 * sys_vidmap
 *  DESC: sets the user video memory mapping with paging
 *  INPUT: screen_start - a pointer we modify to set video memory
 *  OUTPUT: int for error checking
 *  RETURN VAL: -1 for error, 0 for success
 *  SIDE EFFECTS: none
*/
int32_t sys_vidmap(uint8_t** screen_start){
    // bounds checking, if not in range 8000000 - C000000 then invalid argument
    if(screen_start > (uint8_t**)0xC000000 || screen_start < (uint8_t**)0x8000000 ){
        return -1;
    }

    if(current_terminal == terminal_shown){
        *screen_start = (uint8_t*)(VID_MEM_START_ADDR);
    }else{
        // uint8_t* page_34 = 0x88B8000;
        // somehow the current terminal is always stuck at 0 and triggering this
        *screen_start = (uint8_t*)(VID_MEM_START_ADDR+(current_terminal+1)*A_4KB_page_woooo);
    }
    
    return 0;
}
int32_t sys_set_handler(int32_t signum, void* handler_address){
    //  The sethandler and sigreturn calls are related to signal handling and are discussed in the section Signals below.
    //  Even if your operating system does not support signals, you must support these system calls; in such a case, however,
    //  you may immediately return failure from these calls
    return -1;
}
int32_t sys_sigreturn(void){
    //  The sethandler and sigreturn calls are related to signal handling and are discussed in the section Signals below.
    //  Even if your operating system does not support signals, you must support these system calls; in such a case, however,
    //  you may immediately return failure from these calls
    return -1;
}
/*---------------------------------------------*/

/*
 * init_processes
 *  DESC: initializes the PIDs and jumptables to be used later
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN VAL: none
 *  SIDE EFFECTS: none
*/
void init_processes() {
    /* set all PIDs to inactive */
    int i;
    for(i = 0; i < 18; i++){    // we use 18 here for 6 processes over 3 terminals (cp5 stuff, ignore for now)
        terminal_PID[i] = -1;
    }

    global_process_count = 0;

     /* initialize jumptables */
    file_ops.open = &open_file;
    file_ops.close = &close_file;
    file_ops.read = &read_file;
    file_ops.write = &write_file;

    dir_ops.open = &open_dir;
    dir_ops.close = &close_dir;
    dir_ops.read = &read_dir;
    dir_ops.write = &write_dir;

    rtc_ops.open = &rtc_open;
    rtc_ops.close = &rtc_close;
    rtc_ops.read = &rtc_read;
    rtc_ops.write = &rtc_write;

    stdout_ops.open = NULL;
    stdout_ops.close = NULL;
    stdout_ops.read = NULL;
    stdout_ops.write = &terminal_write;

    stdin_ops.open = NULL;
    stdin_ops.close = NULL;
    stdin_ops.read = &terminal_read;
    stdin_ops.write = NULL;
}

/*
 * init_pcb
 *  DESC: initializes a PCB struct
 *  INPUT: process_id - the pid of the process
 *         eip_magic_bytes - the eip of the user prgrm
 *  OUTPUT: none
 *  RETURN VAL: none
 *  SIDE EFFECTS: modifies curr_pcb
*/
void init_pcb(uint32_t process_id, uint32_t eip_magic_bytes) {
    int i;
    pcb_t suck_my_pcb;
    init_fd_entry((file_operations*)(&stdin_ops), NULL, NULL, STDIN_IDX, &suck_my_pcb);
    init_fd_entry((file_operations*)(&stdout_ops), NULL, NULL, STDOUT_IDX, &suck_my_pcb);
    for (i = 2; i < FD_TABLE_SIZE; i++) {
        suck_my_pcb.fd_table[i].status = INACTIVE;  // clear the other entries
    }
    suck_my_pcb.parent_pid = process_id-1;          // parent pid always 1 less than current pid (for now)
    suck_my_pcb.pid = process_id;
    suck_my_pcb.eip = eip_magic_bytes;
    suck_my_pcb.ebp_for_scheduling = curr_pcb->ebp_for_scheduling;
    suck_my_pcb.esp_for_scheduling = curr_pcb->esp_for_scheduling;
    curr_pcb = (pcb_t*)PCB_ADDR(process_id);
    memcpy((void*)curr_pcb, &suck_my_pcb, sizeof(suck_my_pcb));
}

/*
 * init_pcb_start
 *  DESC: initializes a PCB struct for startup (w/o)
 *  INPUT: process_id - the pid of the process
 *  OUTPUT: none
 *  RETURN VAL: none
 *  SIDE EFFECTS: modifies curr_pcb
*/
void init_pcb_start(uint32_t process_id) {
    int i;
    pcb_t suck_my_pcb;
    init_fd_entry((file_operations*)(&stdin_ops), NULL, NULL, STDIN_IDX, &suck_my_pcb);
    init_fd_entry((file_operations*)(&stdout_ops), NULL, NULL, STDOUT_IDX, &suck_my_pcb);
    for (i = 2; i < FD_TABLE_SIZE; i++) {
        suck_my_pcb.fd_table[i].status = INACTIVE;  // clear the other entries
    }
    suck_my_pcb.parent_pid = process_id-1;          // parent pid always 1 less than current pid (for now)
    suck_my_pcb.pid = process_id;
    suck_my_pcb.ebp_for_scheduling = NULL;
    suck_my_pcb.esp_for_scheduling = NULL;
    curr_pcb = (pcb_t*)PCB_ADDR(process_id);
    memcpy((void*)curr_pcb, &suck_my_pcb, sizeof(suck_my_pcb));
}

/*
 * init_fd_entry
 *  DESC: initializes an entry in the file descriptor table
 *  INPUTS: jmptable, inode, flags - fields for the struct
 *          idx - the desired index to fill, -1 if no preference
 *  OUTPUT: an int giving either the index or an error
 *  RETURN VALUE: -1 on error, positive number for the index
 *  SIDE EFFECTS: another file is open 
*/
int32_t init_fd_entry(file_operations* jmptable, uint32_t inode, uint32_t flags, int32_t idx, pcb_t* pcb) {
    /* index not important */
    if (idx < 0) {
        int i, in_use = 0;
        /* find an open entry */
        for (i = 0; i < FD_TABLE_SIZE; i++) {
            if (pcb -> fd_table[i].status == INACTIVE) break;
            in_use++;
        }
        if (in_use == FD_TABLE_SIZE-1) return -1; /* return error if full */
        if (!jmptable) return -1;                 /* invalid ptr */
        /* init entry */
        pcb -> fd_table[i].status = ACTIVE;
        pcb -> fd_table[i].op_jmp = (file_operations*)jmptable;
        pcb -> fd_table[i].offset = 0;
        pcb -> fd_table[i].flags = flags;
        pcb -> fd_table[i].inode = inode;

        return i; /* return index on success */
    } else {
    /* index is important */
        //if (pcb -> fd_table[idx].status == ACTIVE) return -1; /* return error if occupied */
        if (!jmptable) return -1;
        pcb -> fd_table[idx].status = ACTIVE;
        pcb -> fd_table[idx].op_jmp = (file_operations*)jmptable;
        pcb -> fd_table[idx].offset = 0;
        pcb -> fd_table[idx].flags = flags;
        pcb -> fd_table[idx].inode = inode;

        return idx;
    }
}





