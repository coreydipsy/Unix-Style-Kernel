#include "tests.h"
#include "x86_desc.h"
// #include "lib.h"
#include "filesys.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0
#define FD 3 // temporart file descriptor for cp2

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	clear();
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}


/* divide_by_0_test
 * 
 * divide by 0, should cause divide by 0 exception 
 * Inputs: None
 * Outputs: None
 * Side Effects: Cause Exception
 * Coverage: IDT
 */
int divide_by_0_test() {
	TEST_HEADER;
	int i = 1;
	int j = 0;
	//assertion_failure();
	return i/j;
}

/* invalid_opcode_test
 * 
 * give an undefined instruction, should cause invalid opcode exception 
 * Inputs: None
 * Outputs: None
 * Side Effects: Cause Exception
 * Coverage: IDT
 */
void invalid_opcode_test() {
	TEST_HEADER;
    asm volatile("ud2"); // UD2 is an undefined instruction.
}


/* dereference_null_test
 * 
 * try to dereference null, should cause page fault exception 
 * Inputs: None
 * Outputs: Int (doesn't matter)
 * Side Effects: Cause Exception
 * Coverage: IDT
 */
int dereference_null_test() {
	TEST_HEADER;
    int* i = NULL;
	return *i;
}


/* num_exception_test
 * 
 * select which exception to raise 
 * Inputs: None
 * Outputs: None
 * Side Effects: Cause Exception
 * Coverage: IDT
 */
void num_exception_test() {
	TEST_HEADER;
    __asm__("int $0x0B");
}

/* Paging tests, try dereferencing outside of paged memory, results in page fault exception */
/* these test around the video memory page */
int page_test_1() {
	TEST_HEADER;
	int* i = (void*)0xB7FFF;	// fail
	return *i;
}
int page_test_2() {
	TEST_HEADER;
	int* i = (void*)0xB8200;	// success
	return *i;
}
int page_test_3() {
	TEST_HEADER;
	int* i = (void*)0xB8FFB;	// success
	return *i;
}
int page_test_4() {
	TEST_HEADER;
	int* i = (void*)0xB9000;	// fail
	return *i;
}

/* these test around kernel memory page */
int page_test_5() {
	TEST_HEADER;
	int* i = (void*)0x39FFFF;	// fail
	return *i;
}
int page_test_6() {
	TEST_HEADER;
	int* i = (void*)0x400000;	// success
	return *i;
}
int page_test_7() {
	TEST_HEADER;
	int* i = (void*)0x79FFFF;	// success
	return *i;
}
int page_test_8() {
	TEST_HEADER;
	int* i = (void*)0x800000;	// fail
	return *i;
}
/* misc */
int page_test_9() {
	TEST_HEADER;
	int* i = (void*)0xF00000;	// fail
	return *i;
}

int page_test_10() {
	int* i = (void*)0x08048000;	// pass
	return *i;
}

int page_test_11() {
	int* i = (void*)0x08000000;	// pass
	return *i;
}

int page_test_12() {
	int* i = (void*)(0x08000000 + 0x400000);	// pass
	return *i;
}


/* encapsulation for paging tests */
/* test_paging
 * 
 * dereference the incorrect address, test virtual address
 * Inputs: None
 * Outputs: None
 * Side Effects: Cause Page fault Exception
 * Coverage: Paging
 */
void test_paging() {
	// success ones
	// page_test_2();
	// page_test_3();
	// page_test_6();
	// page_test_7();

	// user segment pass
	// page_test_10();
	// page_test_11();

	// user segment fail
	//page_test_12();

	// fail ones
	//page_test_1();
	// page_test_4();
	// page_test_5();
	// page_test_8();
	// page_test_9();
}

/* Checkpoint 2 tests */

// buffer for testing
static uint8_t buffer_for_file_system[10000];

/* test_filesys_frame0
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: print frame0 to the screen
 * Coverage: file system
 */
