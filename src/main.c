#include <stdio.h>
#include "tokens.h"


int main() {
    char* str = "  This is source code \n \
        #hello!    \n \
        // Comment ! \n"
        "/*\n"
        "addawd\n"
        "*/ abc\n";

    tokenize(str);
    
    return 0;
}