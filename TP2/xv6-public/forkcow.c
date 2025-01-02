#include "types.h"
#include "user.h"
#include "date.h"
#include "forkcow.h"

int stdout = 1;
int stderr = 2;

int
main(int argc, char *argv[])
{
    
    pid = forkcow();

    if(pid < 0) {
          printf(stderr, "ERRO\n");
    }

    exit();
}