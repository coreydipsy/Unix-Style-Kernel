#include "paging.h"
#include "x86_desc.h"
#include "paging_asm.h"
#include "system_call.h"

/*
 * paging_init
 *  DESC: sets up paging with a lot of pages
 *  INPUT: none
 *  OUTPUT: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: paging
*/
void paging_init() {
    /* initialize tables of inactive entries */
    init_empty_pdt(page_directory_table);
    init_empty_ptt(first_page_table);
    init_empty_ptt(user_vid_table);

    /* init the first pde entry to be active and point to the first_page_table */
    init_pde_entry(page_directory_table, VID_MEM_PDT_IDX, ACTIVE, SUPERVISOR, READ_WRITE, WITTLE_PAGE, (void*)first_page_table);

    /* init the second pde entry to be active and point directly to the kernel page (big page) */
    init_pde_entry(page_directory_table, KERNEL_PAGE_IDX, ACTIVE, SUPERVISOR, READ_WRITE, BIG_PAGE, (void*)KERNEL_PAGE_ADDR);

    /* init the first entry in the first_page_table to point to video memory */
    init_pte_entry(first_page_table, VID_MEM_PTT_IDX, ACTIVE, SUPERVISOR, READ_WRITE, (void*)VID_MEM_PAGE_ADDR);

    /* whats happening here is we are creating new pages so that we can store the video memory for the 3 terminals */
    init_pte_entry(first_page_table, VID_MEM_PTT_IDX+1, ACTIVE, SUPERVISOR, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR+(FOUR_KB_PAGE)));
    init_pte_entry(first_page_table, VID_MEM_PTT_IDX+2, ACTIVE, SUPERVISOR, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR+(2*FOUR_KB_PAGE)));
    init_pte_entry(first_page_table, VID_MEM_PTT_IDX+3, ACTIVE, SUPERVISOR, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR+(3*FOUR_KB_PAGE)));
    init_pte_entry(first_page_table, VID_MEM_PTT_IDX+4, ACTIVE, SUPERVISOR, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR)); // this is an auxilary page for the keyboard to write to

    /* init the first pde entry to be active and point to the first_page_table */
    // 34 is the next next page, two after 32, the reason why i put it at 34 is i dont want it to be hard to debug if user program accidentally write to this page
    init_pde_entry(page_directory_table, USER_VID_PAGE_IDX, ACTIVE, USER, READ_WRITE, WITTLE_PAGE, (void*)user_vid_table);
    init_pte_entry(user_vid_table, VID_MEM_PTT_IDX, ACTIVE, USER, READ_WRITE, (void*)VID_MEM_PAGE_ADDR);

    /* multiple user pages, similar to above part */
    init_pte_entry(user_vid_table, VID_MEM_PTT_IDX+1, ACTIVE, USER, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR+(FOUR_KB_PAGE)));
    init_pte_entry(user_vid_table, VID_MEM_PTT_IDX+2, ACTIVE, USER, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR+(2*FOUR_KB_PAGE)));
    init_pte_entry(user_vid_table, VID_MEM_PTT_IDX+3, ACTIVE, USER, READ_WRITE, (void*)(VID_MEM_PAGE_ADDR+ (3*FOUR_KB_PAGE)));

    /* init a page for the user program */
    init_pde_entry(page_directory_table, USER_PRG_PAGE_IDX, ACTIVE, USER, READ_WRITE, BIG_PAGE, (void*)USER_PRG1_PAGE_ADDR);

    /* jump to asm file to actually initialize for paging */
    loadPageDirectory(page_directory_table);
    enablePaging();

}

/*
 * set_user_page
 *  DESC: changes where the user page maps to in physical memory, basically an api call
 *  INPUT: new_pid - the pid for the new process
 *  OUTPUT: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: changes the paging, make sure to flush tlb
*/
void set_user_page(int32_t new_pid) {
    init_pde_entry(page_directory_table, USER_PRG_PAGE_IDX, ACTIVE, USER, READ_WRITE, BIG_PAGE, (void*)USER_PHYS_MEM_PAGE_ADDR(new_pid));
}

/*
 * init_empty_pdt
 *  DESC: initializes the pdt of inactive entries that go nowhere
 *  INPUT: table - ptr to the pdt
 *  OUTPUT: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
*/
void init_empty_pdt(pde_entry* table) {
    int i = 0;
    for (i = 0; i < PDE_TABLE_SIZE; i++) {
        init_pde_entry(table, i, INACTIVE, SUPERVISOR, READ, WITTLE_PAGE, (void*)NULL_ADDR);
    }
}

/*
 * init_empty_ptt
 *  DESC: initializes the ptt of inactive entries that go nowhere
 *  INPUT: table - ptr to the page table
 *  OUTPUT: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
*/
void init_empty_ptt(pte_entry* table) {
    int i = 0;
    for (i = 0; i < PTE_TABLE_SIZE; i++) {
        init_pte_entry(table, i, INACTIVE, SUPERVISOR, READ, (void*)NULL_ADDR);
    }
}


/*
 * init_pde_entry
 *  DESC: initializes an entry in the pdt
 *  INPUT: table - ptr to the pdt
 *         idx - index of entry
 *         status, priv, rw, size - fields of the struct that we need to initialize
 *                                - we need size to distinguish 4MB vs 4KB pages
 *         addr - address of where we need to map to. since the table is aligned
 *                to 4kb, we right shift by 12 because we don't lose any information
 *                (0x1000 >> 12 loses no data)
 *  OUTPUT: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
*/
void init_pde_entry(pde_entry* table, int idx, int status, int priv, int rw, int size, void* addr) {
    table[idx].present          = status;
    table[idx].read_write       = rw;
    table[idx].user_super       = priv;
    table[idx].write_through    = 0;
    table[idx].cache_disable    = 0;
    table[idx].accessed         = 0;
    table[idx].avl_bit          = 0;
    table[idx].page_size        = size;
    table[idx].avl              = 0;
    table[idx].addr             = ((int)addr >> ALIGN_4KB);
}

/*
 * init_pte_entry
 *  DESC: initializes an entry in the pdt
 *  INPUT: table - ptr to the pdt
 *         idx - index of entry
 *         status, priv, rw, size - fields of the struct that we need to initialize
 *         addr - address of where we need to map to. since the table is aligned
 *                to 4kb, we right shift by 12 because we don't lose any information
 *                (0x1000 >> 12 loses no data)
 *  OUTPUT: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
*/
void init_pte_entry(pte_entry* table, int idx, int status, int priv, int rw, void* addr) {
    table[idx].present        = status;
    table[idx].read_write     = rw;
    table[idx].user_super     = priv;
    table[idx].write_through  = 0;
    table[idx].cache_disable  = 0;
    table[idx].accessed       = 0;
    table[idx].dirty          = 0;
    table[idx].pg_attr_tble   = 0;
    table[idx].global         = 0;
    table[idx].avl            = 0;
    table[idx].addr           = ((int)addr >> ALIGN_4KB);
}
