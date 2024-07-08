#include "x86_desc.h"
#include "lebron.h"

// list out all the interrupts and exceptions
// pass in the function name directly without the parenthesis is a function pointer that points to the function
// so i can just define it here and pass in directly into the SET_IDT_ENTRY(str, handler)

extern void initialize_IDT();

// list of interrupts

// Interrupt 0—Divide Error Exception (#DE)
extern void interrupt_0_divide_error_exception();

//  Interrupt 1—Debug Exception (#DB)
extern void interrupt_1_debug_exception();

//  Interrupt 2—NMI Interrupt
extern void interrupt_2_NMI_interrupt();

//  Interrupt 3—Breakpoint Exception (#BP)
extern void interrupt_3_breakpoint_exception();

//  Interrupt 4—Overflow Exception (#OF)
extern void interrupt_4_overflow_exception();

//  Interrupt 5—BOUND Range Exceeded Exception (#BR)
extern void interrupt_5_BOUND_range_exceeded_exception();

//  Interrupt 6—Invalid Opcode Exception (#UD)
extern void interrupt_6_invalid_opcode_exception();

//  Interrupt 7—Device Not Available Exception (#NM)
extern void interrupt_7_device_not_available_exception();

//  Interrupt 8—Double Fault Exception (#DF)
extern void interrupt_8_double_fault_exception();

//  Interrupt 9—Coprocessor Segment Overrun
extern void interrupt_9_coprocessor_segment_overrun();

//  Interrupt 10—Invalid TSS Exception (#TS)
extern void interrupt_10_invalid_TSS_exception();

//  Interrupt 11—Segment Not Present (#NP)
extern void interrupt_11_segment_not_present();

//  Interrupt 12—Stack Fault Exception (#SS)
extern void interrupt_12_stack_fault_exception();

//  Interrupt 13—General Protection Exception (#GP)
extern void interrupt_13_general_protection_exception();

//   Interrupt 14—Page-Fault Exception (#PF)
extern void interrupt_14_page_fault_exception();

/*
interrupt 15 does not have a specific, 
universally defined exception associated with it 
in the standard documentation for Intel's x86 processors. 


this means that operating systems and other low-level software are free to 
use interrupt 15 for their own purposes if they choose, 
as long as they account for any potential compatibility issues 
or specific behaviors on certain processors or under certain conditions.

*/

//  Interrupt 16—x87 FPU Floating-Point Error (#MF)
extern void interrupt_16_x87_FPU_floating_point_error();

//  Interrupt 17—Alignment Check Exception (#AC)
extern void interrupt_17_alignment_check_exception();

//  Interrupt 18—Machine-Check Exception (#MC)
extern void interrupt_18_machine_check_exception();

//  Interrupt 19—SIMD Floating-Point Exception (#XF)
extern void interrupt_19_SIMD_floating_point_exception();

// Interrupts 32 to 255—User Defined Interrupts
// dont care for now
void entry_0x80_system_calls();






