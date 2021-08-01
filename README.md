[![Build Status](https://travis-ci.com/wsandst/c-compiler-in-c.svg?branch=main)](https://travis-ci.com/wsandst/c-compiler-in-c)
# Simple C Compiler written in C
This project is a C compiler, written completely in C. The aim of the project is to eventually have a working compiler which can compile itself. For this reason, no external dependencies are used. The compiler generates x86-64 executables.
The compiler uses a handwritten tokenizer, a recursive decent parser and a code generator which generates NASM assembly.

## Supported constructs
* Variables
    * Globals
    * Locals
    * Scopes
    * Currently only supports 64-bit integers
* Expressions
    * All binary operations implemented
    * Unary operations (post and pre)
    * Parenthesis
    * Operator precedence, associativity
* Functions
    * Integer function arguments (up to 6 arguments)
    * Return values (integer)
* Loops
    * While loops
    * Do while loops
    * For loops
    * Breaks, continues
* If statements, else if, else
* Switch
    * Normal cases
    * Default case
    * Breaks
* Gotos, labels


## Build instructions
### Compiler
Compile: `make`  
Run: `./build/ccompiler <source>` 
### Tests
Run tests: `make test`  

## Dependencies
`nasm` - Assembler for the generated Intel-syntax assembly  
`gcc` - Linker  
`valgrind` - Memory leak checker used for the testing

## Testing
The project contains an extensive testing suite, which 
includes both unit tests and compilation tests. These 
compilation tests compile example programs, found under `test/test_code/`, with both GCC and this compiler. The return values are then compared to make sure the compiler behaves identical to GCC. These example programs have been constructed to test various C language constructs. All of the tests are also run with valgrind to make sure there are no memory leaks.