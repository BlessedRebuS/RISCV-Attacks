#include <stdlib.h>
#include <stdio.h>
#include <string.h>



void not_called() {

    printf("exit function\n");
    asm volatile ("li a6, 0");
    asm volatile ("mv a0, a6");
    asm volatile ("li a7, 93");
    asm volatile ("ecall");

    return;

}

void gadgets() {

    asm volatile ("ld a0, 8(sp)");
    asm volatile ("ld ra, 16(sp)");
    asm volatile ("ld a1, 24(sp)");
    asm volatile ("ret");
    return;
}

int test_empty() {

    printf("Empty function\n");

    return 1;

}

void vulnerable_function(char* string) {

    char buffer[64];

    test_empty();

    strcpy(buffer, string);

}



int main(int argc, char** argv) {

    vulnerable_function(argv[1]);

    return 0;

}
