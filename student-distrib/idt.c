#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "idt_helper.h"
#include "terminal.h"

#define NUM_EXCEPTIONS      0x14
#define START_INTERRUPTS    0x20    // PIC mapped to this port
#define NUM_INTERRUPTS      0x10

static void set_trap_gate(int vec);
static void set_intr_gate(int vec);

/*
 * initialize_IDT
 *  DESC: initializes the idt. the first 20 entries are for
 *        exceptions. entries 32-48 are for interrupts
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN VAL: none
 *  SIDE EFFECTS: none
*/
void initialize_IDT() {
    /*
     * basically we are initializing the gate in two parts. this loop
     * initializes everything except for the offset. the
     * SET_IDT_ENTRY fills in the offset. linux just does it
     * in one function
    */
    int i = 0;

    /* init exception gates */
    for (i = 0; i < NUM_EXCEPTIONS; i++) { set_trap_gate(i); }
    /* init interrupt gates */
    for (i = START_INTERRUPTS; i < START_INTERRUPTS + NUM_INTERRUPTS; i++) { set_intr_gate(i); }
    set_trap_gate(0x80);
    idt[0x80].dpl = 3;

    /* int_(i) is the assembly linkage for each handler (found in idt_helper.S) 
       SET_IDT_ENTRY initializes the offset fields for the struct */
    SET_IDT_ENTRY(idt[0], int_0);
    SET_IDT_ENTRY(idt[1], int_1);
    SET_IDT_ENTRY(idt[2], int_2);
    SET_IDT_ENTRY(idt[3], int_3);
    SET_IDT_ENTRY(idt[4], int_4);
    SET_IDT_ENTRY(idt[5], int_5);
    SET_IDT_ENTRY(idt[6], int_6);
    SET_IDT_ENTRY(idt[7], int_7);
    SET_IDT_ENTRY(idt[8], int_8);
    SET_IDT_ENTRY(idt[9], int_9);
    SET_IDT_ENTRY(idt[10], int_10);
    SET_IDT_ENTRY(idt[11], int_11);
    SET_IDT_ENTRY(idt[12], int_12);
    SET_IDT_ENTRY(idt[13], int_13);
    SET_IDT_ENTRY(idt[14], int_14);

    // there is no intel interrupt 15, can be used for customized interrupt
    // SET_IDT_ENTRY(idt[15], interrupt_0_divide_error_exception);

    SET_IDT_ENTRY(idt[16], int_16);
    SET_IDT_ENTRY(idt[17], int_17);
    SET_IDT_ENTRY(idt[18], int_18);
    SET_IDT_ENTRY(idt[19], int_19);

    /* IRQ1 on the pit maps to port 0x20, used for scheduling */
    SET_IDT_ENTRY(idt[0x20], PIT_helper);

    /* IRQ1 on the pic maps to port 0x21, used for keyboard */
    SET_IDT_ENTRY(idt[0x21], keyboard_helper);

    /* IRQ8 on the pic maps to port 0x29, used for rtc */
    SET_IDT_ENTRY(idt[0x28], rtc_helper);

    /* set an entry for system calls at 0x80, will be used later */
    SET_IDT_ENTRY(idt[0x80], system_call_helper);
    
    /* call lidt to load the IDTR */
    lidt(idt_desc_ptr);
}

/* List of handlers, all they do now is print something and halt with a while loop */

// Interrupt 0—Divide Error Exception (#DE)
void interrupt_0_divide_error_exception(){
    //print_bron_sad();
    //printf("Divide Error Exception (#DE)\n");
    char arr[] = "Divide Error Exception (#DE)\n";
    terminal_write(2, (void*)arr, 30);  // 30 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened

}

//  Interrupt 1—Debug Exception (#DB)
void interrupt_1_debug_exception(){
    //print_bron_sad();
    //printf("Debug Exception (#DB)");
    char arr[] = "Debug Exception (#DB)\n";
    terminal_write(2, (void*)arr, 23);  // 23 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
    
}

