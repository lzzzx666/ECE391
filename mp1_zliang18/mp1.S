
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs
total_exp:			.long 0x0 # used to check whether should notify user (when returned explosion >0)
.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
	pushl %ebp
	movl %esp, %ebp
	pushl %eax	
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	pushfl
	movl $0, total_exp # let total explode be zero
	call update_missiles
	call redraw_cities
	call redraw_crosshairs
	popfl
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	leave
	ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention


jumptable:
	.long	mp1_ioctl_startgame
	.long	mp1_ioctl_addmissile
	.long	mp1_ioctl_movexhairs
	.long	mp1_ioctl_getstatus
	.long	mp1_ioctl_endgame
	# jmp table for ioctl

.globl mp1_ioctl
mp1_ioctl:
	pushl %ebp
	movl %esp, %ebp	
	movl 12(%ebp), %ebx
	cmpl $0, %ebx	# test if cmd is valid
	jl error
	cmpl $4, %ebx # cmd must between 0-4
	jg error	
	jmp *jumptable(, %ebx, 4) # set edx pointing to target function in jmp table

error:
	movl $-1, %eax	# return -1 if cmd is not 0-4 

justret:	
	leave
	ret


# ----------------- Functions private to this file -------------------

# void update_missile()
# update the missile by testing it is moving out of range or exploding or just 
# flying normally
# Inputs : no
# Outputs  : no
# Registers: Standard C calling convention

update_missiles:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx	
	pushl %esi
	pushl %edi	
	movl mp1_missile_list, %esi # let esi = this pointer

list_traverse:
	cmpl $0, %esi		
	je	traverse_done	
	movl Y(%esi), %eax		
	movl X(%esi), %ebx 	
	pushl %eax #store x,y
	pushl %ebx
	shrl $16, %eax # get high 16 bits of x,y
	shrl $16, %ebx
	pushl $' '    
	pushl %eax  #restore x,y
	pushl %ebx  
	call draw_pixel #draw space at original place
	addl $12, %esp
	popl %ebx
	popl %eax

	cmpl $0, EXPLODED(%esi)
	jne	exploding
	movl VY(%esi), %ecx		
	movl VX(%esi), %edx		
	addl %ecx, %eax	 # get new x, y position
	addl %edx, %ebx		
	cmpl $0x00000000, %eax	
	jge	y1left		
	movl $0x00000000, %eax 
	jmp outofrange
y1left:
	cmpl $0x0018FFFF, %eax # test if 0=<y<=24 and 0=<x<=79
	jle	y1right		
	movl $0x0018FFFF, %eax # 0x18 = 24
	jmp outofrange 
y1right:
	cmpl $0x00000000, %ebx
	jge	x1left		
	movl $0x00000000, %ebx
	jmp outofrange  # out of range case
x1left:
	cmpl $0x004FFFFF, %ebx  # 0x4F = 79
	jle	 inrange		#past all test means in range
	movl $0x004FFFFF, %ebx
	jmp  outofrange

outofrange:
	pushl NEXT(%esi)
	pushl %esi
	call mp1_missile_remove		
	addl $4, %esp
	popl %esi			 
	jmp	 list_traverse
inrange:			
	movl DEST_Y(%esi), %ecx	 # test if missile reach destination
	movl %eax, %edx		
	shrl $16, %edx		# get high 16 bits
	cmpl %ecx, %edx
	jne	 normal
	movl DEST_X(%esi), %ecx	
	movl %ebx, %edx		
	shrl $16, %edx		
	cmpl %ecx, %edx
	jne	normal # normal case

exploding:
	pushl %eax	#store eax
	pushl %esi
	call missile_explode # explode(*this)
	popl %esi
	addl %eax, total_exp # sum the return value
	popl %eax  #restore eax
	decl EXPLODED(%esi)
	cmpl $0, EXPLODED(%esi)
	jne	still_exp		
	pushl NEXT(%esi) # if explode = 0, remove this missile
	pushl %esi 
	call mp1_missile_remove		
	addl $4, %esp
	popl %esi
	jmp	 list_traverse

