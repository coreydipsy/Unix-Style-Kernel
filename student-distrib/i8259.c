/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

uint8_t save_master_data;
uint8_t save_slave_data;

/* Define magic numbers for Master and Slave ports*/
#define MASTER_COMMAND       MASTER_8259_PORT         
#define MASTER_DATA          MASTER_8259_PORT + 1
#define SLAVE_COMMAND        SLAVE_8259_PORT    
#define SLAVE_DATA           SLAVE_8259_PORT + 1 

//use MASK_ALL to mask master data and slave data
#define MASK_ALL             0xFF

//create a magic number for the number 8 because it represents the start
//IRQ8 number for slave pic
#define EIGHT               8
//IRQ2 number for master pic
#define TWO               2


/* Initialize the 8259 PIC */
/*
    ADDED NOTES 3/16/2024
    - we do not need to lock and unlock
    - outb_p is the same thing as outb and io_wait()
        - we want to wait because the commands take time for it to work
    - our code is set up so outb(COMMAND, PORT)
*/

/*
 * i8259_init
 *  DESC: this function initilize PIC
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: masking other interrupt for a while
*/
void i8259_init(void) {

    //mask interrupts
    cli();

    //mask 0xff the data ports
    outb(MASK_ALL, MASTER_DATA);
    outb(MASK_ALL, SLAVE_DATA);

    //ICW 1 (selecting the primary and secondary PIC)
    outb(ICW1, MASTER_COMMAND);
    outb(ICW1, SLAVE_COMMAND);

    //ICW 2 (map IR0 - IR7 to 0x20 - 0x27, and map IR0 - IR7 to 0x28 - 0x2f)
    outb(ICW2_MASTER, MASTER_DATA);
    outb(ICW2_SLAVE, SLAVE_DATA);

    //ICW 3 (Master PIC has a slave PIC connected to its IRQ2, tell slave PIC that its a slave)
    outb(ICW3_MASTER, MASTER_DATA);
    outb(ICW3_SLAVE, SLAVE_DATA);

    //ICW 4 (tells PICs to use a certain type of mode)
    outb(ICW4, MASTER_DATA);
    outb(ICW4, SLAVE_DATA);

    //mask 0xff the data ports
    outb(MASK_ALL, MASTER_DATA);
    outb(MASK_ALL, SLAVE_DATA);

    enable_irq(2); //enable IRQ2 on master pic because it connects to slave pic

    //unmask interrupts
    sti();

}

/*
 * enable_irq
 *  DESC: this function unmask some PIC ports
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
*/
void enable_irq(uint32_t irq_num) {
    
    uint16_t port;
    uint16_t value;

    if(irq_num < EIGHT) // check we need to enable an IRQ on master
    {
        port = MASTER_DATA;
    }
    else // check we need to enable an IRQ on slave
    {
        port = SLAVE_DATA;
        irq_num = irq_num - EIGHT; //IRQ number is from 0 to 7
    }

     //important note. We left shift 0000 0001 based on the irq_num. 
    /* 
        if we want to unmask IRQ1, we want to do inb(port) & 1111 1101. 
    */
    value = inb(port) & ~(1 << irq_num);
    outb(value, port);


}


/*
 * disable_irq
 *  DESC: this function disable some IRQ
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
*/
void disable_irq(uint32_t irq_num) {

    uint16_t port;
    uint16_t value;

    if(irq_num < EIGHT) // check we need to enable an IRQ on master
    {
        port = MASTER_DATA;
    }
    else // check we need to enable an IRQ on slave
    {
        port = SLAVE_DATA;
        irq_num = irq_num - EIGHT; //IRQ number is from 0 to 7
    }

    //important note. We left shift 0000 0001 based on the irq_num. 
    /* 
        if we want to mask IRQ1, we want 0000 0010. 
    */
    value = inb(port) | (1 << irq_num);
    outb(value, port);



}

/*
 * send_eoi
 *  DESC: Send end-of-interrupt signal for the specified IRQ
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
*/
void send_eoi(uint32_t irq_num) {

    //one question however, inside i8259.h, it says this - 
        /* End-of-interrupt byte.  This gets OR'd with
        * the interrupt number and sent out to the PIC
        * to declare the interrupt finished */
    // So OR this with the interrumpt number and send it out

    if(irq_num >= EIGHT) // if IRQ is connected to slave pic, send eoi to slave and master
    {
        outb(EOI | (irq_num - EIGHT), SLAVE_COMMAND); // have to do irq_num - 8 because IRQ is from 0 - 7 on the pics
        outb(EOI | TWO, MASTER_COMMAND); // slave pic is connected to IRQ 2 on the master pic
    }else{
        //if IRQ is on master pic, send eoi to master pic
        outb(EOI | irq_num, MASTER_COMMAND);
    }


}

