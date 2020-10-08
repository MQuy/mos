### Definition

Both file descriptor and stream represent a connection to a file. Stream provides higher-level interface on top of file descriptor.

in C, stream is called `FILE` which contains position indicator, buffer, error, end-of-file status ... There are 3 standard streams for each process named `stdin`, `stdout` and `stderr` (correspond to file descriptor 0, 1 and 2)

#### Buffer

There are 3 kinds of buffering strategies:

- `unbuffered`: are transitted individually as soon as possible
- `line buffered`: are transmited when a newline character is encountered
- `fully buffered`: are transmited in blocks of arbitrary size, which is usually used for newly opened streams except for terminal

#### Design

```c++
struct FILE {
  int fd;
  int flags;
  int offset;
  char *read_ptr, *read_base, *read_end;
  char *write_ptr, *write_base, *write_end;
  char *save_ptr; // for ungetc
  int blksize;
};

int fopen(const char *pathname, const char *mode){
  1. if mode contains
    | `r` -> flags = O_RDONLY, offset = begin of file
    | `r+` -> flags = O_RDWR, offset = begin of file
    | `w` -> flags = O_WRONLY, offset = begin of file and truncate or create
    | `w+` -> same as `w` except flags = O_RDWR
    | `a` -> flags = O_WRONLY, offset = end of file and file is created if not exist
    | `a+` -> same as `a` except flags = O_RDWR
  2. `fd = open(pathname, flags)` and `lseek(fd, offset, SEEK_CUR)`
  3. `fstat(fd)` -> `blksize` is block size and `flags` is seekable or not depend on file's mode
}

int fgetc(FILE *stream) {
  1. if end of file -> return EOF
  2. if stream is
    | non-seekable
      - `read(fd, ch, 1)`
    | seekable
      | `read_base == read_end`
        - from `offset`, find boundary aligned by `blksize`
        - allocate and assign to `read_base/read_end`
        - `read(fd, read_base, read_end - read_base)`
      | otherwise
        - return `*read_ptr`
}

```