still_exp:
	pushl $' '
	pushl DEST_Y(%esi) #y
	pushl DEST_X(%esi)#x
	call draw_pixel
	addl $12, %esp
	pushl %eax
	pushl %ebx
	shrl $16, %eax #y get high 16 bits and draw it
	shrl $16, %ebx #x
	pushl $EXPLOSION
	pushl %eax
	pushl %ebx
	call draw_pixel		# draw explostion
	addl $12, %esp
	popl %ebx
	popl %eax
	jmp	proceed_next

normal:			
	movl %eax, Y(%esi)
	movl %ebx, X(%esi)		
	shrl $16, %eax
	shrl $16, %ebx
	pushl C(%esi) # draw character if normal
	pushl %eax
	pushl %ebx			
	call draw_pixel	
	addl $12, %esp

proceed_next:
	movl NEXT(%esi), %esi

	# movl NEXT(%edi), %esi #update this and prev pointer
	jmp	list_traverse

traverse_done:
	cmpl $0, total_exp
	je nothing_done		
	call mp1_notify_user # it total_exp>0, should notify user

nothing_done:
	popl %edi
	popl %esi
	popl %ebx	
	leave
	ret



# void mp1_missile_remove(struct missile* prev, struct missile* this)
# remove input pointer from the list and screen and also free its memory
# Input: pointer to missile that will be removed
# Output: pointer to the next missile
# Registers: Standard C calling convention

mp1_missile_remove:
	pushl %ebp
	movl %esp, %ebp
	pushl %eax	
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	movl 8(%ebp), %edx	 #this
	movl mp1_missile_list, %edi	#
	cmpl %edx, %edi
	je	movehead

removeloop:		
	movl %edi, %esi 	#store prev pointer
	movl NEXT(%edi), %edi
	cmpl %edx, %edi
	je should_remove
	jmp removeloop


movehead:
	movl NEXT(%edx), %esi # prev->next = this->next  
	movl %esi, mp1_missile_list	
	jmp removefree

should_remove:
	movl NEXT(%edx), %edi
	movl %edi, NEXT(%esi)

removefree:
	pushl %edx
	call mp1_free	# free the missile
	addl $4, %esp
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax		
	leave
	ret



# void draw_pixel(x,y,char)
# draw a char at given position
# input: x,y and char
# output: no
# Registers: Standard C calling convention

draw_pixel:
	pushl %ebp
	movl %esp, %ebp
	pushl %eax # callee save
	pushl %ebx
	pushl %edi
	pushl %esi
	pushl %ecx

	movl 8(%ebp), %ebx # x
	movl 12(%ebp), %edi # y
	movl 16(%ebp), %ecx # char
	imull $160, %edi, %esi # 80 * 2 * y
	movl %esi, %edi
	imull $2, %ebx, %esi # 2 * x
	addl %esi, %edi # 160 y + 2x
	movl %edi, %eax

	pushl %edx # save edx since it may be clobbed
	call mp1_poke
	popl %edx

	popl %ecx
	popl %esi
	popl %edi
	popl %ebx
	popl %eax
	
	leave
	ret


# int mp1 ioctl startgame (unsigned long ignore);
# initialize all of the variables used by the driver
# Inputs   :  no
# Outputs  : -1 if cmd is invalid
# Registers: use jmp table

mp1_ioctl_startgame:
	movl $0, mp1_missile_list
	movl $0x00FFFFFF, base_alive	
	movl $0, mp1_score
	movl $40, crosshairs_x
	movl $12, crosshairs_y
	leave 
	ret