void test_filesys_frame0() {
	int i;
	char* name = "frame0.txt";
	
	int open = open_file_execute((const uint8_t*)name);
	int read = read_file(FD, buffer_for_file_system, 187);// file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
	//return;

	for(i = 0; i < 187; i++) {
        putc_for_files(buffer_for_file_system[i]);
    }

	printf("\nfile name: frame0.txt\n");


}


/* test_filesys_frame1
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: print frame1 to the screen
 * Coverage: file system
 */
void test_filesys_frame1() {
	int i;
	char* name = "frame1.txt";
	
	int open = open_file_execute((const uint8_t*)name);
	int read = read_file(FD, buffer_for_file_system, 187);// file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
	//return;

	for(i = 0; i < 187; i++) {
        putc_for_files(buffer_for_file_system[i]);
    }

	printf("\nfile name: frame1.txt\n");

}


/* test_filesys_very_large_name_good
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: print verylargetextwithverylongname to the screen, this should print
 * Coverage: file system
 */
void test_filesys_very_large_name_good() {
	int i;
	char* name = "verylargetextwithverylongname.tx";
	
	int open = open_file_execute((const uint8_t*)name);
	int read = read_file(FD, buffer_for_file_system, 5277);// file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
	//return;

	for(i = 0; i < 5277; i++) {
        putc_for_files(buffer_for_file_system[i]);
    }

	printf("\nfile name: verylargetextwithverylongname.tx\n");
}


/* test_filesys_very_large_name_bad
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: print verylargetextwithverylongname to the screen, this should not print
 * Coverage: file system
 */
void test_filesys_very_large_name_bad() {
	int i;
	char* name = "verylargetextwithverylongname.txt";
	
	int open = open_file_execute((const uint8_t*)name);
	int read = read_file(FD, buffer_for_file_system, 600);// file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
	//return;

	for(i = 0; i < 600; i++) {
        putc_for_files(buffer_for_file_system[i]);
    }

	printf("\nfile name: verylargetextwithverylongname.txt\n");

}


/* test_filesys_fish
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: print fish to the screen
 * Coverage: file system
 */
void test_filesys_fish() {
	int i;
	char* name = "fish";
	// buffer for fish
	static uint8_t buffer_for_fish[36200]; // bigger than 36164 which is the fish size

	
	int open = open_file_execute((const uint8_t*)name);
	int read = read_file(FD, buffer_for_fish, 36200); //36164, but only take the first 500 // file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
	//return;

	for(i = 0; i < 36200; i++) {
		if(buffer_for_fish[i] == '\0'){continue;}
        putc_for_files(buffer_for_fish[i]);
    }

	printf("\nfile name: fish\n");

}


/* test_filesys_shell
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: print shell to the screen
 * Coverage: file system
 */
void test_filesys_shell() {
	int i;
	char* name = "shell";
	
	int open = open_file_execute((const uint8_t*)name);
	int read = read_file(FD, buffer_for_file_system, 5605);// file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
	//return;

	for(i = 0; i < 5605; i++) {
		if(buffer_for_file_system[i] == '\0'){continue;}
        putc_for_files(buffer_for_file_system[i]);
    }

	printf("\nfile name: shell\n");
}


/* test_filesys_grep
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: print grep to the screen
 * Coverage: file system
 */
void test_filesys_grep() {
	int i;
	char* name = "grep";
	
	int open = open_file_execute((const uint8_t*)name);
	int read = read_file(FD, buffer_for_file_system, 6150);// file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
	//return;

	for(i = 0; i < 6150; i++) {
		if(buffer_for_file_system[i] == '\0'){continue;}
        putc_for_files(buffer_for_file_system[i]);
    }

	printf("\nfile name: grep\n");
}


/* test_filesys_cat
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: print cat to the screen
 * Coverage: file system
 */
void test_filesys_cat() {
	int i;
	char* name = "cat";
	
	int open = open_file_execute((const uint8_t*)name);
	int read = read_file(FD, buffer_for_file_system, 5445);// file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
	//return;

	for(i = 0; i < 5445; i++) {
		if(buffer_for_file_system[i] == '\0'){continue;}
        putc_for_files(buffer_for_file_system[i]);
    }
	printf("\nfile name: cat\n");


}

