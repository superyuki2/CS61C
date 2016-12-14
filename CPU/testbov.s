addiu $s0, $s0, 32767
sll $s2, $s0, 17
addiu $s1, $s1, 32767
sll $s1, $s1, 17
bov $s2, $s1, one
addiu $s0, $s0, 2
addiu $s0, $s0, 2

one:
  addiu $s2, $s1, 1
  addiu $s2, $s2, 2
  addiu $s2, $s2, 2


