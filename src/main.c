#include <stdio.h>
#include "tokens.h"


int main() {
    /*char* str = "  This is source code \n \
        #hello!    \n \
        // Comment ! \n"
        "\" This is a string! \"";*/

    char* str = "\"This \\\"is a string\" abc \"another one\" mdd 'this \" is a char tho'";

    Tokens tokens = tokenize(str);
    
    return 0;
}