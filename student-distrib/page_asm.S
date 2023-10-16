.global set_cr0,set_cr2,set_cr3,set_cr4,set_cr
.global	get_cr0,get_cr2,get_cr3,get_cr4
.global	callee_push, callee_pop, caller_push, caller_pop


/** all of the set_cr and get_cr should not set up stack frame
*	since calling from c code would pass the param by eax*/
set_cr:
	movl %eax,%cr3
	movl %cr4,%eax
	orl $0x00000010,%eax
	movl %eax,%cr4
	movl %cr0,%eax
	orl $0x80000000,%eax
	movl %eax,%cr0
	ret


# set_cr0:
# 	pushl %ecx
# 	movl %cr0,%ecx
# 	orl %eax,%ecx
#     movl %ecx,%cr0
# 	popl %ecx
#     ret

# set_cr2:
#     movl %eax,%cr2
#     ret

# set_cr3:  
#     movl %eax,%cr3
#     ret

# set_cr4:
# 	pushl %ecx
# 	movl %cr4,%ecx
# 	orl %eax,%ecx
#     movl %ecx,%cr4
# 	popl %ecx
#     ret

# get_cr0:
# 	movl %cr0,%eax
# 	ret

# get_cr2:
# 	movl %cr2,%eax
# 	ret

# get_cr3:
# 	movl %cr3,%eax
# 	ret

# get_cr4:
# 	movl %cr4,%eax
# 	ret



/*helper functions for set up the stack frame*/
# void caller_push(void)
# push ecx 
# input: none
# output: none
caller_push:
		pushl %ecx
		xchgl %ecx,4(%esp)
		xchgl %ecx,(%esp)
		xchgl %ecx,4(%esp)
		ret

# void caller_pop(void)
# pop ecx 
# input: none
# output: none
caller_pop:
		xchgl %ecx,4(%esp)
		xchgl %ecx,(%esp)
		xchgl %ecx,4(%esp)
		popl %ecx
		ret

# void callee_push(void)
# push ebx esi edi ebp
# input: none
# output: none
callee_push:
		pushl %ebx
		pushl %esi
		pushl %edi
		xchgl 12(%esp),%ebp
		pushl %ebp
		leal 16(%esp),%ebp
		ret

# void callee_pop(void)
# pop ebx esi edi ebp
# input: none
# output: none
callee_pop:
		movl (%esp),%ebx
		leal -12(%ebp),%esp
		movl (%ebp),%ebp # pop ebp
		popl %edi
		popl %esi
		movl %ebx,4(%esp)
		popl %ebx
		ret

