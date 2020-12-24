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
