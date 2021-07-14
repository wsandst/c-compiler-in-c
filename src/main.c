#include <stdio.h>
#include "tokens.h"


int main() {
    char* str = "  This is source code \n   #hello!    \n \
        Interesting! #hmm";

    tokenize(str);
    
    return 0;
}