mp1_ioctl_addmissile:
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %ebx
	pushl $36 # malloc total 36 bytes for a missile
	call mp1_malloc
	addl $4, %esp
	cmpl $0, %eax	# check if eax == nullptr
	je mallocfail
	movl 8(%ebp), %ebx # user's address(from)
	movl %eax, %esi #copy kernel address 
	pushl $36 # copy total 36 bytes
	pushl %ebx	
	pushl %eax	# push kernel's address(to)
	call mp1_copy_from_user
	addl $12, %esp
	cmpl $0, %eax
	jne copyfail
	movl mp1_missile_list, %eax # update list head
	movl %eax, NEXT(%esi)
	movl %esi, mp1_missile_list 
	movl $0, %eax 	#return 0
	jmp notfail

copyfail:
	# free the kernel's address
	pushl %esi
	call mp1_free
	addl $4, %esp

mallocfail:
	movl $-1, %eax # return -1 if fail

notfail:
	popl %ebx	
	popl %esi
	popl %edx
	popl %ecx
	leave
	ret

# int mp1 ioctl movexhairs (unsigned long xhair delta packed)
# moves the crosshairs
# input: the position vector f crosshairs
# output: no
# Registers: use jmp table 

mp1_ioctl_movexhairs:
	# SHL (left), SAR (arithmetic right), and SHR (logical right)
	pushl %ebx
	pushl %edi
	pushl %esi
	pushl %ecx
	pushl %edx
	movl 8(%ebp), %ebx
	andl $0x0000FFFF, %ebx # x offset
	shll $16, %ebx
	sarl $16, %ebx # get x offset as a signed-number
	movl crosshairs_x, %edi
	addl %ebx, %edi

	movl 8(%ebp), %ebx # y offset
	andl $0xFFFF0000, %ebx
	sarl $16, %ebx # get y offset as a signed-number
	movl crosshairs_y, %esi
	addl %ebx, %esi

xleft:	# x>=0
	cmpl $0, %edi
	jge xright
	xorl %edi, %edi
	
xright:	# x<=79
	cmpl $79, %edi
	jle yleft
	movl $79, %edi

yleft:  # y>=0
	cmpl $0, %esi
	jge yright
	xorl %esi, %esi

yright:	#y<=24
	cmpl $24, %esi
	jle test_original
	movl $24, %esi
#edi = x, esi = y

test_original:	
	cmpl crosshairs_x, %edi # test if its position changed
	jne draw_xhairs
	cmpl crosshairs_y, %esi
	jne draw_xhairs
	jmp noneed

draw_xhairs:
	pushl $' '
	pushl crosshairs_y # delete old one 
	pushl crosshairs_x
	call draw_pixel
	addl $12, %esp
 
	pushl $'+'    # draw new one
	pushl %esi
	pushl %edi
	call draw_pixel
	addl $12, %esp

noneed:	
	movl %edi, crosshairs_x # update x and y position
	movl %esi, crosshairs_y
	popl %ecx
	popl %ecx
	popl %esi
	popl %edi
	popl %ebx
	xorl %eax, %eax
	leave
	ret

# int mp1 ioctl getstatus (unsigned long* user status);
# retrieve the current score and the status of the three cities
# input: a pointer to a integer in user space
# output : -1 if copy failed else 0
# Registers: use jmp table

mp1_ioctl_getstatus:
	pushl %ebx
	pushl %edi
	pushl %esi
	pushl %ecx
	movl 8(%ebp), %edi	
	movl mp1_score, %ebx		
	andl $0x0000FFFF, %ebx	 # lower 16 bits <- mp1_score
	movl base_alive, %edx
	andl $0x000000FF, %edx	# 16th bit <- base1
	cmpl $0, %edx
	je bit1_done
	orl	$0x00010000, %ebx	

bit1_done:
	movl base_alive, %edx
	andl $0x0000FF00, %edx # 17th bit <- base2
	cmpl $0, %edx
	je bit2_done
	orl	$0x00020000, %ebx	

bit2_done:
	movl base_alive, %edx
	andl $0x00FF0000, %edx # 18th bit <- base3
	cmpl $0, %edx
	je bit3_done
	orl $0x00040000, %ebx	