//  Interrupt 2—NMI Interrupt
void interrupt_2_NMI_interrupt(){
    //print_bron_sad();
    //printf("NMI Interrupt");
    char arr[] = "NMI Interrupt\n";
    terminal_write(2, (void*)arr, 15);  // 15 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 3—Breakpoint Exception (#BP)
void interrupt_3_breakpoint_exception(){
    //print_bron_sad();
    //printf("Breakpoint Exception (#BP)");
    char arr[] = "Breakpoint Exception (#BP)\n";
    terminal_write(2, (void*)arr, 28);  // 28 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 4—Overflow Exception (#OF)
void interrupt_4_overflow_exception(){
    //print_bron_sad();
    //printf("Overflow Exception (#OF)");
    char arr[] = "Overflow Exception (#OF)\n";
    terminal_write(2, (void*)arr, 26);  // 26 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 5—BOUND Range Exceeded Exception (#BR)
void interrupt_5_BOUND_range_exceeded_exception(){
    //print_bron_sad();
    //printf("BOUND Range Exceeded Exception (#BR)");
    char arr[] = "Bound Range Exceeded Exception (#BR)\n";
    terminal_write(2, (void*)arr, 38);  // 38 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 6—Invalid Opcode Exception (#UD)
void interrupt_6_invalid_opcode_exception(){
    //print_bron_sad();
    //printf("Invalid Opcode Exception (#UD)");
    char arr[] = "Invalid Opcode Exception (#UD)\n";
    terminal_write(2, (void*)arr, 32);  // 32 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 7—Device Not Available Exception (#NM)
void interrupt_7_device_not_available_exception(){
    //print_bron_sad();
    //printf("Device Not Available Exception (#NM)");
    char arr[] = "Device Not Available Exception (#NM)\n";
    terminal_write(2, (void*)arr, 38);  // 38 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 8—Double Fault Exception (#DF)
void interrupt_8_double_fault_exception(){
    //print_bron_sad();
    //printf("Double Fault Exception (#DF)");
    char arr[] = "Double Fault Exception (#DF)\n";
    terminal_write(2, (void*)arr, 30);  // 30 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 9—Coprocessor Segment Overrun
void interrupt_9_coprocessor_segment_overrun(){
    //print_bron_sad();
    //printf("Coprocessor Segment Overrun");
    char arr[] = "Coprocessor Segment Overrun\n";
    terminal_write(2, (void*)arr, 30);  // 30 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 10—Invalid TSS Exception (#TS)
void interrupt_10_invalid_TSS_exception(){
    //print_bron_sad();
    //printf("Invalid TSS Exception (#TS)");
    char arr[] = "Invalid TSS Exception (#TS)\n";
    terminal_write(2, (void*)arr, 29);  // 29 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 11—Segment Not Present (#NP)
void interrupt_11_segment_not_present(){
    //print_bron_sad();
    //printf("Segment Not Present (#NP)");
    char arr[] = "Segment Not Present (#NP)\n";
    terminal_write(2, (void*)arr, 27);  // 27 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 12—Stack Fault Exception (#SS)
void interrupt_12_stack_fault_exception(){
    //print_bron_sad();
    //printf("Stack Fault Exception (#SS)");
    char arr[] = "Stack Fault Exception (#SS)\n";
    terminal_write(2, (void*)arr, 29);  // 29 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 13—General Protection Exception (#GP)
void interrupt_13_general_protection_exception(){
    //print_bron_sad();
    //printf("General Protection Exception (#GP)");
    char arr[] = "General Protection Exception (#GP)\n";
    terminal_write(2, (void*)arr, 36);  // 36 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//   Interrupt 14—Page-Fault Exception (#PF)
void interrupt_14_page_fault_exception(){
    //print_bron_sad();
    //printf("Page-Fault Exception (#PF) Occured!\n");
    char arr[] = "Page-Fault Exception (#PF)\n";
    terminal_write(2, (void*)arr, 28);  // 28 for size of the error message
    sys_halt(-1); //bad things happened
    //sys_execute((uint8_t*)"shell");
    //while(1){};
}

// no interrupt 15

//  Interrupt 16—x87 FPU Floating-Point Error (#MF)
void interrupt_16_x87_FPU_floating_point_error(){
    //print_bron_sad();
    //printf("x87 FPU Floating-Point Error (#MF)");
    char arr[] = "x87 FPU Floating-Point Error (#MF)\n";
    terminal_write(2, (void*)arr, 36);  // 36 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 17—Alignment Check Exception (#AC)
void interrupt_17_alignment_check_exception(){
    //print_bron_sad();
    //printf("Alignment Check Exception (#AC)");
    char arr[] = "Alignment Check Exception (#AC)\n";
    terminal_write(2, (void*)arr, 33);  // 33 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 18—Machine-Check Exception (#MC)
void interrupt_18_machine_check_exception(){
    //print_bron_sad();
    //printf("Machine-Check Exception (#MC)");
    char arr[] = "Machine-Check Exception (#MC)\n";
    terminal_write(2, (void*)arr, 31);  // 30 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}

//  Interrupt 19—SIMD Floating-Point Exception (#XF)
void interrupt_19_SIMD_floating_point_exception(){
    //print_bron_sad();
    //printf("SIMD Floating-Point Exception (#XF)");
    char arr[] = "SIMD Floating-Point Exception (#XF)\n";
    terminal_write(2, (void*)arr, 37);  // 30 for size of the error message
    //while(1){};
    sys_halt(-1); //bad things happened
}


// Interrupts 32 to 255—User Defined Interrupts



/*
 * set_intr_gate
 *  DESC: initializes a gate in the IDT
 *  INPUT: vec - entry in the IDT
 *  OUTPUT: none
 *  SIDE EFFECTS: none
 *  RETURN VALUE: none
 * 
 *  check x86_desc.h for the idt_desc_t struct, explains most of this
*/
static void set_intr_gate(int vec) {
    idt[vec].seg_selector = KERNEL_CS;  // set to kernel code segment
    idt[vec].reserved4 = 0;             // 8-bits reserved for ???
    idt[vec].reserved3 = 0;             // 1110 -> 32-bit interrupt gate
    idt[vec].reserved2 = 1;
    idt[vec].reserved1 = 1;
    idt[vec].size = 1;                  // set to 32-bit
    idt[vec].reserved0 = 0;             // just set this to 0
    idt[vec].dpl = 0;                   // 0 = highest priority
    idt[vec].present = 1;               // basically says this is a legit entry
}

/*
 * set_trap_gate
 *  DESC: initializes a gate in the IDT
 *  INPUT: vec - entry in the IDT
 *  OUTPUT: none
 *  SIDE EFFECTS: none
 *  RETURN VALUE: none
 * 
 *  check x86_desc.h for the idt_desc_t struct, explains most of this
*/
static void set_trap_gate(int vec) {
    idt[vec].seg_selector = KERNEL_CS;  // set to kernel code segment
    idt[vec].reserved4 = 0;             // 8-bits reserved for ???
    idt[vec].reserved3 = 1;             // 1111 -> 32-bit trap gate
    idt[vec].reserved2 = 1;
    idt[vec].reserved1 = 1;
    idt[vec].size = 1;                  // set to 32-bit
    idt[vec].reserved0 = 0;             // just set this to a 0
    idt[vec].dpl = 0;                   // 0 = highest priority
    idt[vec].present = 1;               // basically says this is a legit entry
}  
