.global _start

.section .text
_start:
	la a0, shell
	li a1, 0
	li a2, 0
	li a7, 221
	ecall 

	li a0, 1
	li a7, 93
	ecall

shell:
 .ascii "/bin/bash"
