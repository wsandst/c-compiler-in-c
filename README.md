[![Build Status](https://travis-ci.com/wsandst/c-compiler-in-c.svg?branch=main)](https://travis-ci.com/wsandst/c-compiler-in-c)
# ccic - a C Compiler written in C
 **ccic** is a self-hosting/bootstrapping C compiler, written completely in C. The compiler uses no dependencies to simplify bootstrapping. It generates Linux x86-64 executables.
The compiler uses a handwritten tokenizer, a recursive decent parser and a code generator which generates NASM assembly. GCC is then used as a linker. It includes headers for commonly used parts of the C standard library, which have been modified from GCC to accommodate for fewer macro features. These headers can be found under `clib/`

## Supported constructs
* Variables
    * Globals
    * Locals
    * Scopes
* Expressions
    * Binary operations
    * Unary operations (post and pre)
    * Parenthesis
    * Operator precedence, associativity
    * Logical short circuiting
    * Most floating point operations
    * Pointer operations
    * Struct operations
* Functions
    * Integer function arguments
    * Float function arguments
    * Struct arguments by value
        * No support for struct unpacking, always sent as a pointer
            and then dereffed
    * Return values (integer/float/struct)
    * Variadic functions
        * Full variadic function calling support (ex printf)
        * Limited support for variadic function definitions
            * Missing support for va_arg macro, can only call va_begin and va_end
            * No support for floating point or struct arguments
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
* Types
    * Integers
    * Floats (only 64 bit)
    * Pointers
    * Arrays
    * Type casting (explicit and implicit)
    * Typedefs
    * Enums
    * Structs
* Preprocessor
    * Includes (normal and standard library includes)
    * Pragma once
    * Simple defines, undef
    * Ifdef, ifndef
    * Missing support for macro functions
* Gotos, labels

## Build instructions
### Compiler
Compile: `make`  
Run: `./build/ccic <source>`  
The compiler expects a `clib/` folder in the working folder
### Tests
Run tests: `make test`  
Extensive valgrind tests: `make test-full`  
Run tests with a bootstrapped compiler: `make bootstrap-test`  
Triangle bootstrapping test: `make bootstrap-triangle-test`

## Dependencies
`nasm` - Assembler for the generated Intel-syntax assembly  
`gcc` - Linker  
`valgrind` - Memory leak checker used for the testing

## Testing
The project contains an extensive testing suite, which 
includes both unit tests and compilation tests. These 
compilation tests compile example programs, found under `test/compilation/`, with both GCC and this compiler. The return values are then compared to make sure the compiler behaves identical to GCC. These example programs have been constructed to test various C language constructs. All of the tests are also run with valgrind to make sure there are no memory leaks. The bootstrapping tests compile the compiler using itself and then performs the tests with this self-hosted compiler.