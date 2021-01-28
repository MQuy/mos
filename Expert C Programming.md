### Chapter 2: It's Not a bug, It's a Language Feature

- The details of a language make the different between a reliable and an error-prone one.
  - In Summer 1961, incorrect precision in orbital trajectory calculator program at NASA happened due to
    Fortran's feature. Blank characters are not significant and can even occur in the middle of an identifier (to help cardpunch walloppers and readability of programs)
  ```fortran
  DO 10 I=1.10
  ->
  DO10I = 1.10
  ```

#### SINS OF COMMISSION

- switch's `default` can appear anywhere in the list cases and any form of statements are permitted
  ```c
  switch(i) {
    case 5 + 3: do_again:
    case 2: printf("I loop unremittingly\n"); goto do_again;
    defau1t: i++; // typo
  }
  ```
- default fall through on switches is a design defect in C
- adjacent string literals are concatenated into one which leads to one potential issue
  ```c
  char names[] = {
    "luffy",
    "zoro"      // no comma!
    "sanji",
    "nami",
  }
  ```
- too much default visiblity

#### SINS OF MISSION

- many symbols are "overloadded" - given different meanings when used in different contexts
  for example `void` in return type, no function parameter and a generic pointer
- `sizeof` is the operator _not_ a function call
  ```c
  sizeof(int) // use for type, has to be enclosed in parentheses
  sizeof * p  // p is int *
              // use for variable, not require
  ```
- some of the operators have the wrong precedence like ==/!= higher than bitwise.
  Long story short, early C has no separate operators for & and &&, & is intepreted as && when boolean is expected. Later, && was introduced and Dennis was afraid to change the precedence due to backwards compatibility. In retrospect, he said it would be better to just change it

#### SINS OF OMISSION

- if there is more than one possibility for the next token, the compiler will prefer the longest sequence of characters
  ```c
  z = y+++x;
  ->
  z = y++ + x;
  ```

### Chapter 3: Unscambling Declarations in C

- C philosophy that the declaration of an object should look like its use
  ```c
  int *p[3];
  *p[i] // usage
  ---
  char (*j)[20];
  j = (char (*)[20])malloc(20); // have to keep redundant parentheses around the asterisk
  ```
- function arugments might not be pushed into stack, they can be in registers for speed when possible
- when assigning struct, struct elements are treated as first-class
  ```c
  struct s { int a[100]; }
  ```
- different between `typedef` and `#define`
  - you can extend a macro typename with other type specifiers, but not typedef
  ```c
  #define peach int
  unsigned peach i; // works
  ---
  typedef int banana
  unsigned banana i; // no
  ```
  - typedef provides the type for every declarator in a declaration
  ```c
  #define int_ptr int *
  int_ptr chalk, cheese; // -> int * chalk, cheese; <- only chalk is the int pointer
  ---
  typedef char * char_ptr
  char_ptr x, y // both x, y are char pointers
  ```
- there are multiple namespaces in C (everything within a namespace must be unique)
  - label names
  - tags (one namespace for all structs, enums and unions)
  - member names (each struct or union has its own namespace)
  - everything else

### Chap 4: The Shocking Truth: C Arrays and Pointers Are Not the Same!

- definition occurs in only one place while declaration ocurrs multiple times
- the main different between pointer and array is address vs content of address (it is much clear when looking at assembly version of c)
  ```c
  char a[] = "hello"; a[i];
  1. a doesn't exist, when refer to a, it is replace with the first element address, say 0x1000
  2. get content from address (0x1000 + i)
  -----
  char *a = "hello"; a[i];
  1. a is memory address, in x86 it is 4-byte, say 0x1000
  2. get content from address 0x1000, say 0x5000
  3. get content from address (0x5000 + i)
  ```
  so when the scenario like below
  ```c
  # file1.c
  char a[] = "hello";
  -------
  # file2.c
  extern char *a;
  doing a[0] ->
  1. content of address a, in x86, it is "hell" (0x6C6C6568)
  2. get content of 0x6C6C6568 -> might corrupt your program
  ```
- a pointer definition does not allocate space for what is pointed at, only for pointer
  ```c
  char *p = "hello"; // work
  int *i = 10; // work, in this case 10 is memory address
  float *f = 3.14; // doesn't work, since 3.14 is value
  ```

### Chap 5: Thinking of Linking

- Benefits of dynamic linking
  - is smaller than its sl counterpart (avoid coping library into executable)
  - when linking to a particular library share a single copy of the library at runtime
  - permits easy versioning of libraries, new libraries can be shipped -> old program can get benefit without be relinked
  - allows users to select at runtime which library to execute against (for example one for speed, one for memory efficiency or containing debugging info)
- five special secrets of linking with libraries
  - dynamic libraries are called "lib*something.so*", static libraries are called "lib*something.a*"
  - you tell the compiler to link with, for example, "libthread.so" by giving the option -lthread
  - the compiler expects to find the libraries in certain directories (for example _-Lpathname -Rpathname_)
  - identify your libraries by looking at the headers files you have used (sometimes, you have to use tools like _nm_ to manually search for a needed symbol)
  - symbol from static libraries are extracted when needed (looking for _undefined_ symbols) by linker, while all library symbols go to the virtual address space for dynamic libraries
    ✍️ in static linking, if there is no undefined, so nothing will be extracted -> you have to put like this `gcc main.c -lm`
