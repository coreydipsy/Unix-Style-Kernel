#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "system_call.h"
// IO port addresses for the CMOS/RTC
#define RTC_PORT_INDEX 0x70
#define RTC_PORT_DATA  0x71

// setting 0x80 will disable NMI
#define Reg_A_NMI_Disabled 0x8A
#define Reg_B_NMI_Disabled  0x8B

//used for changing the rate
#define high_mask 0xF0

//max and min rtc interrupt frequencies
#define max_frequency 32768
#define min_frequency 2

// only need regular Register C
#define Reg_C  0x0C

// RTC use IRQ8
#define RTC_IRQ_NUM 8

#define FASTEST_FREQ_FOR_QEMU 1024


// "https://wiki.osdev.org/RTC" 

/*
 * rtc_init
 *  DESC: this function initilize RTC, and enable the irq8
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: masking other interrupt for a while
*/
void rtc_init(){

    //mask interrupts
    cli();

    // telling the PIC to enable irq8
    enable_irq(RTC_IRQ_NUM);

    // select register B, and disable NMI
    outb(Reg_B_NMI_Disabled, RTC_PORT_INDEX);

    // read the current value of register B		
    char prev = inb(RTC_PORT_DATA);	  

    // set the index again (a read will reset the index to register D)              
    outb(Reg_B_NMI_Disabled, RTC_PORT_INDEX);		

    // write the previous value ORed with 0x40. This turns on bit 6 of register B
    outb(prev | 0x40, RTC_PORT_DATA);	            

    // unmask interrupts
    sti();

    // read_wait variable is used in the rtc_read() function so it waits until the next interrupt 
    //read_wait = 1; 

}



/*
 * rtc_interrupt_handler
 *  DESC: this function handles the RTC interrupt
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: masking other interrupt for a while, make the screen flicker
*/
void rtc_interrupt_handler()
{
    // mask interrupt
    cli();
    int i;
    /*set the read_wait to 0. Allows rtc_read() function to not get stuck 
    in while loop*/
   // read_wait = 0;


    for(i = 0; i < 3; i++){
        if (terminal_array[i].rtc_counter < terminal_array[i].rtc_interval) terminal_array[i].rtc_counter++;
    }

    outb(0x0C, RTC_PORT_INDEX);	  // select register C
    inb(RTC_PORT_DATA);		// just throw away contents


    // telling the PIC irq8 is done
    send_eoi(RTC_IRQ_NUM);

    // unmask interrupt
    sti();

}

/*
    IMPORTANT NOTES to understand frequency of RTC.

    frequency of interrupt rate =  32768 >> (rate-1);

    The default rate is set to 6. If we do 32768 >> (6-1), we get 1024. 1024 hz is the default frequency for interrupt rate. 

*/

/*
 *  rtc_open
 *  DESC:   We want to set the frequency of the interrupt firing to 2Hz. 
            To do this, we need to set the rate to 15. 
            32768 >> (15-1) = 2. 
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: reads from ports but we mask so its ok
*/
int32_t rtc_open(const uint8_t* filename, int32_t fd)
{
    rate = 0x06;	//set rate to 6. This is equal to 1024 hz (Max Freq)

    // mask interrupt
    cli();

    outb(Reg_A_NMI_Disabled, RTC_PORT_INDEX);		// set index to register A, disable NMI
    char prev = inb(RTC_PORT_DATA);	// get initial value of register A
    outb(Reg_A_NMI_Disabled, RTC_PORT_INDEX);		// reset index to A
    outb((prev & high_mask) | rate, RTC_PORT_DATA); //write only our rate to A. Note, rate is the bottom 4 bits.
    sti();
    //unmask interrupts
    terminal_array[current_terminal].rtc_counter = 0;



    return 0;
}


/*
 *  rtc_close
 *  DESC:   - we don't do anything for this function in checkpoint 2. 
            or checkpoint 3, we need to do this function. 
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: none
*/
int32_t rtc_close(int32_t fd)
{
    
    return 0;
}

/*
 *  rtc_read
 *  DESC:   by using this function, we can know when an rtc interrupt was fired. 
            For example, if we want only 10 rtc interrupts and stop the program, 
            we call this function 10 times. 
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: none
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
    terminal_array[current_terminal].rtc_counter = 0;
    while(terminal_array[current_terminal].rtc_counter != terminal_array[current_terminal].rtc_interval){}
    return 0;
}

/*
 *  rtc_write
 *  DESC:  -given a frequency, set the rtc interrupt frequency. 
           -we need to do this simple calculation to find the 
           -rate value
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: changes the rate global value
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
    int frequency = *((int32_t*)buf);
    // this will get the frequency 
    cli();
    terminal_array[current_terminal].rtc_interval = (FASTEST_FREQ_FOR_QEMU/frequency); 
    sti();

    //check if the frequncy passed in is between the valid numbers
    if(frequency < min_frequency || frequency > max_frequency)
    {
        return -1;
    }


    //terminal_array[current_terminal].rtc_rate = rate;
    terminal_array[current_terminal].rtc_counter = 0;


    return 0;

}

