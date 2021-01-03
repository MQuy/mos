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
