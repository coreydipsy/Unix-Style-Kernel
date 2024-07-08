    #include "keyboard.h"
    #include "lib.h"
    #include "i8259.h"
    #include "types.h"
    #include "system_call.h"
    #include "paging.h"
    #include "system_call_asm.h"

    #define MAX_CHARS_PER_LINE      80
    #define FD_FOR_KEYBOARD      23

int hello_end_flag = 0;
int random_count = 0;
    //int stats_count;

int generateRandom(int m) {
    static uint32_t seed = 123456; 
    const uint32_t a = 1664525;    
    const uint32_t c = 1013904223; 
    seed = a * seed + c; 

    return seed % m;
}


/* Function to check if a word is in the buffer */
int containsWord(const int8_t* buffer, const int8_t* word) {
    int32_t buffer_len = strlen(buffer);
    int32_t word_len = strlen(word);
    int32_t i, j;

    /* If the word is longer than the buffer, it can't be contained */
    if (word_len > buffer_len) {
        return 0;
    }

    /* Check every possible starting point in the buffer */
    for (i = 0; i <= buffer_len - word_len; i++) {
        int match = 1;  

        /* Check each character of the word */
        for (j = 0; j < word_len; j++) {
            if (buffer[i + j] != word[j]) {
                match = 0;  // Characters do not match
                break;
            }
        }

        /* If all characters matched, the word is contained in the buffer */
        if (match) {
            return 1;
        }
    }

    return 0;  // not found
}

