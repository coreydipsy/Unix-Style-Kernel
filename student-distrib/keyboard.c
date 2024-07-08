    #include "keyboard.h"
    #include "system_call.h"
    #include "lib.h"
    #include "i8259.h"
    #include "types.h"
    #include "terminal.h"


    #define KEYBOARD_IRQ    0x01
    #define DATA_PORT       0x60

    // screen size
    #define MAX_CHARS_PER_LINE      80

    // hex values for these keys
    #define CTRL                0x1D
    #define CTRL_RELEASE        0x9D
    #define SHIFT_L             0x2A
    #define SHIFT_L_RELEASE     0xAA
    #define SHIFT_R             0x36
    #define SHIFT_R_RELEASE     0xB6

    #define CAPS_LOCK           0x3A
    #define ALT_L               0x38
    #define ALT_L_RELEASE       0xB8 //both left and right alt are the same
    #define ENTER               0x1C
    #define ENTER_RELEASE       0x9C
    #define BACKSPACE           0x0E
    #define L                   0x26
    #define KEYPAD_STAR         0x37
    #define F1                  0x3B
    #define F2                  0x3C
    #define F3                  0x3D
    #define TAB                 0x0F

    #define FROM_KEYBOARD_FD        23  // this is the jersery number lebron is wearing
    #define MAX_BUFFER_SIZE         128


    int cap_ctrl_shift(uint8_t input);
    // memset(keyboard_buffer,'\0',sizeof(keyboard_buffer));
    /*
    * keyboard_init()
    *  DESC: initializes the keyboard
    *  INPUT: none
    *  OUTPUT: none
    *  RETURN VALUE: none
    *  SIDE EFFECTS: enables IRQ1 on the PIC
    */
    void keyboard_init() {
        cli();                      // disable interrupts
        enable_irq(KEYBOARD_IRQ);   // set IRQ1 for keyboard
        sti();                      // enable interrupts
    }

    //flags for these keys
    int ctrl =0;
    int shiftL =0;
    int shiftR = 0;
    int caps = 0;
    int altL = 0;
    int enter = 0;

    int flag_for_over_80  = 0;
    //int flag_for_over_80_and back = 0;
    int flag_buffer_full;

    
/* cap_ctrl_shift
 * 
 * Inputs: data_port inb value basically the key pressed
 * Outputs: 1 if it is a key that will impact what array we use when typing, 0 if not
 * Side Effects: raises the flag that was pressed/ lowers flag that was released
 */
    int cap_ctrl_shift(uint8_t input){
        switch(input){
            case CTRL:
                ctrl =1;
                return 1;
            case CTRL_RELEASE:
                ctrl = 0;
                return 1;
            case SHIFT_L:
                shiftL =1;
                return 1;
            case SHIFT_L_RELEASE:
                shiftL=0;
                return 1;
            case SHIFT_R:
                shiftR = 1;
                return 1;
            case SHIFT_R_RELEASE:
                shiftR=0;
                return 1;
            case CAPS_LOCK:
                if(caps == 0){caps = 1;}
                else if(caps == 1){caps = 0;}
                return 1;
            case ALT_L:
                altL = 1;
                return 1;
            case ALT_L_RELEASE:
                altL = 0;
                return 1;
            default:
                return 0;
        }   
    }
    // these 4 array are in the order that they are in hex codes each of these are difference beacuse of the flags that are set
    // based on caps shift it will go to one of these tables
    // the arrays size are all 58 because we only support 58 keys
    char nothing_pressed[58]= {'\0','\0','1','2',
    '3','4','5','6','7','8','9','0','-','=','\b',' ', 
    'q','w','e','r','t','y','u','i','o','p','[',']', '\n','\0',
    'a','s','d','f','g','h','j','k','l',';','\'','`','\0','\\','z',
    'x','c','v','b','n','m',',','.','/','\0','\0','\0',' '}; 
    char only_caps[58]= {'\0','\0','1','2',
    '3','4','5','6','7','8','9','0','-','=','\b',' ', 
    'Q','W','E','R','T','Y','U','I','O','P','[',']', '\n','\0',
    'A','S','D','F','G','H','J','K','L',';','\'','`','\0','\\','Z',
    'X','C','V','B','N','M',',','.','/','\0','\0','\0',' '}; 
    char only_shift[58]= {'\0','\0','!','@',
    '#','$','%','^','&','*','(',')','_','+','\b',' ', 
    'Q','W','E','R','T','Y','U','I','O','P','{','}', '\n','\0',
    'A','S','D','F','G','H','J','K','L',':','\"','~','\0','|','Z',
    'X','C','V','B','N','M','<','>','?','\0','\0','\0',' '}; 
    char shift_caps[58]= {'\0','\0','!','@',
    '#','$','%','^','&','*','(',')','_','+','\b',' ', 
    'q','w','e','r','t','y','u','i','o','p','{','}', '\n','\0',
    'a','s','d','f','g','h','j','k','l',':','\"','~','\0','|','z',
    'x','c','v','b','n','m','<','>','?','\0','\0','\0',' '}; 
    int numchar = 0; // number of things in the keyboard buffer
