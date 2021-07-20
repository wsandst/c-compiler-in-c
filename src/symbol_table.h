// Mapping variable names to the symbol map.
// IDENT -> Variable (which contains the offsets and such)
// So, register_variable(IDENT, something to identify the unique scope)
// then, get_variable(IDENT, something to identify the unique scope)
// This needs to be a hashtable

// Each scope has its own symbol table. Then every scope has an inherent stack offset 
// To find a variable, first search the current scope and traverse all the way to the top
// The symbol table is a tree of scopes. Every time a new scope is made, I make a new child
// Global scope -> function scope -> block scope etc

// This is a tree of tables
struct SymbolTable {
    bool is_global; // Is this the global scope, at the top?
    SymbolTable* children;
    int children_count;
    SymbolTable* parent;
    //Variable* vars;
    //Function* funcs;
};