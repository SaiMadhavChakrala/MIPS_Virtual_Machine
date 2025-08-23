.data
  .align 2
__vm_temp:       .space 32
__vm_ptr_this:   .word 0
__vm_ptr_that:   .word 0

.bss
  .align 2
__stack: .space 65536
__stack_top:

.text
.globl __start
__start:
  la   $sp, __stack_top
  jal  main
  li   $v0, 10
  syscall

# push: expects value in $t0
__vm_push:
  addi $sp, $sp, -4
  sw   $t0, 0($sp)
  jr   $ra

# pop: returns value in $t0
__vm_pop:
  lw   $t0, 0($sp)
  addi $sp, $sp, 4
  jr   $ra
