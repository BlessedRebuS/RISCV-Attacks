#include <stdio.h>
#include <stdlib.h>



void not_called(){

	asm ("li a5, 1");

	asm ("jal ra, exit_function +20");

	return;

}



void exit_function(){

	printf("exit function");
	exit(0);

	return;

}



int main(){

	register long s2 asm ("s2");

	asm volatile ("jal ra, not_called");

	printf("Val of res: %ld\n", s2);

	return 0;

}


