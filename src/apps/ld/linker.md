### Object files

There are three types of object files

- Relocatable object file: contains binary code, data and other sections (like .sym, .rel.text rel.data) that can be combined with others at compile time to create executable object file.
- Executable object file: contains binary code, data and other sections that can be copied directly into memory and executed.
- Shared object file: special type of relocatable object file that can be loaded into memory and linked dynamically (runtime or loadtime).

### Static Linking

Static linker takes a collection of relocatable object files and generates a fully linked executable object file via two steps

- Symbol resolution: associate each symbol reference with exactly one symbol definition
  - If referencing to local symbols in the same module -> straightforward
  - If referencing to global symbols (define in other modules) -> scan all modules
- Relocation: consists of two steps
  - Merge all sections with the same type into a new one and assign runtime address for each section/symbol
  - Correct symbol references with new runtime addresses. Basically, whenever the assembler encounters a reference which is global or undefined, it generates relocation entry to tell linker how to modify it later

> Note: when resolving references using static libraries, linker only copies relocatable object files whose symbols are used

### Dynamic Linking
