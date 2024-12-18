#include "types.h"
#include "user.h"
#include "date.h"

int stdout = 1;
int stderr = 2;

int
main(int argc, char *argv[])
{
  struct rtcdate r;

  if (date(&r)) {
    printf(stderr, "ERRO\n");
    exit();
  }

  printf(stdout, "Data: %d-%d-%d\n", r.day, r.month, r.year);

  exit();
}