/* test_filesys_ls
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: print ls to the screen
 * Coverage: file system
 */
void test_filesys_ls() {
	int i;
	char* name = "ls";
	
	int open = open_file_execute((const uint8_t*)name);
	int read = read_file(FD, buffer_for_file_system, 5349);// file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
	//return;

	for(i = 0; i < 5349; i++) {
		if(buffer_for_file_system[i] == '\0'){continue;}
        putc_for_files(buffer_for_file_system[i]);
    }

	// // for seeing ELF
	// for(i = 0; i < 100; i++) {
	// 	if(buffer_for_file_system[i] == '\0'){continue;}
    //     putc(buffer_for_file_system[i]);
    // }
	printf("\nfile name: ls\n");


}



/* open_dir_test
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: open the directory and print directory files to the screen
 * Coverage: file system
 */
void open_dir_test() {
	char* name = ".";
	
	int open = open_dir((const uint8_t*)name, 0);
	int read = read_dir(FD, buffer_for_file_system, 6000);// file size

	if (open) printf("ERROR IN OPEN");
	if (read) printf("ERROR IN READ");
}

/* encapsulation for file systems tests */
/* test_filesys
 * 
 * calling different tests
 * Inputs: None
 * Outputs: None
 * Side Effects: print to the screen
 * Coverage: file system
 */
void test_filesys() {
	clear();
	// open_dir_test();
	// test_filesys_frame0();
	 test_filesys_frame1();
	// test_filesys_shell();
	// test_filesys_grep();
	// test_filesys_very_large_name_good();
	// test_filesys_very_large_name_bad();
	// test_filesys_cat();
	// test_filesys_fish();
	// test_filesys_ls();
}

/*
 * test_rtc_all
 *  DESC: iterates through all frequencies and prints the the screen
 *  INPUT/OUTPUT: none
 *  SIDE EFFECTS: writes to the screen
 * doesnt support this after cp2
*/
// void test_rtc_all() {
// 	int rate = 2, i;
// 	while (rate <= 32768) {
// 		rtc_write(rate);
// 		for (i = 0; i <= rate; i++) {
// 			rtc_read();
// 			putc_for_files('L');
// 		}
// 		rate *= 2;
// 		clear();
// 	}
// }


/*
 * rtc_tests
 *  DESC: tests the rtc
 *  INPUTS: none
 *  OUTPUTS: none
 *  SIDE EFFECTS: writes to the screen, changes the rtc
*/
void test_rtc() {
	//rtc_open(); 
	//rtc_write(2); 
	//rtc_write(4); 
	//rtc_write(8); 
	//rtc_write(16); 
	//rtc_write(32); 
	//rtc_write(64); 
	//rtc_write(128); 
	//rtc_write(256); 
	//rtc_write(512); 
	//rtc_write(1024); 
	// rtc_write(2048); 
	//rtc_write(4096); 
	// rtc_write(8192); 
	// rtc_write(16384);
	//rtc_write(32768); 
	// while(1)
	// {
	// 	printf("rtc_read_initiate\n");
	// 	rtc_read();
	// 	printf("rtc_read_finished\n");
	// }
	//test_rtc_all();
}

/*
 * test_keyboard
 *  DESC: clears the screen and then writes
 *  INPUTS/OUTPUTS: none
*/
void test_keyboard() {
	clear(); // comment this out if you do not want the cursor to start at the top 
    //left of the screen and to see the intial boot info this is for terminal testing
    int z;
    z=10;
    char arr[128]="";
    int a;
    while(z>0){
        a =terminal_read(2, (void* )arr,128);
        terminal_write(2, (void*)arr, a);

    }

}




// add more tests here


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	clear();
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("divide_by_zero_test", divide_by_0_test());
	// divide_by_0_test();
	// invalid_opcode_test();
	//	test_page_fault_exception();
	// dereference_null_test();
	// num_exception_test();
	// test_paging();
	// test_filesys();
	// test_rtc();
	// test_keyboard();
}
