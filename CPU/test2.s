addiu $sp, $sp, -8
sw $ra, 0($sp) 
sw $s0, 4($sp)
addiu $s0, $s0, 1
lw $ra, 0($sp)
lw $s0, 4($sp)
addiu $sp, $sp, 8
jr $ra
