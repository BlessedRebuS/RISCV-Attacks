#include <stdio.h>

void not_called(){
    asm volatile ("li s2, 1");
    return;
}

void hello(){
    printf("hello\n");
    return;
}

int main(){
    register long s2 asm ("s2");
    asm volatile ("jal ra, not_called");
    printf("Val of res: %ld\n", s2);
    return 0;
}