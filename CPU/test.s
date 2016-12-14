addiu $s0, $0, 1
addiu $s1, $0, 2
addu $s1, $s1, $s0
and $s1, $s1, $s0
jal change_ra
and $s1, $s1, $s2
or $s1, $s1, $s2
j dummy

change_ra:
  add $s2, $s0, $s1
  jr $ra

dummy:
  add $t0, $t0, $0

