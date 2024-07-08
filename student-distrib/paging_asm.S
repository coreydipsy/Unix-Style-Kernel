.text
.global loadPageDirectory, enablePaging
.align 4


loadPageDirectory:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%ebp), %eax
    movl %eax, %cr3
    leave
    ret

enablePaging:
    pushl %ebp
    movl %esp, %ebp
    movl %cr4, %eax
    orl  $0x00000010, %eax
    movl %eax, %cr4
    movl %cr0, %eax
    orl $0x80000001, %eax
    movl %eax, %cr0
    leave
    ret


// .align 4096
//  label:
// (whatever you want aligned)
