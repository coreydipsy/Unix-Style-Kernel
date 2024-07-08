#ifndef PAGING_H
#define PAGING_H

#include "lib.h"

#define PDE_TABLE_SIZE      1024
#define PTE_TABLE_SIZE      1024
#define PAGING_ALIGNMENT    4096    // 4kb alignment

/* important fields of the page directory/table entries, via https://wiki.osdev.org/Paging#Page_Directory */
#define ACTIVE          1
#define INACTIVE        0
#define USER            1
#define SUPERVISOR      0
#define READ_WRITE      1
#define READ            0
#define BIG_PAGE        1
#define WITTLE_PAGE     0

#define VID_MEM_PDT_IDX     0x00    // first index in PDT maps to first_page_table, which is used for video memory

#define VID_MEM_PTT_IDX     0xB8    // in the first_page_table each idx maps to its 4kb equivalent
                                    // in memory (e.g. idx2 -> 0x2000). something to do with lib.c,
                                    // thank you CA's. so to access video memory, we need to use
                                    // the video memory address >> 12 (0xB8000 -> 0xB8)

#define KERNEL_PAGE_IDX     0x01    // second index in PDT maps directly to the kernel page (4MB)

#define USER_PRG_PAGE_IDX       0x20
#define USER_PRG1_PAGE_ADDR     0x800000
#define USER_PRG2_PAGE_ADDR     0xC00000 
#define USER_VID_PAGE_IDX       34


#define VID_MEM_PAGE_ADDR       0xB8000     // given in sanjeevi slides
#define KERNEL_PAGE_ADDR        0x400000    // starts at 4MB
#define NULL_ADDR               0x000000    // magic numbers scare me

#define ALIGN_4KB               12  // used for bit-shifting to align correctly, names self-explanatory
#define ALIGN_4MB               22

#define FOUR_KB_PAGE            0x1000

#define USER_PHYS_MEM_PAGE_ADDR(process_id) (EIGHT_MB + (FOUR_MB * process_id))

/* page table entry */
typedef union pte_entry {
    uint32_t val;
    struct {
        uint32_t present            : 1;
        uint32_t read_write         : 1;
        uint32_t user_super         : 1;
        uint32_t write_through      : 1;
        uint32_t cache_disable      : 1;
        uint32_t accessed           : 1;
        uint32_t dirty              : 1;
        uint32_t pg_attr_tble       : 1;
        uint32_t global             : 1;
        uint32_t avl                : 3;
        uint32_t addr               : 20;
    } __attribute__ ((packed));
} pte_entry;

/* page directory entry */
typedef union pde_entry {
    uint32_t val;
    struct {
        uint32_t present          : 1;
        uint32_t read_write       : 1;
        uint32_t user_super       : 1;
        uint32_t write_through    : 1;
        uint32_t cache_disable    : 1;
        uint32_t accessed         : 1;
        uint32_t avl_bit          : 1;
        uint32_t page_size        : 1;
        uint32_t avl              : 4;
        uint32_t addr             : 20;
    } __attribute__ ((packed));
} pde_entry;

/* page directory table */
pde_entry page_directory_table[PDE_TABLE_SIZE] __attribute__((aligned(PAGING_ALIGNMENT)));
/* page entry table*/
pte_entry first_page_table[PTE_TABLE_SIZE] __attribute__((aligned(PAGING_ALIGNMENT)));
pte_entry user_vid_table[PTE_TABLE_SIZE] __attribute__((aligned(PAGING_ALIGNMENT)));

/* used in other files*/
extern void paging_init();
extern void set_user_page(int32_t new_pid);
extern void init_pde_entry(pde_entry* table, int idx, int status, int priv, int rw, int size, void* addr);
extern void init_pte_entry(pte_entry* table, int idx, int status, int priv, int rw, void* addr);
extern void init_empty_pdt(pde_entry* table);
extern void init_empty_ptt(pte_entry* table);

#endif
