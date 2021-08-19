// Enums

enum TEST_ENUM { A, B, C, D, E };

typedef enum {
    YAY,
    ANON,
    ENUM,
} SUPER_COOL_ENUM;

int main() {
    enum TEST_ENUM t1 = B; // Type here
    enum TEST_ENUM2 { COOL1, COOL2, COOL3 } t2 = COOL2;
    SUPER_COOL_ENUM t3 = YAY;
    return t1 + t2 + t3;
}

// The difference between a type qualifier and a definition is
// enum NAME IDENT vs enum NAME { }
// Any place where I might expect a type, there can be struct/enum as well
// These can contain their own initialization! Important
// int; is a totally fine statement.
// I shouldn't complain if there is no identifier, needs to be a ; though