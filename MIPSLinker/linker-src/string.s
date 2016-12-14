# CS 61C Summer 2016 Project 2-2 
# string.s

#==============================================================================
#                              Project 2-2 Part 1
#                               String README
#==============================================================================
# In this file you will be implementing some utilities for manipulating strings.
# The functions you need to implement are:
#  - strlen()
#  - strncpy()
#  - copy_of_str()
# Test cases are in linker-tests/test_string.s
#==============================================================================

.data
newline:	.asciiz "\n"
tab:	.asciiz "\t"

.text
#------------------------------------------------------------------------------
# function strlen()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string input
#
# Returns: the length of the string
#------------------------------------------------------------------------------
strlen: 
	# YOUR CODE HERE
	add $v0, $0, $0 #initialize counter
	beq $0, $a0, end
loop: 
	lb $t0, 0($a0) #load first char
	beq $t0, $0, end #while (char != '\0')
	addi $a0, $a0, 1 #increment string pointer by 1
	addi $v0, $v0, 1 #increment counter by 1
	j loop #back to loop	
end:
	jr $ra

#------------------------------------------------------------------------------
# function strncpy()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = pointer to destination array
#  $a1 = source string
#  $a2 = number of characters to copy
#
# Returns: the destination array
#------------------------------------------------------------------------------
strncpy:
	# YOUR CODE HERE
	beq $a1, $0, end2 #source string is empty
	add $t1, $0, $0 #counter
loop2:
	beq $a2, $0, end2 #if number of chars to copy is 0
	add $t2, $t1, $a1 #t2 has ptr to src at t1th elem
	lb $t3, 0($t2) #load first elem of t2 into t3
	add $t4, $t1, $a0 #t4 has ptr to t1th elem
	sb $t3, 0($t4) #store first elem of t2 (t3) into t4
	addi $t1, $t1, 1 #increment ptr to src string
	addi $a2, $a2, -1 #decrement number of chars to copy
	j loop2
end2: 
	move $v0, $a0
	jr $ra

#------------------------------------------------------------------------------
# function copy_of_str()
#------------------------------------------------------------------------------
# Creates a copy of a string. You will need to use sbrk (syscall 9) to allocate
# space for the string. strlen() and strncpy() will be helpful for this function.
# In MARS, to malloc memory use the sbrk syscall (syscall 9). See help for details.
#
# Arguments:
#   $a0 = string to copy
#
# Returns: pointer to the copy of the string
#------------------------------------------------------------------------------
copy_of_str:
	# YOUR CODE HERE
	addi $sp, $sp, -12 #prologue
	sw $ra, 0($sp)
	sw $a0, 4($sp) 
	sw $s0, 8($sp)
	jal strlen #call on strlen w a0. v0 now has length of string
	add $a0, $v0, $0 #store strlen into a0 for syscall.
	add $s0, $v0, $0 #store strlen into s0 for later call on strncpy 
	addi $v0, $0, 9
	syscall #v0 now has malloc-ed space
	add $a0, $v0, $0 #a0 now has malloc-ed space
	lw $a1, 4($sp) #a1 has the string to copy (original a0)
	add $a2, $s0, $0 #a2 has strlen
	jal strncpy #call on strncpt w a0-a2. v0 now has copy of str.
	lw $ra, 0($sp) #epilogue
	lw $a0, 4($sp) 
	lw $s0, 8($sp)
	addi $sp, $sp, 12
	jr $ra

###############################################################################
#                 DO NOT MODIFY ANYTHING BELOW THIS POINT                       
###############################################################################

#------------------------------------------------------------------------------
# function streq() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string 1
#  $a1 = string 2
#
# Returns: 0 if string 1 and string 2 are equal, -1 if they are not equal
#------------------------------------------------------------------------------
streq:
	beq $a0, $0, streq_false	# Begin streq()
	beq $a1, $0, streq_false
streq_loop:
	lb $t0, 0($a0)
	lb $t1, 0($a1)
	addiu $a0, $a0, 1
	addiu $a1, $a1, 1
	bne $t0, $t1, streq_false
	beq $t0, $0, streq_true
	j streq_loop
streq_true:
	li $v0, 0
	jr $ra
streq_false:
	li $v0, -1
	jr $ra			# End streq()

#------------------------------------------------------------------------------
# function dec_to_str() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Convert a number to its unsigned decimal integer string representation, eg.
# 35 => "35", 1024 => "1024". 
#
# Arguments:
#  $a0 = int to write
#  $a1 = character buffer to write into
#
# Returns: the number of digits written
#------------------------------------------------------------------------------
dec_to_str:
	li $t0, 10			# Begin dec_to_str()
	li $v0, 0
dec_to_str_largest_divisor:
	div $a0, $t0
	mflo $t1		# Quotient
	beq $t1, $0, dec_to_str_next
	mul $t0, $t0, 10
	j dec_to_str_largest_divisor
dec_to_str_next:
	mfhi $t2		# Remainder
dec_to_str_write:
	div $t0, $t0, 10	# Largest divisible amount
	div $t2, $t0
	mflo $t3		# extract digit to write
	addiu $t3, $t3, 48	# convert num -> ASCII
	sb $t3, 0($a1)
	addiu $a1, $a1, 1
	addiu $v0, $v0, 1
	mfhi $t2		# setup for next round
	bne $t2, $0, dec_to_str_write
	jr $ra			# End dec_to_str()
