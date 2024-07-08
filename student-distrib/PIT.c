// #define COUNTDOWN_DONE_MSG 1
// #include "PIT.h"
#include "system_call.h"
#include "lib.h"
#include "i8259.h"
#include "x86_desc.h"
#include "paging.h"
#include "system_call_asm.h"


#define PIT_channel0    0x40 

#define PIT_command_port     0x43 

#define WE_WANT_40_HZ     40 

#define LOW_MASK     0xFF 

#define HIGH_MASK     0xFF00 

#define WE_WANT_40_HZ     40 

#define HAHA_THIS_IS_4KB 	0x1000
#define lo_hi_rate_gen     0x43 
#define BASE_FREQ 		1193182
#define PIT_uses_IRQ0	0
#define LEBRON_HEAT_NUM_ALSO_MAX_PROCESS	6


int Lebron_FMVP_count = 0;
int After_Three_Execute_Flag = 0;
int destination_PID;
int i;


/*
 * PIT_init
 *  DESC: this function initilize PIT, and enable the irq0
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: masking other interrupt for a while
*/
 void PIT_init() {
    int PIT_speed = BASE_FREQ / WE_WANT_40_HZ;
    // Disable interrupts
	/*
	Bit/s        Usage
	7            Output pin state
	6            Null count flags
	4 and 5      Access mode :
					0 0 = Latch count value command
					0 1 = Access mode: lobyte only
					1 0 = Access mode: hibyte only
					1 1 = Access mode: lobyte/hibyte
	1 to 3       Operating mode :
					0 0 0 = Mode 0 (interrupt on terminal count)
					0 0 1 = Mode 1 (hardware re-triggerable one-shot)
					0 1 0 = Mode 2 (rate generator)
					0 1 1 = Mode 3 (square wave generator)
					1 0 0 = Mode 4 (software triggered strobe)
					1 0 1 = Mode 5 (hardware triggered strobe)
					1 1 0 = Mode 2 (rate generator, same as 010b)
					1 1 1 = Mode 3 (square wave generator, same as 011b)
	0            BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
	We want square wave generator 0011 0110 or 0011 0100, 34 or 36
	*/

    outb(lo_hi_rate_gen, PIT_command_port);
	// Access mode: lobyte/hibyte
	outb(PIT_speed & LOW_MASK, PIT_channel0);		// Low byte
	// right shift by a byte so it is 8
	outb((PIT_speed & HIGH_MASK)>>8, PIT_channel0);	// High byte
	enable_irq(PIT_uses_IRQ0);

    

 }


/*
 * remapB8
 *  DESC: This function re map the virtual address B8 to the correct page
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: a lot of the functions write to virtual address B8 but that's not what we always want bruh took me a long time to figure it out
*/
void remapB8(int from_terminal, int to_terminal){
	cli();
	if(to_terminal == terminal_shown){
		// set the kernel vidmap virtual address B8 to actual B8
		init_pte_entry(first_page_table, VID_MEM_PTT_IDX, ACTIVE, SUPERVISOR, READ_WRITE, (void*)VID_MEM_PAGE_ADDR);
		// set the user vidmap virtual address B8 to actual B8
		init_pte_entry(user_vid_table, VID_MEM_PTT_IDX, ACTIVE, USER, READ_WRITE, (void*)VID_MEM_PAGE_ADDR);
	}else{
		// set the kernel virtual address B8 to B9, BA, BB accordingly
		init_pte_entry(first_page_table, VID_MEM_PTT_IDX, ACTIVE, SUPERVISOR, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR+((to_terminal+1)*HAHA_THIS_IS_4KB)));
		// set the user virtual address B8 to B9, BA, BB accordingly
		init_pte_entry(user_vid_table, VID_MEM_PTT_IDX, ACTIVE, USER, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR+((to_terminal+1)*HAHA_THIS_IS_4KB)));
	}
	flush_tlb();
	sti();
}


/*
 * PIT_interrupt_handler
 *  DESC: this function handles the PIT interrupt
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: this is super important as it handles the whole scheduling ah ah ah
*/
void PIT_interrupt_handler(){
    
	//LeScheduling();
	// -----------------------------------------------
	/*
	Steps in Scheduling:
	- increase the current terminal
	- save current informantion
	- context switch (change tss)
	- flush tlb to go to the correct user program
	*/
	int from_terminal = current_terminal;

	/**/
	if(After_Three_Execute_Flag == 0){
		if(Lebron_FMVP_count != 0){
			// the first time is the only time we dont save
			// when the second time we come in the current terminal should still be the old value
			// count = 1, save current_terminal = 0
			// count = 2, save current_terminal = 1
			// every terminal has max process of 6 so times 6
			curr_pcb = (pcb_t*)PCB_ADDR(current_terminal*LEBRON_HEAT_NUM_ALSO_MAX_PROCESS);
			__asm__("movl %%ebp, %0" : "=r" (curr_pcb->ebp_for_scheduling));
			__asm__("movl %%esp, %0" : "=r" (curr_pcb->esp_for_scheduling));
		}
		if(Lebron_FMVP_count == 2){
			// when the third time we come in, it is the last time
			After_Three_Execute_Flag = 1;
		}
		// update current terminal
		// first time 0
		// second time 1
		// third time 2
		current_terminal = Lebron_FMVP_count;
		terminal_array[current_terminal].status = 1;
		curr_pcb = (pcb_t*)PCB_ADDR(current_terminal*LEBRON_HEAT_NUM_ALSO_MAX_PROCESS);
		// increase count so next time the current terminal will be updated
		Lebron_FMVP_count++;
		send_eoi(PIT_uses_IRQ0);
		remapB8(from_terminal, current_terminal);
		sys_execute((uint8_t*)"shell");
		
	}else{
		// first time we get here will save terminal 2
		//current_terminal = (current_terminal + 1) % 3;
		curr_pcb = (pcb_t*)PCB_ADDR(current_pid);
		__asm__("movl %%ebp, %0" : "=r" (curr_pcb->ebp_for_scheduling));
		__asm__("movl %%esp, %0" : "=r" (curr_pcb->esp_for_scheduling));

	}

	// first time gets here, current terminal = 2, get updated to 0
	// round robin +1 mod 3 will kepp looping
	current_terminal = (current_terminal + 1) % 3;



	// ----------------------------------------------------------------------------select new PID
	// get the PID we are jumping to, the newest process at that terminal
	for (i = LEBRON_HEAT_NUM_ALSO_MAX_PROCESS*current_terminal+5; i >= LEBRON_HEAT_NUM_ALSO_MAX_PROCESS*current_terminal; i--) {
		// -1 means not used
        if (terminal_PID[i] != -1) { 
            destination_PID = i;
			current_pid = i;
			// this is the pcb of where we are jumping to
            curr_pcb = (pcb_t*)PCB_ADDR(destination_PID);
            break;
        }
    }

	/* modify paging & flush tlb */
    set_user_page(destination_PID);
	remapB8(from_terminal,current_terminal);
	flush_tlb();

	tss.ss0 = KERNEL_DS;
    tss.esp0 = PID_STACK(destination_PID);// switch to that PID

	__asm__("movl %0, %%esp" :: "r" (curr_pcb->esp_for_scheduling));
	__asm__("movl %0, %%ebp" :: "r" (curr_pcb->ebp_for_scheduling));
	//------------------------------------------------
    
	// we have verified that terminal is in fact switching
	send_eoi(PIT_uses_IRQ0);
	return;

    
}








