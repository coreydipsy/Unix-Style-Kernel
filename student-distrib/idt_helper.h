#include "idt.h"
#include "keyboard.h"
#include "rtc.h"
#include "system_call.h"

/* each of these are just copy/paste assembly linkage, couldn't
   figure out how to do inline assembly */

/* for exceptions */
extern void int_0();
extern void int_1();
extern void int_2();
extern void int_3();
extern void int_4();
extern void int_5();
extern void int_6();
extern void int_7();
extern void int_8();
extern void int_9();
extern void int_10();
extern void int_11();
extern void int_12();
extern void int_13();
extern void int_14();
extern void int_16();
extern void int_17();
extern void int_18();
extern void int_19();

/* for interrupts */
extern void keyboard_helper();
extern void rtc_helper();
extern void system_call_helper();
extern void PIT_helper();
