#define ASM     1

.text

// for exceptions
.global int_0, int_1, int_2, int_3, int_4, int_5, int_6, int_7, int_8
.global int_9, int_10, int_11, int_12, int_13, int_14, int_16, int_17
.global int_18, int_19

// for interrupts
.global keyboard_helper, rtc_helper, system_call_helper, PIT_helper

// system call jump table
// we leave the first one open because there are no systemcall 1, 0 is just a dummy value
// need to add the other 4 later sys_call_getargs, sys_call_vidmap, sys_call_sethandler, sys_call_sigreturn 
syscall_jumptable:
        .long 0, halt_helper, execute_helper, read_helper, write_helper, open_helper, close_helper, getargs_helper, vidmap_helper, sethandler_helper, sigreturn_helper


// each of these are identical. they save all the registers & flags
// call the associated handler, pops flags/regs, does iret

// for exceptions
int_0:
    pushal
    pushfl
    call interrupt_0_divide_error_exception
    popfl
    popal
    iret

int_1:
    pushal
    pushfl
    call interrupt_1_debug_exception
    popfl
    popal
    iret

int_2:
    pushal
    pushfl
    call interrupt_2_NMI_interrupt
    popfl
    popal
    iret

int_3:
    pushal
    pushfl
    call interrupt_3_breakpoint_exception
    popfl
    popal
    iret

int_4:
    pushal
    pushfl
    call interrupt_4_overflow_exception
    popfl
    popal
    iret

int_5:
    pushal
    pushfl
    call interrupt_5_BOUND_range_exceeded_exception
    popfl
    popal
    iret

int_6:
    pushal
    pushfl
    call interrupt_6_invalid_opcode_exception
    popfl
    popal
    iret

int_7:
    pushal
    pushfl
    call interrupt_7_device_not_available_exception
    popfl
    popal
    iret

int_8:
    pushal
    pushfl
    call interrupt_8_double_fault_exception
    popfl
    popal
    iret

int_9:
    pushal
    pushfl
    call interrupt_9_coprocessor_segment_overrun
    popfl
    popal
    iret

int_10:
    pushal
    pushfl
    call interrupt_10_invalid_TSS_exception
    popfl
    popal
    iret

int_11:
    pushal
    pushfl
    call interrupt_11_segment_not_present
    popfl
    popal
    iret

int_12:
    pushal
    pushfl
    call interrupt_12_stack_fault_exception
    popfl
    popal
    iret

int_13:
    pushal
    pushfl
    call interrupt_13_general_protection_exception
    popfl
    popal
    iret

int_14:
    pushal
    pushfl
    call interrupt_14_page_fault_exception
    popfl
    popal
    iret

int_16:
    pushal
    pushfl
    call interrupt_16_x87_FPU_floating_point_error
    popfl
    popal
    iret

int_17:
    pushal
    pushfl
    call interrupt_17_alignment_check_exception
    popfl
    popal
    iret

int_18:
    pushal
    pushfl
    call interrupt_18_machine_check_exception
    popfl
    popal
    iret

int_19:
    pushal
    pushfl
    call interrupt_19_SIMD_floating_point_exception
    popfl
    popal
    iret

// for interrupts
keyboard_helper:
    pushal
    pushfl
    call keyboard_interrupt_handler
    popfl
    popal
    iret

rtc_helper:
    pushal
    pushfl
    call rtc_interrupt_handler
    popfl
    popal
    iret

PIT_helper:
    pushal
    pushfl
    call PIT_interrupt_handler
    popfl
    popal
    iret

// desc -- this is the helper function for system call interrupt
// input -- When a program invokes a system call, the arguments to the system call handler are passed in the following convention. 
// ● EAX - System Call number (What syscall the program wants to execute)
// ● EBX - The first argument to the system call
// ● ECX - The second argument to the system call
// ● EDX - The third argument to the system call
// output -- none
// return val -- error or no
// side affect -- run systemcall
system_call_helper:
    cmpl $1, %eax
    jl system_call_jump_back_bad
    cmpl $10, %eax
    jg system_call_jump_back_bad
    jmp *syscall_jumptable(,%eax, 4)
system_call_jump_back:
    iret

system_call_jump_back_bad:
    movl $-1, %eax
    jmp system_call_jump_back


// desc -- helper function to put arguments onto the stack for system_call halt
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t halt(uint8tstatus);
halt_helper:
    // only one argument
    // push the argument
    pushl %ebx 

    // call halt
    call sys_halt

    // pop off the stack 
    addl $4, %esp


    jmp system_call_jump_back

// desc -- helper function to put arguments onto the stack for system_call execute
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t execute(constuint8t* command);
execute_helper:
    // only one argument


    // push the argument
    pushl %ebx 

    // call halt
    call sys_execute

    // pop off the stack 
    addl $4, %esp

    
    jmp system_call_jump_back


// desc -- helper function to put arguments onto the stack for system_call read
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t read(int32t fd, void* buf, int32t nbytes);
read_helper:
    // three arguments

    // push three arguments
    pushl %edx 
    pushl %ecx
    pushl %ebx

    // call read
    call sys_read

    // pop three stuff off the stack 
    addl $12, %esp

    
    jmp system_call_jump_back

// desc -- helper function to put arguments onto the stack for system_call write
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t write(int32t fd, const void* buf,int32t nbytes);
write_helper:
    // three arguments


    // push three arguments
    pushl %edx 
    pushl %ecx
    pushl %ebx

    // call write
    call sys_write

    // pop three stuff off the stack 
    addl $12, %esp

    
    jmp system_call_jump_back

// desc -- helper function to put arguments onto the stack for system_call open
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t open(constuint8t* filename);
open_helper:
    // only one argument


    // push the argument
    pushl %ebx 

    // call open
    call sys_open

    // pop off the stack 
    addl $4, %esp

    
    jmp system_call_jump_back

// desc -- helper function to put arguments onto the stack for system_call close
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t close(int32t fd);
close_helper:
    // only one argument


    // push the argument
    pushl %ebx 

    // call close
    call sys_close

    // pop off the stack 
    addl $4, %esp

    
    jmp system_call_jump_back



// desc -- helper function to put arguments onto the stack for system_call getargs
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t getargs(uint8t* buf,int32tnbytes);
getargs_helper:
    // two arguments

    // push two arguments 
    pushl %ecx
    pushl %ebx

    // call getargs
    call sys_getargs

    // pop three stuff off the stack 
    addl $8, %esp

    jmp system_call_jump_back

// desc -- helper function to put arguments onto the stack for system_call vidmap
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t vidmap(uint8t** screenstart);
vidmap_helper:
    // only one argument
    // push the argument
    pushl %ebx 

    // call vide0_map
    call sys_vidmap

    // pop off the stack 
    addl $4, %esp

    jmp system_call_jump_back


// desc -- helper function to put arguments onto the stack for system_call set_handler
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t sethandler(int32tsignum,void* handleraddress);
sethandler_helper:
    // two arguments

    // push two arguments 
    pushl %ecx
    pushl %ebx

    // call set_handler
    call sys_set_handler

    // pop three stuff off the stack 
    addl $8, %esp

    jmp system_call_jump_back

// desc -- helper function to put arguments onto the stack for system_call sigreturn
// input --none
// output -- none
// return val -- error or no
// side affect -- run systemcall, will return back to linkage
// int32t sigreturn(void);
sigreturn_helper:
    // no arguments

    // call sigreturn
    call sys_sigreturn

    jmp system_call_jump_back