- interposing is the practice of supplanting a library function by user-written function of the same name, usually for debugging or performance reasons

### Chap 9: More about Arrays

- array of type parameters are coverted to pointer of type by the compiler, other cases, they are as they are defined (while pointers are always pointers)
  ```c
  my_function(int *a) {}
  my_function(int a[]) {}
  my_function(int a[100]) {}
  are the same
  ```
  the reason for c to treat array parameters as pointers is efficiency (you don't want to copy array when passing to a function). Other data arguments are passed by value except arrays and functions
- an array reference `a[i]` is always rewritten to `*(a + 1)` by the compiler
  ```c
  a[6] == 6[a] // true <- *(a + 6) == *(6 + a)
  ```
- array names are not modifiable l-values
  ```c
  int p[] = {1, 2};
  p = 0; // doesn't work
  int *c;
  c =  0; // work
  ----
  void demo(int a[]) {
    a = 0; // work <- compiler converts `int a[]` to `int *a`
  }
  ```
- multidimentional array is a single block of memory while array of array, each of which can be of different lengths and occupy their own memory block
  C only supports array of array
  ```c
  int carrot[10][20]; // carrot is a 10-element array, each element is 20-int array
  carrot[i][j] == *(*(carrot + i) + j) // true
  carrot + i == (char *)carrot + i * 20 * 4 // carrot == int (*)[20]
  ```

### More About Pointers

- Iliffe vector is a data structure used to implement n-dimensional arrays in one-dimensional array
  ```c
  int *box[10];
  ```
- when looking at `squash[i][j]`, you cannot tell whether it is declared as
  ```c
  int squash[10][20];
  vs
  int (*squash)[20];
  vs
  int **squash;
  vs
  int *squash[20];
  ```
- array name is written as a pointer argument isn't recursive
  ```c
  char c[8][10] -> char (*c)[10]
  char *c[15] -> char **c
  char (*c)[10] -> char (*c)[10]
  char **c -> char **c
  ```
- no way to pass general multidimensional array to a function, you could either use one-dimension array (convert two into one using `arr[row_size * i + j]`) or rewrite the matrix to Iliffe vector
  ```c
  void do_something(int a[][3][5]) {}
  ---
  int a[100][3][5]; do_something(a); // work
  int b[2][3][5]; do_something(b); // work
  ---
  int c[5][3][3]; do_something(c); // not compile
  int d[2][4][5]; do_something(d); // not compile
  ```

### Appendix: Secrets of Programmer Job Interviewer

- library calls are part of the language or application, and system calls are part of the operating system

### Notes

- `long double` is 80-bit extended precision on x86 processors -> occupy 96 bits
  ```c
    long double a = 3.14, b = a; // sizeof(a) == 16UL
    a == b; // true
    memcmp(&a, &b, sizeof(a)); // false because of uninitialized padding bytes
  ```
- [why `calloc` exists?](https://vorpus.org/blog/why-does-calloc-exist/)
  - `calloc` checks for overflow and errors out if the multiplication cannot fit into 32-bit or 64-bit integer (depend on how os/kernel is implemented)
  ```c
  malloc(INTPTR_MAX * INTPTR_MAX); // work
  calloc(INTPTR_MAX, INTPTR_MAX); // error
  ```
  - when a operating system hands out memory to a process (depend on how os/kernel is implemented), it always zeros it out first (for security reason).
    - for large buffer, it probably comes from os -> `calloc` cheats by skipping zeroing out.
    - for small buffer, `calloc` == `malloc` + `memset`
  - when handing 1GB of memory using, kernel probably does the trick that only mapping/zeroing out the first block 4KB and mark the rest as copy-on-write. Later when writing that rest, kernel actualy does the job. with `malloc` + `memset`, we do the mapping/zeroing out upfront while `calloc` we could do it later
- some compilers permit multiple characters in a character constant, the actual value is implementation-defined
  ```c
  char str[] = 'yes'; // valid
  ```
- [The Clockwise/Spiral Rule to parse C declaration](http://c-faq.com/decl/spiral.anderson.html)
- only the four operators `&&`, `||`, `?:` and `,` specify an order of evaluation, others evaluate their operands in undefined order
- better to declare a variable as `unsigned` when we expect it to non-negative then depending our implementation-defined like right-shift, division ...
- [a definition is the special kind of declaration](https://en.cppreference.com/w/c/language/declarations)
  - every declaration of an `enum` or `typedef` is a definition
  - for function, declaration that includes body is a function definition
  - for objects, declaration that allocates storage (not `extern`) is a definition
  ```c
  extern int n; // declaration
  int n; // declaration
  int n = 10; // definition
  ```
  - for structs and unions, declaration that specify list of members is a definition
- [C rules and recommendation](https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard)
- adjacent strings are concatenated one by one
  ```c
  char x[] = "hello" " world"; // x == "hello world"
  char y[] = "\x12" "3"; // y == "\0223", not "\x123"
                         // "\x12" "3" are two characters while "\x123" is one multibyte character
  ```
-
