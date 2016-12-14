addiu $s0, $s0, 2
addiu $s1, $s1, 1
addu $s2, $s1, $0
beq $s1, $s2, one
addiu $s0, $s0, 2
addiu $s0, $s0, 2

one: 
  addiu $s2, $s1, 1
  bne $s2, $s1, two
  addiu $s2, $s2, 2
  addiu $s2, $s2, 2

two:
  addiu $s2, $s2, 20000	
  addiu $s1, $s1, 20000
  bov $s2, $s1, three
  addiu $s2, $s2, 2
  addiu $s2, $s2, 2

three: 
  beq $s1, $s2, four
  add $s1, $0, $0
  add $s2, $0, $0
  bne $s1, $s2, four
  bov $s1, $s2, four
  addiu $s0, $s0, 2
  addiu $s0, $s0, 2
  j dummy

four: 
  addiu $s0, $s0, 1
  
dummy: 
  add $t0, $t0, $0

