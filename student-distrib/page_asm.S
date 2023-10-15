.global init_page_reg

init_page_reg:
    movl  $0, %eax
    movl  %eax, %cr3
    movl  %cr4, %eax
    movl  $0x00000010, %eax
    movl  %eax, %cr4
    movl  %cr0, %eax
    movl  $0x80000000, %eax
    movl  %eax, %cr0
    ret