/* terminal_write
 * print either from the file or keyboard
 * Inputs: nbytes is how many chars to write, buf it the buffer it outputs, fd is not used
 * Outputs: error return -1 else 0
 * Side Effects: write to the screen
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    cli();
    int i;
    // int flag_for_dot;
    ternimal_t* terminal_shown_struct_pointer;
    

    if(strncmp((int8_t*)buf,(int8_t*)"391OS> ", sizeof(buf) ) == 0 && terminal_array[current_terminal].cursor_xpos != 0){
        putc('\n');
    }

    if(buf ==NULL){
        return -1;
    }

    terminal_shown_struct_pointer = &terminal_array[terminal_shown];

    // new numchar will be terminal specific
    int * terminal_numchar = &(terminal_shown_struct_pointer->numchar_for_this_terminal);
    if(fd == FD_FOR_KEYBOARD){
        putc_for_keyboard(((char*)buf)[(*terminal_numchar)]);
    }
    else{
       for(i = 0; i < nbytes; i++){
            if(((char*)buf)[i] != '\0'){
                putc(((char*)buf)[i]);
            }
        }     
    }

    sti();
    return nbytes;
}
/* terminal_read
 * copies what is in keybaord buffer into the buf passed in
 * Inputs: nbytes is how many chars to write, buf it the buffer it outputs, fd is not used
 * Outputs: how many bytes we read, -1 of buf is null
 * Side Effects: copies keyboard buffer into buf, also resets numchar and clears keyboard buffer
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    int i;
    int printed = 0;
    //-----------------------------------------------------------------------------------
    //Lebron AI
    if(strncmp((int8_t*)buf,(int8_t*)"Lebron AI", sizeof(buf) ) == 0 || strncmp((int8_t*)buf,(int8_t*)"lebron AI", sizeof(buf) ) == 0 || strncmp((int8_t*)buf,(int8_t*)"lebron ai", sizeof(buf) ) == 0){
        char arr[] = "Starting Lebron AI : )\n Other user program will be disables\n Type 'quit' to exit\n";
        terminal_write(1, (void*)arr, sizeof(arr));  // 20 for size of the error message
        char arr4[] = "--------------------------------------------------------------------------------\n";
        char arr0[] = "Ladies and gentlemen, get ready to witness greatness! \n\n";
        char arr1[] = "Introducing the four-time NBA MVP, three-time NBA champion,\n\n";
        char arr2[] = "Standing at 6 feet 9 inches, wearing number 23 for your Los Angeles Lakers,\n\n";
        char arr3[] = "the King himself ---- LeBron James!\n";
        terminal_write(1, (void*)arr4, sizeof(arr4));
        terminal_write(1, (void*)arr0, sizeof(arr0));
        terminal_write(1, (void*)arr1, sizeof(arr1));
        terminal_write(1, (void*)arr2, sizeof(arr2));
        terminal_write(1, (void*)arr3, sizeof(arr3));
        LebronAI_flag = 1;
        printed = 1;
        putc('\n');
    }

    if(strncmp((int8_t*)buf,(int8_t*)"quit", sizeof(buf) ) == 0  && LebronAI_flag == 1 && printed == 0){
        char arr0[] = "\n''''''''''''''''''''''''''''''''''''''\n";
        char arr1[] = "''''''''''''###############'''''''''''\n";
        char arr2[] = "''''''''''###^^'''''''''^^###'''''''''\n";
        char arr3[] = "''''''''###^               ^###'''''''\n";
        char arr4[] = "''''''''##     ---------      ##''''''\n";
        char arr5[] = "'''''''#     --------------    #''''''\n";
        char arr6[] = "'''''''#                       #''''''\n";
        char arr7[] = "'''''$$#    ####        ####   #$$''''\n";
        char arr8[] = "''''$''#   (00@0)      (0@00)  #''$'''\n";
        char arr9[] = "''''$''#         |'''|         #''$'''\n";
        char arr10[] = "''''$''#         |'''|         #''$'''\n";
        char arr11[] = "'''''$'#        %%'''%%        #'$''''\n";
        char arr12[] = "''''''$#        /@/^\\@\\        #$'''''\n";
        char arr13[] = "'''''''#                       #''''''\n";
        char arr14[] = "'''''''##        .....        ##''''''\n";
        char arr15[] = "'''''''###      /     \\      ###''''''\n";
        char arr16[] = "'''''''#####   /       \\   #####''''''\n";
        char arr17[] = "''''''''######           ######'''''''\n";
        char arr18[] = "'''''''''########     ########''''''''\n";
        char arr19[] = "''''''''''''###############'''''''''''\n";
        char arr20[] = "''''''''''''''##########''''''''''''''\n";
        char arr21[] = "''''''''''''''''''''''''''''''''''''''\n";
        char arr22[] = "--------------------------------------------------------------------------------\n";
        char arr[] = "Turning off Lebron AI : (";
        terminal_write(1, (void*)arr0, sizeof(arr0));
        terminal_write(1, (void*)arr1, sizeof(arr1));
        terminal_write(1, (void*)arr2, sizeof(arr2));
        terminal_write(1, (void*)arr3, sizeof(arr3));
        terminal_write(1, (void*)arr4, sizeof(arr4));
        terminal_write(1, (void*)arr5, sizeof(arr5));
        terminal_write(1, (void*)arr6, sizeof(arr6));
        terminal_write(1, (void*)arr7, sizeof(arr7));
        terminal_write(1, (void*)arr8, sizeof(arr8));
        terminal_write(1, (void*)arr9, sizeof(arr9));
        terminal_write(1, (void*)arr10, sizeof(arr10));
        terminal_write(1, (void*)arr11, sizeof(arr11));
        terminal_write(1, (void*)arr12, sizeof(arr12));
        terminal_write(1, (void*)arr13, sizeof(arr13));
        terminal_write(1, (void*)arr14, sizeof(arr14));
        terminal_write(1, (void*)arr15, sizeof(arr15));
        terminal_write(1, (void*)arr16, sizeof(arr16));
        terminal_write(1, (void*)arr17, sizeof(arr17));
        terminal_write(1, (void*)arr18, sizeof(arr18));
        terminal_write(1, (void*)arr19, sizeof(arr19));
        terminal_write(1, (void*)arr20, sizeof(arr20));
        terminal_write(1, (void*)arr21, sizeof(arr21));
        terminal_write(1, (void*)arr22, sizeof(arr22));


        terminal_write(1, (void*)arr, sizeof(arr)); 
        LebronAI_flag = 0;
        printed = 1;
        putc('\n');
    }

    if((containsWord((int8_t*)buf, (int8_t*)"GOAT") || containsWord((int8_t*)buf, (int8_t*)"goat") || containsWord((int8_t*)buf, (int8_t*)"Jordan") || containsWord((int8_t*)buf, (int8_t*)"jordan") || containsWord((int8_t*)buf, (int8_t*)"mj") || containsWord((int8_t*)buf, (int8_t*)"MJ")) && LebronAI_flag == 1 && printed == 0){
        random_count = (generateRandom(4)+random_count)%4;
        char arr0[] = "Lebron is the GOAT if u disagree you might as well disable me";
        char arr1[] = "Bro Lebron is the GOAT, stop asking";
        char arr2[] = "Lebron James > Michael Jordan";
        char arr3[] = "L E B R O N  J A M E S";
        if(random_count == 0){
            terminal_write(1, (void*)arr0, sizeof(arr0));  
        }else if(random_count == 1){
            terminal_write(1, (void*)arr1, sizeof(arr1));  
        }else if(random_count == 2){
            terminal_write(1, (void*)arr2, sizeof(arr2));  
        }else if(random_count == 3){
            terminal_write(1, (void*)arr3, sizeof(arr3));  
        }
        printed = 1;
        putc('\n');

    }

    if((containsWord((int8_t*)buf, (int8_t*)"Hi") || containsWord((int8_t*)buf, (int8_t*)"hi") || containsWord((int8_t*)buf, (int8_t*)"hello") || containsWord((int8_t*)buf, (int8_t*)"Hello")) && LebronAI_flag == 1 && printed == 0){
        random_count = (generateRandom(2)+random_count)%2;
        char arr0[] = "How wo, How can I help you";
        char arr1[] = "Hi, I can tell u information about LeGoat";
        if(random_count == 0){
            terminal_write(1, (void*)arr0, sizeof(arr0));  
        }else if(random_count == 1){
            terminal_write(1, (void*)arr1, sizeof(arr1));  
        }
        printed = 1;
        putc('\n');
    }


    if((containsWord((int8_t*)buf, (int8_t*)"awards") || containsWord((int8_t*)buf, (int8_t*)"stats") || containsWord((int8_t*)buf, (int8_t*)"stat")) && LebronAI_flag == 1 && printed == 0){
        random_count = (generateRandom(8)+random_count)%8;
        char arr0[] = "4x NBA champion: 2012, 2013, 2016, 2020";
        char arr1[] = "4x NBA Finals Most Valuable Player: 2012, 2013, 2016, 2020";
        char arr2[] = "4x NBA Most Valuable Player: 2009, 2010, 2012, 2013";
        char arr3[] = "20x NBA All-Star: 2005-2024 lmao too long i can't even list it all out";
        char arr4[] = "3x NBA All-Star Game MVP: 2006, 2008, 2018";
        char arr5[] = "19x All-NBA selection:";
        char arr6[] = "6x NBA All-Defensive selection";
        char arr7[] = "3x Olympic medalist";
        if(random_count == 0){
            terminal_write(1, (void*)arr0, sizeof(arr0));
        } else if(random_count == 1){
            terminal_write(1, (void*)arr1, sizeof(arr1));
        } else if(random_count == 2){
            terminal_write(1, (void*)arr2, sizeof(arr2));
        } else if(random_count == 3){
            terminal_write(1, (void*)arr3, sizeof(arr3));
        } else if(random_count == 4){
            terminal_write(1, (void*)arr4, sizeof(arr4));
        } else if(random_count == 5){
            terminal_write(1, (void*)arr5, sizeof(arr5));
        } else if(random_count == 6){
            terminal_write(1, (void*)arr6, sizeof(arr6));
        } else if(random_count == 7){
            terminal_write(1, (void*)arr7, sizeof(arr7));
        }
        printed = 1;
        putc('\n');


    }

    if((containsWord((int8_t*)buf, (int8_t*)"quotes") || containsWord((int8_t*)buf, (int8_t*)"said"))&& LebronAI_flag == 1 && printed == 0){
        random_count = (generateRandom(10)+random_count)%10;
        char arr0[] = "Lebron: You can't be afraid to fail. It's the only way you succeed.";
        char arr1[] = "Lebron: I never get too high on my stardom or what I can do.";
        char arr2[] = "Lebron: I like criticism. It makes you strong.";
        char arr3[] = "Lebron: I feel my calling here goes above basketball.";
        char arr4[] = "Lebron: I'm gonna use all my tools, god-given ability, and make the best life I can.";
        char arr5[] = "Lebron: I always say, decisions I make, I live with them.";
        char arr6[] = "Lebron: I think, team first. It allows me to succeed.";
        char arr7[] = "Lebron: I treated it like every day was my last day with a basketball.";
        char arr8[] = "Lebron: The first time I stepped on an NBA court I became a businessman.";
        char arr9[] = "Lebron: I laugh and joke, but I don't get distracted very easily.";
        if(random_count == 0){
            terminal_write(1, (void*)arr0, sizeof(arr0));
        } else if(random_count == 1){
            terminal_write(1, (void*)arr1, sizeof(arr1));
        } else if(random_count == 2){
            terminal_write(1, (void*)arr2, sizeof(arr2));
        } else if(random_count == 3){
            terminal_write(1, (void*)arr3, sizeof(arr3));
        } else if(random_count == 4){
            terminal_write(1, (void*)arr4, sizeof(arr4));
        } else if(random_count == 5){
            terminal_write(1, (void*)arr5, sizeof(arr5));
        } else if(random_count == 6){
            terminal_write(1, (void*)arr6, sizeof(arr6));
        } else if(random_count == 7){
            terminal_write(1, (void*)arr7, sizeof(arr7));
        } else if(random_count == 8){
            terminal_write(1, (void*)arr8, sizeof(arr8));
        } else if(random_count == 9){
            terminal_write(1, (void*)arr9, sizeof(arr9));
        }
        printed = 1;
        putc('\n');

    }

    if((containsWord((int8_t*)buf, (int8_t*)"ECE391") || containsWord((int8_t*)buf, (int8_t*)"391") || containsWord((int8_t*)buf, (int8_t*)"ECE"))&& LebronAI_flag == 1 && printed == 0){
        random_count = (generateRandom(6)+random_count)%6;
        char arr0[] = "This class is too hard bruh";
        char arr1[] = "why modex, modex bad";
        char arr2[] = "I enjoy doing ECE391";
        char arr3[] = "I am either doing ECE391 or glazing Lebron";
        char arr4[] = "Plz give me extra credits I am an AI";
        char arr5[] = "No one has ever done this, unlike all the other extra credits";
        if(random_count == 0){
            terminal_write(1, (void*)arr0, sizeof(arr0));
        } else if(random_count == 1){
            terminal_write(1, (void*)arr1, sizeof(arr1));
        } else if(random_count == 2){
            terminal_write(1, (void*)arr2, sizeof(arr2));
        } else if(random_count == 3){
            terminal_write(1, (void*)arr3, sizeof(arr3));
        } else if(random_count == 4){
            terminal_write(1, (void*)arr4, sizeof(arr4));
        } else if(random_count == 5){
            terminal_write(1, (void*)arr5, sizeof(arr5));
        }
        printed = 1;
        putc('\n');

    }

    if((containsWord((int8_t*)buf, (int8_t*)"corey") || containsWord((int8_t*)buf, (int8_t*)"Corey") || containsWord((int8_t*)buf, (int8_t*)"kaiyiyu2"))&& LebronAI_flag == 1 && printed == 0){
        random_count = (generateRandom(4)+random_count)%4;
        char arr0[] = "If you are talking shit about my creator, stop, get some help";
        char arr1[] = "Corey is smart plz give him extra credit";
        char arr2[] = "What?";
        char arr3[] = "Hmmm wdym by that";
        if(random_count == 0){
            terminal_write(1, (void*)arr0, sizeof(arr0));
        } else if(random_count == 1){
            terminal_write(1, (void*)arr1, sizeof(arr1));
        } else if(random_count == 2){
            terminal_write(1, (void*)arr2, sizeof(arr2));
        } else if(random_count == 3){
            terminal_write(1, (void*)arr3, sizeof(arr3));
        } 
        printed = 1;
        putc('\n');

    }

    if((containsWord((int8_t*)buf, (int8_t*)"fun") || containsWord((int8_t*)buf, (int8_t*)"fact")) && LebronAI_flag == 1 && printed == 0){
        random_count = (generateRandom(8)+random_count)%8;
        char arr0[] = "Lebron James is Ambidextrous";
        char arr1[] = "LeBron James has a stake in Liverpool FC";
        char arr2[] = "LeBron founded a school for at-risk kids in his hometown of Akron.";
        char arr3[] = "He's a luxury car enthusiast with a Lamborghini painted like a pair of his Nike shoes.";
        char arr4[] = "LeBron is a food lover with investments in Blaze Pizza, a fast-growing restaurant chain.";
        char arr5[] = "LeBron famously celebrates 'Taco Tuesday'on social media posts, complete with a catchy yell.";
        char arr6[] = "His fear of the dark is well-known, leading teammates to tease him about needing a nightlight.";
        char arr7[] = "LeBron once hosted 'SNL' and performed in a sketch where he danced and sang in a wig.";
        if(random_count == 0){
            terminal_write(1, (void*)arr0, sizeof(arr0));
        } else if(random_count == 1){
            terminal_write(1, (void*)arr1, sizeof(arr1));
        } else if(random_count == 2){
            terminal_write(1, (void*)arr2, sizeof(arr2));
        } else if(random_count == 3){
            terminal_write(1, (void*)arr3, sizeof(arr3));
        } else if(random_count == 4){
            terminal_write(1, (void*)arr4, sizeof(arr4));
        } else if(random_count == 5){
            terminal_write(1, (void*)arr5, sizeof(arr5));
        } else if(random_count == 6){
            terminal_write(1, (void*)arr6, sizeof(arr6));
        } else if(random_count == 7){
            terminal_write(1, (void*)arr7, sizeof(arr7));
        } 
        printed = 1;
        putc('\n');

    }

    if((containsWord((int8_t*)buf, (int8_t*)"ls") || containsWord((int8_t*)buf, (int8_t*)"hello") || containsWord((int8_t*)buf, (int8_t*)"pingpong") || containsWord((int8_t*)buf, (int8_t*)"fish")) && LebronAI_flag == 1 && printed == 0){
        random_count = (generateRandom(3)+random_count)%3;
        char arr0[] = "This is Lebron AI not shell";
        char arr1[] = "type quit if you want to use shell bruh";
        char arr2[] = "do i look like shell bruh";
        char arr3[] = "I AM NOT SHELL LOOK AT THE COLOR";
        if(random_count == 0){
            terminal_write(1, (void*)arr0, sizeof(arr0));
        } else if(random_count == 1){
            terminal_write(1, (void*)arr1, sizeof(arr1));
        } else if(random_count == 2){
            terminal_write(1, (void*)arr2, sizeof(arr2));
        } else if(random_count == 3){
            terminal_write(1, (void*)arr3, sizeof(arr3));
        } 
        printed = 1;
        putc('\n');

    }

    if((containsWord((int8_t*)buf, (int8_t*)"Who") || containsWord((int8_t*)buf, (int8_t*)"who") || containsWord((int8_t*)buf, (int8_t*)"Lebron") || containsWord((int8_t*)buf, (int8_t*)"lebron")) && LebronAI_flag == 1 && printed == 0){
        char arr0[] = "\nLeBron James is a celebrated NBA player, often hailed as the greatest.\n\n";
        char arr1[] = "He currently plays for the Los Angeles Lakers, earning multiple MVP awards.\n\n";
        char arr2[] = "James has secured several NBA championships, showcasing his dominance.\n\n";
        char arr3[] = "Off the court, he's known for philanthropy and influential business ventures.";

        terminal_write(1, (void*)arr0, sizeof(arr0));
        terminal_write(1, (void*)arr1, sizeof(arr1));
        terminal_write(1, (void*)arr2, sizeof(arr2));
        terminal_write(1, (void*)arr3, sizeof(arr3));
        printed = 1;
        putc('\n');

    }

    if((containsWord((int8_t*)buf, (int8_t*)"recent games") || containsWord((int8_t*)buf, (int8_t*)"results") || containsWord((int8_t*)buf, (int8_t*)"recent") || containsWord((int8_t*)buf, (int8_t*)"games")) && LebronAI_flag == 1 && printed == 0) {
        char arr0[] = "\nDATE   | OPP   | PTS  |  REB  |  A   |   S   |   BLK  |  MIN   | RES\n";
        char arrl[] = "------------------------------------------------------------------------------\n";
        char arr1[] = "Apr 29   @DEN    30      9       11      4       0       44:05   L: 108-106\n";
        char arr2[] = "Apr 27   DEN     30      5       4       3       1       38:50   W: 119-108\n";
        char arr3[] = "Apr 25   DEN     26      6       9       2       1       42:09   L: 112-105\n";
        char arr4[] = "Apr 22   @DEN    26      8       12      2       2       38:03   L: 101-99\n";
        char arr5[] = "Apr 20   @DEN    27      6       8       1       1       40:45   L: 114-103\n";

        terminal_write(1, (void*)arr0, sizeof(arr0));
        terminal_write(1, (void*)arrl, sizeof(arrl));
        terminal_write(1, (void*)arr1, sizeof(arr1));
        terminal_write(1, (void*)arr2, sizeof(arr2));
        terminal_write(1, (void*)arr3, sizeof(arr3));
        terminal_write(1, (void*)arr4, sizeof(arr4));
        terminal_write(1, (void*)arr5, sizeof(arr5));
        printed = 1;
        putc('\n');
    }


    if(LebronAI_flag == 1 && printed == 0){
        random_count = (generateRandom(10)+random_count)%10;
        char arr0[] = "I don't understand what you mean LOL";
        char arr1[] = "What are you yapping lmao";
        char arr2[] = "I am a text based AI, can't do";
        char arr3[] = "That's a wild statement not gonna lie";
        char arr4[] = "Are you mad rn hahaha";
        char arr5[] = "You can ask me GOAT, Lebron's awards, quotes, facts and recent games";
        char arr6[] = "Dude you need to calm down";
        char arr7[] = "I have a really smart friend called chatGPT, have u considered asking him";
        char arr8[] = "I don't know lol";
        char arr9[] = "That's crazy";
        if(random_count == 0){
            terminal_write(1, (void*)arr0, sizeof(arr0));
        } else if(random_count == 1){
            terminal_write(1, (void*)arr1, sizeof(arr1));
        } else if(random_count == 2){
            terminal_write(1, (void*)arr2, sizeof(arr2));
        } else if(random_count == 3){
            terminal_write(1, (void*)arr3, sizeof(arr3));
        } else if(random_count == 4){
            terminal_write(1, (void*)arr4, sizeof(arr4));
        } else if(random_count == 5){
            terminal_write(1, (void*)arr5, sizeof(arr5));
        } else if(random_count == 6){
            terminal_write(1, (void*)arr6, sizeof(arr6));
        } else if(random_count == 7){
            terminal_write(1, (void*)arr7, sizeof(arr7));
        } else if(random_count == 8){
            terminal_write(1, (void*)arr8, sizeof(arr8));
        } else if(random_count == 9){
            terminal_write(1, (void*)arr9, sizeof(arr9));
        }
        putc('\n');


    }

    //-----------------------------------------------------------------------------------
    // terminal shown is the one keyboard wants to write to
    ternimal_t* terminal_shown_struct_pointer = &terminal_array[current_terminal];

    // new keyboard buffer will be terminal_shown_struct_pointer->terminal_buffer
    // i am just trying to make the expression easier to read
    char* terminal_shown_buf = terminal_shown_struct_pointer->terminal_buffer;

    // new numchar will be terminal specific
    int * terminal_numchar = &(terminal_shown_struct_pointer->numchar_for_this_terminal);
    terminal_array[current_terminal].enter_flag =0;
    terminal_array[current_terminal].over_80_flag = 0;
    if(buf ==NULL){
        return -1;
    }
    while(terminal_array[current_terminal].enter_flag == 0){ // only run the code after this if enter is pressed
        continue;
    }
    //terminal_switch_in_progress = 0;
    int read;
if(nbytes < (*terminal_numchar)){ // read nbytes even if its not everything
    for(i = 0; i < nbytes; i++){
        if(terminal_shown_buf[i] != '\0'){
            ((char*)buf)[i] = terminal_shown_buf[i];
        }
    }
    read = nbytes;
}
if(nbytes >= (*terminal_numchar)){
    for(i = 0; i < (*terminal_numchar); i++){ // read everything in the buffer and end once we do
        if(terminal_shown_buf[i] != '\0'){
            ((char*)buf)[i] = terminal_shown_buf[i];
        }
    }
    read = (*terminal_numchar);
}

    (*terminal_numchar) =0;
    memset(terminal_shown_buf,'\0',sizeof(terminal_shown_buf)); // reset buffer since enter flag has been active
    return read;
}
/* terminal_open
 * opens the terminal
 * Inputs: it is the file passed in
 * Outputs: 0
 * Side Effects: nothing
 */
int32_t terminal_open(const uint8_t* filename, int32_t fd){
    return 0;
}
/* terminal_close
 * closes terminal
 * Inputs: file direct passed in
 * Outputs: 0
 * Side Effects: nothing
 */
int32_t terminal_close(int32_t fd){
    return 0;
}

