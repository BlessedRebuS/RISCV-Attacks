#include <stdio.h>
#include <unistd.h>


int main(void){
    
    execve("/bin/bash", NULL, NULL);

    return 0;

}

