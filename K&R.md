## The C Programming Language (Brian Kernighan & Dennis Ritchie)

### Introduction

- is used to develop UNIX system and is inspired by BCPL/B (both are typeless)
- offer straightforward, single-thread control flow: tests loops, grouping and subrograms but not multiprogramming, parallel operations, synchronization or coroutines
- is not a strongly-typed language

### Chap 1: A Tutorial Introduction

- every program must have a main somewhere
- there is no input/output defined in C (from ANSI standard)
- the isolated semicolon is called a null statement
- in history (the original definition of C), a function could be written like
  ```c
  power(base, n)
  int base, n;
  {}
  ```
- an external variable must be defined, exactly once, outside of any function. To access, it needs to be declared beforehand either in a function or in a file.

### Chap 2: Types, Operators, and Expressions

- variable names are case-sensitive and made up of letters (includes understore) and digits (the first character must be a letter)
- `short` and `int` are at least 16 bits, `long` is at least 32 bits and `short` is no longer than `int` which is no longer than `long`
- signed types obeys two's complement machine
- an integer constant's type is `int` (by default). If an integer is too big, its type is `long`. An floating-point constant's type is double
  ```c
  1 -> int
  1L -> long
  1U -> unsigned
  1F -> float
  ```
- the value of a character constant is the numeric value of the character in the machine's character set (usually ASCII)
  ```c
  '\ooo' -> octal
  '\xhh' -> hexadecimal
  ```
- a constant expression only contains constants <- may be evaluated during compilation instead of runtime
  ```c
  int x = 1 + 2
  char str[] = "hello," " world"
  ```
- the first name in enum has value 0 and so on, unless explicit values are specified. Unspecified values continue from the last specified value. Comparing to `#define`
  - enum values might be checked which doesn't in `#define`
  - debugger may be able print values
  - values can be generated for you
- `%` operator cannot be applied to float/double
- the direction of truncate for / and sign of the result for `%` are machine-dependent for negative operands
- when an operator has operands of different types, the lower type is promoted to the higher type (except for `float` to save computing time)
- comparison between signed and unsigned values are machine-dependent because they depend on the sizes of the various integer types
- rounding or truncating when converting double to float is implementation-dependent
- `char` values (except printable character) can be signed/unsigned depend on machine -> when converting `char` to `int`, it might be negative
- right shifting a signed quantity will fill with sign bits (arithmetic shift) or with 0 bits (logical shift) is machine-dependent
- `expr1 op= expr2` is equivalent to `expr1 = exrp1 op expr2` except that `expr1` is computed only once
- `expr1 ? expr2 : expr3` -> type of exrp1 is deteminated by the higher type between `expr2` and `expr3` regarding the result of `expr1` is true or false
- doesn't specify the order in operands of an operator are evaluated. The same for function arguments, nested assignments, and increment/decrement, cause side effects, can lead to different results

  ```c
  x = f() + g() <- f may be evaluated before g or vice versa
  a[i] = i++
  ```

### Chapter 3: Control Flow

- all case expressions must be different.
- put `break` after the last case even though it's logically unnecessary (good for added cases at the end later)
- the commas, separate function arguments, variables in declarations ..., are not comma operations, and do not guarantee left to right evaluation

### Chapter 4: Funtions and Program Structure

- if the return type is omitted, `int` is assumed
- lacking of function prototype, a function is implicitly declared by its first appearance in an expression
- `register` variables are to be placed in machine registers (compiler can ignore the advice) and is only applied to automatic variables/function parameters
- a `static` variable is initialized only the first time encountering
- in absence of explicit initialization, external and static variables are guaranted to be zero
- if fewer initializers for an array, missing elements will be zero for external, static and automatic variables
- `#include "filename"` searchs for the file where the source program is found; if not found there, will be looking like `#include <filename>` which follows an implementation-defined rule
- `#define dprint(epxr) #expr`, with actual argument, each `"` is replaced by `\"` and each `\` by `\\`

### Chapter 5: Pointers and Arrays

- `&` operator only applies to objects (variables and array elements) in memory, not for expressions, constants, or register variables
- a pointer is a variable while array name is not
- `char *msg = "now is the time"` <- undefined behaviour if modifying the string contents

### Chapter 6: Structures

- only legal operations on a structure are copying/assigning as a unit, taking its address (`&`), and accessing its members
- `sizeof` cannot be used in `#if` because preprocessor which doesn't parse type names. But the expression in `#define` is not evaluated by preprocessor, so it is legal
- don't assume the size of structure is the sum of sizes of its members due to alignment for different objects
- it is illegal for a structure to contain an instance of itself (pointer of itself, is not an issue)
- `typedef` doesn't create a new type (new name for existing type), like `#define` except it is interpreted by the compiler
- a union may only be initialized with a value of the type of its first member
- bit fields in structure don't have address, so `&` operator cannot be applied and left-right or right-left order depends on machine (little or big endian)
  ```c
  struct {
    unsigned int is_static : 1;
  }
  ```

### Appendix A: Reference Manual

- there are two types of storage classes: automatic and static
  - automatic objects are local to block, and are discarded on exit from the block (`register` variables are automatic)
  - static objects appear local to block but retain their values across exit and reentry (objects declared outside of all blocks are always static)
- `lvalue` is an expression referring to an object which is named region of storage
- when converting
  - a integer to given unsigned type, in two's complement representation
    - narrower unsigned type: left-truncate bits
    - wider unsigned type: zero-filling unsigned values and sign-extending signed values
  - a integer to signed type, the value is unchanged if it fits, otherwise, implementation-defined
  - negative floating values to unsigned integral types is not specified
- removing qualifiers, when converting pointer of the same type, doesn't change operation on the underlying object
- the term "actual argument" is used for an expression passed by a function call, "formal argument" for input object described in function declaration
