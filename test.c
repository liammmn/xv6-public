#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "[USER] calling write\n");
  write(1, "hello from trace_test\n", 22);
  exit();
}