/* keyboard_interrupt_handler
 * 
 * Inputs: None
 * Outputs: charcter on screen based on flags that are set
 * Side Effects: echos to the screen what is typed  
 */

    void keyboard_interrupt_handler() {
        // printf("LShift: %d", shiftL);    
        cli(); // disable interrupts
        
        // terminal shown is the one keyboard wants to write to
        ternimal_t* terminal_shown_struct_pointer = &terminal_array[terminal_shown];

        // new keyboard buffer will be terminal_shown_struct_pointer->terminal_buffer
        // i am just trying to make the expression easier to read
        char* terminal_shown_buf = terminal_shown_struct_pointer->terminal_buffer;

        // new numchar will be terminal specific
        int * terminal_numchar = &(terminal_shown_struct_pointer->numchar_for_this_terminal);
        uint8_t input = inb(DATA_PORT);
        if(cap_ctrl_shift(input) == 1){
            send_eoi(KEYBOARD_IRQ); 
            sti();                         // this is something like a shift alt etc that does not write anything to the screen
            return;
        }  

        if (altL == 1 && input == F1) {
            //terminal_array[terminal_shown].enter_flag = 1;
            send_eoi(KEYBOARD_IRQ);
            sti();
            terminal_swap(0);
            
            return;
        } else if (altL == 1 && input == F2) {
            //terminal_array[terminal_shown].enter_flag = 1;
            send_eoi(KEYBOARD_IRQ);
            sti();
            //printf("wait what");
            terminal_swap(1);
            
            return;
        } else if (altL == 1 && input == F3) {
            //terminal_array[terminal_shown].enter_flag = 1;
            send_eoi(KEYBOARD_IRQ);
            sti();
            terminal_swap(2);
            
            return;
        }
        
        if(((input >0 && input <KEYPAD_STAR) || (input > KEYPAD_STAR && F1 > input)) && input != 1){
            
            if(input == BACKSPACE && (*terminal_numchar) >0){ //checking for backspace and handle that

                // if((terminal_array[terminal_shown].over_80_flag == 1) && ((*terminal_numchar) == 79)){
                //     putc_for_keyboard('\b');
                //     //keyboard_buffer[numchar]='\0';
                //     terminal_shown_buf[*terminal_numchar]='\0';
                // }
                putc_for_keyboard('\b');
                //numchar--;
                (*terminal_numchar)--;
                //keyboard_buffer[numchar]='\0';
                terminal_shown_buf[*terminal_numchar]='\0';
            }
            else if(input == ENTER){
                putc_for_keyboard('\n'); // handle enter 
                terminal_array[terminal_shown].enter_flag =1;
            }
            else if(ctrl == 1 && input == L && input != BACKSPACE){
                //numchar =0;
                *terminal_numchar = 0;
                clear_for_keyboard(); // ctrl + l
            }
            // 125 = max buffer - tab
            else if (input == TAB && input != BACKSPACE && (*terminal_numchar) < MAX_BUFFER_SIZE - 3)
            {
                // putc('\t');
                //keyboard_buffer[numchar] = ' ';
                    terminal_shown_buf[*terminal_numchar]=' ';
                    terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
                //keyboard_buffer[numchar] = ' ';
                terminal_shown_buf[*terminal_numchar]=' ';
                terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
                //keyboard_buffer[numchar] = ' ';
                terminal_shown_buf[*terminal_numchar]=' ';
                terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
                //keyboard_buffer[numchar] = ' ';
                terminal_shown_buf[*terminal_numchar]=' ';
                terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
            }
            else if(shiftL == 1 && caps ==0 && input != BACKSPACE  && (*terminal_numchar) <MAX_BUFFER_SIZE){ //next few if statements are just to see which arr to use based on flags
                //keyboard_buffer[numchar] = only_shift[input];
                terminal_shown_buf[*terminal_numchar]= only_shift[input];
                // putc(keyboard_buffer[numchar]);
                terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
                // printf("1");
            }
            else if(shiftL == 1 && shiftR == 1 && caps ==0 && input != BACKSPACE  && (*terminal_numchar) <MAX_BUFFER_SIZE){
                // keyboard_buffer[numchar] = only_shift[input];
                terminal_shown_buf[*terminal_numchar]= only_shift[input];
                // putc(keyboard_buffer[numchar]);
                terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
                // printf("1");
            }
            else if(shiftL ==0 && shiftR ==0 && caps ==0 && input != BACKSPACE  && (*terminal_numchar) <MAX_BUFFER_SIZE){
                //keyboard_buffer[numchar] = nothing_pressed[input];
                terminal_shown_buf[*terminal_numchar]= nothing_pressed[input];
                // putc(keyboard_buffer[numchar]);
                terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
                // printf("2");
            }
            else if(caps == 1 && shiftL ==0 && shiftR ==0 && input != BACKSPACE && (*terminal_numchar) <MAX_BUFFER_SIZE){
                //keyboard_buffer[numchar] = only_caps[input];
                terminal_shown_buf[*terminal_numchar]= only_caps[input];
                // putc(keyboard_buffer[numchar]);
                terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
                // printf("3");
            }
            else if(((caps == 1 && shiftL ==1) || (caps == 1 && shiftR ==1)) && (input != BACKSPACE) && (*terminal_numchar) <MAX_BUFFER_SIZE){
                // keyboard_buffer[numchar] = only_shift[input];
                terminal_shown_buf[*terminal_numchar]= nothing_pressed[input];
                // putc(keyboard_buffer[numchar]);
                terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
                // printf("4");
            }
            else if(shiftR == 1 && caps ==0 && input != BACKSPACE && (*terminal_numchar) <MAX_BUFFER_SIZE){
                // keyboard_buffer[numchar] = only_shift[input];
                terminal_shown_buf[*terminal_numchar]= only_shift[input];
                // putc(keyboard_buffer[numchar]);
                terminal_write(FROM_KEYBOARD_FD, terminal_shown_buf, MAX_BUFFER_SIZE);
                //numchar++;
                (*terminal_numchar)++;
                // printf("5");
            }
            // if(hello_flag && ((*terminal_numchar) == 58) && (input != BACKSPACE)){
            //      terminal_array[terminal_shown].over_80_flag = 1;
            //      putc_for_keyboard('\n');
            //      hello_flag = 0;
            //      } 
            //if(terminal_array[terminal_shown].cursor_xpos == 79 && input != BACKSPACE ){ terminal_array[terminal_shown].over_80_flag = 1;putc_for_keyboard('\n');} 
            //if((*terminal_numchar) == 73 && input != BACKSPACE && hello_end_flag){ terminal_array[terminal_shown].over_80_flag = 1;putc_for_keyboard('\n');} 
            //if(numchar == MAX_CHARS_PER_LINE && input != BACKSPACE){putc('\n'); flag_for_over_80 = 1;} // reached max char on screen so new line
            if((*terminal_numchar) == KEYBOARD_BUFFER_SIZE){
                    if(input == ENTER){
                        terminal_array[terminal_shown].enter_flag =1;
                    }
                    else if(input == BACKSPACE){
                        putc_for_keyboard('\b');
                        //numchar--;
                        (*terminal_numchar)--;  
                        //keyboard_buffer[numchar]='\0';
                        terminal_shown_buf[*terminal_numchar]='\0';
                    }
            } // force enter cuz buffer is full
        }
            
            send_eoi(KEYBOARD_IRQ);
            sti();
    }