bit3_done:
	movl mp1_score, %ecx	# store mp1_score
	movl %ebx, mp1_score
	leal mp1_score, %esi	
	pushl %edx
	pushl %ecx	
	pushl $4 # copy a 32-bit value
	pushl %esi
	pushl %edi
	call mp1_copy_to_user	
	addl $12, %esp
	popl %ecx	
	popl %edx		
	movl %ecx, mp1_score # restore mp1_score
	cmpl $0, %eax
	je exit
	movl $-1, %eax # return -1 if copy failed

exit:
	popl %ecx
	popl %edi
	popl %esi
	popl %ebx			
	leave
	ret

# int mp1 ioctl endgame (unsigned long ignore)
# perform all the cleanup work
# input: no 
# output : no
# Registers: use jmp table

mp1_ioctl_endgame:
	pushl %ebx
	pushl %esi # callee saved
	
loop:
	movl mp1_missile_list, %ebx
	cmpl $0, %ebx
	je	listdone
	movl NEXT(%ebx), %esi
	movl %esi, mp1_missile_list
	pushl %eax
	pushl %ecx		
	pushl %edx
	pushl %ebx
	call mp1_free  # free every node
	addl $4, %esp
	popl %edx
	popl %ecx		
	popl %eax
	jmp loop

listdone:	
	popl %esi
	popl %ebx
	xorl %eax, %eax
	leave
	ret

# void redraw_cities()
# redrawing the cities to ensure that any destroyed cities are drawn as destroyed
# Inputs : no
# Outputs  : no
# Registers: Standard C calling convention

redraw_cities:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %edi
	pushl %esi
	pushl %ecx
	movl base_alive, %ebx

city1:	
	movl %ebx, %edi
	andl $0x000000FF, %edi
	cmpl $0, %edi # check if base1 alive
	je basenotalive1
	leal base_pic, %edi
	jmp startdraw1
basenotalive1:
	leal dead_base_pic, %edi
startdraw1:
	movl $18, %ecx # first city x-> 18-22
	movl $0, %esi
drawloop1:
	pushl (%edi, %esi) # draw M[edi + esi]
	pushl $24 #y
	pushl %ecx #x
	call draw_pixel
	addl $12, %esp 
	addl $1, %esi
	addl $1, %ecx
	cmpl $5, %esi # draw total 5 pixels
	jl drawloop1

city2:
	movl %ebx, %edi
	andl $0x0000FF00, %edi
	cmpl $0, %edi  # check if base2 alive
	je basenotalive2
	leal base_pic, %edi
	jmp startdraw2
basenotalive2:
	leal dead_base_pic, %edi
startdraw2:
	movl $38, %ecx # second city ->38-42
	movl $0, %esi
drawloop2:
	pushl (%edi, %esi) # draw M[edi + esi]
	pushl $24 #y
	pushl %ecx #x
	call draw_pixel
	addl $12, %esp
	addl $1, %esi
	addl $1, %ecx
	cmpl $5, %esi # draw total 5 pixels
	jl drawloop2

city3:
	movl %ebx, %edi
	andl $0x00FF0000, %edi
	cmpl $0, %edi # check if base3 alive
	je basenotalive3
	leal base_pic, %edi
	jmp startdraw3
basenotalive3:
	leal dead_base_pic, %edi
startdraw3:
	movl $58, %ecx # third city x->58-62
	movl $0, %esi
drawloop3:
	pushl (%edi, %esi) # draw M[edi + esi]
	pushl $24
	pushl %ecx
	call draw_pixel
	addl $12, %esp
	addl $1, %esi
	addl $1, %ecx
	cmpl $5, %esi # draw total 5 pixels
	jl drawloop3

	popl %ecx
	popl %esi
	popl %edi
	popl %ebx
	leave
	ret

# void redraw_crosshairs()
# redraw the crosshairs
# Inputs : no
# Outputs  : no
# Registers: Standard C calling convention

redraw_crosshairs:
	pushl %ebp
	movl %esp, %ebp
	pushl $'+'
	pushl crosshairs_y 
	pushl crosshairs_x
	call draw_pixel
	addl $12, %esp
	leave
	ret



