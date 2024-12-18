#include "virt2real.h"
#include "types.h"
#include "user.h"

#define STDOUT 1
#define STDERR 2

char *
convert_address(char *v_address)
{
  char *physical_address = virt2real(v_address);

  printf(STDOUT, "Virtual address: %d\n", v_address);
  printf(STDOUT, "Physical address: %p\n", physical_address);

  return physical_address;
}

int
main(int argc, char *argv[])
{
  if(argc <= 1){
    printf(STDERR, "Use: %s <virtual_address>\n", argv[0]);
    exit();
  }

  convert_address(argv[1]);
  
  exit();
}
