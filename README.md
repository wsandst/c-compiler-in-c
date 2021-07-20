# Simple C Compiler written in C
This project is a C compiler, written completely in C. The goal of the project is to 
eventually have a working compiler which can compile the compiler itself. 
For this reason no external dependencies are used. The aim of the project is also to practice the programming language C.

## Build instructions
### Compiler
Compile: `make`  
Run: `./build/ccompiler`  
### Tests
Run tests: `make test`  

## Dependencies
`NASM` is required to compile the generated Intel-syntax Assembly, and `GCC` is used as a linker.