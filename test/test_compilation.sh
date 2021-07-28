# This test script compiles various examples in test_code in both gcc and ccompiler
# and then compares the exit codes. If the exit codes do not match, the test fails
# Credit to Nora Sandler for most of this script 
RED='\033[0;31m'
GREEN='\033[0;32m'
CLEAR='\033[0m'
echo "[TEST] Running automated compilation tests..."
failed_test=false
for i in test/test_code/*/*.c
do
    gcc -w $i               #compile with gcc
    ./a.out                 #run it
    expected=$?             #get exit code
    ./build/ccompiler $i               #compile with ccompiler
    #base="${i%.*}"
    #$base                   #run the thing we assembled
    ./output
    actual=$?                #get exit code
    echo -n "[TEST] $i:    "
    if [ "$expected" -ne "$actual" ]
    then
        echo "${RED}FAIL: expected ${expected}, got ${actual}${CLEAR}"
        failed_test=true
    else
        #echo "Returned: ${actual}"
        echo "${GREEN}OK${CLEAR}"
    fi
    rm output
    rm output.asm
done

#cleanup
rm a.out

if [ "$failed_test" = true ] ; then
    echo "[TEST] ${RED}Compilation test failed. ${CLEAR}"
    exit 1
fi
echo "[TEST] ${GREEN}All compilation tests passed!${CLEAR}"
