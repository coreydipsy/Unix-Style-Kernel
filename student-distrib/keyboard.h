#include "types.h"

#define KEYBOARD_BUFFER_SIZE    128

/* initialize the keyboard*/
extern void keyboard_init();

/* handle a keypress */
extern void keyboard_interrupt_handler();


 /* important variables */
char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
int numchar;
int flag_for_over_80;
int enter;


