.text
.global execute_cleanup, flush_tlb
.align 4

// flush_tlb
// desc -- tlb is the cache used by the mmu of the cpu, we need to flush it everytime we change paging
// input -- none
// output -- none
// side effects -- flush the tlb
// cr3 register is the Page Directory Base Register (PDBR),
flush_tlb:
    movl %cr3, %eax
    movl %eax, %cr3
    ret

// execute_cleanup
// desc -- the execute clean up prepare us for the context switch
// input -- ebp and eip
// output -- none
// side effects -- context switch to the user
execute_cleanup:
    pushl   %ebp
    movl    %esp, %ebp
    // ebp in ecx
    movl    8(%ebp), %ecx
    // eip in edx
    movl   12(%ebp), %edx

    // push SS
    // 0x002B is USER_DS
    pushl $0x002B

    // push ESP
    pushl %ecx

    // push EFLAGS
    pushfl
    pop %ecx
    // turn the IF bit, bit 9 to high
    orl $0x200, %ecx
    pushl %ecx

    // push CS
    // 0x0023 is USER_DS
    pushl $0x0023

    // push EIP
    pushl %edx

    // sti

    iret
