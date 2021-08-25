# This test script compiles various examples in test/compilation in both gcc and ccompiler
# and then compares the exit codes. If the exit codes do not match, the test fails
use_valgrind=false
silence_valgrind=false
use_multithreading=false

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CLEAR='\033[0m'
failed_test=false

# flag options:
#   -full: runs valgrind on every compilator run
#   -mt: runs the compilation tests in parallel
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -f|--full) use_valgrind=true ;;
        -mt|--multithreading) use_multithreading=true ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

function run_compilation_test {
    gcc -w -g $1     #compile with gcc
    ./a.out       #run it
    expected=$?   #get exit code
    #compile with ccompiler, optionally use valgrind to check for mem issues
    if [ "$use_valgrind" = true ] ; then
        echo -e "${YELLOW}Running compiler with valgrind, expect slow compilation!${CLEAR}"
        if [ "$silence_valgrind" = true ] ; then
            valgrind --leak-check=full --error-exitcode=1 --log-fd=2 2>/dev/null ./build/ccompiler $1
        else
            valgrind --leak-check=full --error-exitcode=1 ./build/ccompiler $1
        fi
        if [ $? -ne 0 ]; then
            echo -e "${RED}VALGRIND FAIL ${CLEAR}"
            failed_test=true
            exit 1
        fi
    else
        ./build/ccompiler $1 output$2
    fi
    ./output$2    # Run the binary we assembled
    actual=$?   # get exit code from binary
    echo -n "[TEST] $1:    "
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
    rm a.out
}

# Multi-threaded variant, uses less printing
function run_compilation_test_mt {
    gcc -w -g -o gcc$2.out $1     #compile with gcc
    ./gcc$2.out > /dev/null      #run it
    expected=$?   #get exit code
    #compile with ccompiler, optionally use valgrind to check for mem issues
    if [ "$use_valgrind" = true ] ; then
        valgrind --leak-check=full --error-exitcode=1 --log-fd=2 2>/dev/null ./build/ccompiler $1 output$2 > /dev/null
        if [ $? -ne 0 ]; then
            echo -e "[TEST] $1: ${RED}VALGRIND FAIL ${CLEAR}"
            failed_test=true
            exit 1
        fi
    else
        ./build/ccompiler $1 output$2 > /dev/null
    fi
    ./output$2 > /dev/null   # Run the binary we assembled
    actual=$?   # get exit code from binary
    # Give error if exit code does not match gcc
    if [ "$expected" -ne "$actual" ] ; then
        echo -e "[TEST] $1: ${RED}FAIL: expected ${expected}, got ${actual}${CLEAR}"
        failed_test=true
    else
        echo -e "[TEST] $1: ${GREEN}OK${CLEAR}"
    fi
    rm output$2
    rm output$2.asm
    rm gcc$2.out
}

echo "[TEST] Running automated compilation tests..."

if [ "$use_multithreading" = true ] ; then
    # Multithreaded variant
    c=0
    for i in test/compilation/*/*.c
    do
        ((c++))
        run_compilation_test_mt $i $c &
    done
else 
    # Normal variant 
    for i in test/compilation/*/*.c
    do
        run_compilation_test $i
    done
fi

# Wait until all subprocesses are complete
wait

if [ "$failed_test" = true ] ; then
    echo -e "[TEST] ${RED}Compilation test failed. ${CLEAR}"
    exit 1
fi
echo -e "[TEST] ${GREEN}All compilation tests passed!${CLEAR}"
