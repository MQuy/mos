#include <stdint.h>
#include <stddef.h>
#include <include/ctype.h>
#include <libc/unistd.h>

#define SERVER_NAME "window_server"

int main()
{
  char buf[12];
  msgopen(SERVER_NAME, NULL);
  switch (fork())
  {
  case 0:
    msgsnd(SERVER_NAME, "Hello world\n", 1, 12);
    break;

  default:
    msgrcv(SERVER_NAME, buf, 1, 12);
    break;
  }
  return 0;
}