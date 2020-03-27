#include <libc/unistd.h>
#include <libc/string.h>
#include <libc/stdlib.h>

static const char defaultdir[] = "/dev/shm/";

int32_t shm_open(const char *name, int32_t flags, int32_t mode)
{
  char *fname;
  if (name[0] == '/')
    fname = name;
  else
  {
    fname = malloc(strlen(name) + sizeof(defaultdir));
    strcpy(fname, defaultdir);
    strcat(fname, name);
  }
  return open(fname, flags, mode);
}