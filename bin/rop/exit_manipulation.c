#include <stdio.h>

void not_called(){
	    asm volatile ("li s2, 1");
	        asm volatile ("jal ra, exit_function +22");
		    return;
}

void exit_function(){
	    printf("exit function\n");
	        asm volatile ("li s2, 0");
		    asm volatile ("mv a0, s2");
		        asm volatile ("li a7, 93");
			    asm volatile ("ecall");
			        return;
}

int main(){
	    register long s2 asm ("s2");
	        asm volatile ("jal ra, not_called");
		    printf("Val of res: %ld\n", s2);
		        return 0;
}
