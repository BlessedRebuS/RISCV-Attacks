#include <stdlib.h>

# use PATH injection using export PATH=$PWD:$PATH and replace ls with a malicious file

int main( void )
{
    system("ls");

    return 0;
}