#define USER_CS     0x0023
#define USER_DS     0x002B
// the execute clean up prepare us for the context switch
extern void execute_cleanup(uint32_t ebp, uint32_t eip);
// we need to flush the tlb everytime we change paging
extern void flush_tlb();
