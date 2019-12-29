#include <stdint.h>
#include <include/ctype.h>
#include <libc/unistd.h>

int main()
{
  int32_t fildes[2];
  const int BSIZE = 100;
  char buf[BSIZE];

  pipe(fildes);

  switch (fork())
  {
  case -1: /* Handle error */
    break;

  case 0:                        /* Child - reads from pipe */
    close(fildes[1]);            /* Write end is unused */
    read(fildes[0], buf, BSIZE); /* Get data from pipe */
    /* At this point, a further read would see end-of-file ... */
    close(fildes[0]); /* Finished with pipe */
    break;

  default:                                 /* Parent - writes to pipe */
    close(fildes[0]);                      /* Read end is unused */
    write(fildes[1], "Hello world\n", 12); /* Write data on pipe */
    close(fildes[1]);                      /* Child will see EOF */
    break;
  }
  return 0;
}