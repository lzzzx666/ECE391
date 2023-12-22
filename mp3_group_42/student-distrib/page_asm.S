.global	get_cr, set_cr
.global	callee_push, callee_pop, caller_push, caller_pop
.global cr0,cr2,cr3,cr4

/** define the global variable to store thr control registers*/
cr0:
	.long 0x0
cr2:
	.long 0x0
cr3:
	.long 0x0
cr4:
	.long 0x0
	

/**	void set_cr()
*  	set the control registers to the values given in the corresponding cr0, cr2, cr3, cr4
*   input: none
*	output: none
*	side effects: none
*	note: must set cr0 after finishing setting other control registers
*/
set_cr:
	call callee_push	#set up stack frame
	movl cr2,%eax		#set cr2
	movl %eax,%cr2
	movl cr3,%eax		#set cr3
	movl %eax,%cr3
	movl cr4,%eax		#set cr4
	movl %eax,%cr4
	movl cr0,%eax		#set cr0
	movl %eax,%cr0
	call callee_pop  	#tear down stack frame
	ret

/**	get set_cr()
*  	store the control registers values to the the corresponding global variables cr0, cr2, cr3, cr4
*   input: none
*	output: none
*	side effects: none
*/
get_cr:
	call callee_push	#set up stack frame
	movl %cr0,%eax		#get cr0
	movl %eax,cr0
	movl %cr2,%eax		#get cr2
	movl %eax,cr2
	movl %cr3,%eax		#get cr3
	movl %eax,cr3
	movl %cr4,%eax		#get cr4
	movl %eax,cr4
	call callee_pop		#tear down stack frame
	ret

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

