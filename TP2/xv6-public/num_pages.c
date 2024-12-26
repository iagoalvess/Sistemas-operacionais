#include "types.h"
#include "user.h"
#include "num_pages.h"

int stdout = 1;
int stderr = 2;

int
main(int argc, char *argv[])
{
    
    n_pages = num_pages();

    printf(stdout, "%d\n", n_pages);

    exit();
}