# This test script compiles various examples in test_code in both gcc and ccompiler
# and then compares the exit codes. If the exit codes do not match, the test fails
# Credit to Nora Sandler for the original version of this script 
use_valgrind=false
silence_valgrind=false

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CLEAR='\033[0m'
failed_test=false

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -f|--full) use_valgrind=true ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

echo "[TEST] Running automated compilation tests..."
for i in test/compilation/*/*.c
#for i in test/test_code/programs/ex2.c
do
    gcc -w -g $i     #compile with gcc
    ./a.out       #run it
    expected=$?   #get exit code
    #compile with ccompiler, optionally use valgrind to check for mem issues
    if [ "$use_valgrind" = true ] ; then
        echo -e "${YELLOW}Running compiler with valgrind, expect slow compilation!${CLEAR}"
        if [ "$silence_valgrind" = true ] ; then
            valgrind --leak-check=full --error-exitcode=1 --log-fd=2 2>/dev/null ./build/ccompiler $i
        else
            valgrind --leak-check=full --error-exitcode=1 ./build/ccompiler $i
        fi
        if [ $? -ne 0 ]; then
            echo -e "${RED}VALGRIND FAIL ${CLEAR}"
            failed_test=true
        fi
    else
        ./build/ccompiler $i
    fi
    ./output    # Run the binary we assembled
    actual=$?   # get exit code from binary
    echo -n "[TEST] $i:    "
    # Give error if exit code does not match gcc
    if [ "$expected" -ne "$actual" ] ; then
        echo -e "${RED}FAIL: expected ${expected}, got ${actual}${CLEAR}"
        failed_test=true
    else
        #echo "Returned: ${actual}"
        echo -e "${GREEN}OK${CLEAR}"
    fi
    rm output
    rm output.asm
done

#cleanup
rm a.out

if [ "$failed_test" = true ] ; then
    echo -e "[TEST] ${RED}Compilation test failed. ${CLEAR}"
    exit 1
fi
echo -e "[TEST] ${GREEN}All compilation tests passed!${CLEAR}"