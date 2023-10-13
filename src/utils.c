#include <stdio.h>

int check(int status, char *message)
{
  if (status < 0)
  {
    perror(message);
    return 0;
  }
  return 1;
}