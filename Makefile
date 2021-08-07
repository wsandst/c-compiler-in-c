SRC_DIR := src
TEST_DIR := test/unit
OBJ_DIR := build/obj
TEST_OBJ_DIR := build/test_obj
BIN_DIR := build

EXE := $(BIN_DIR)/ccompiler
SRC := $(wildcard $(SRC_DIR)/*.c)
SRC += $(wildcard $(SRC_DIR)/util/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
SRC_NO_MAIN := $(filter-out $(SRC_DIR)/compiler.c, $(SRC))
OBJ_NO_MAIN := $(SRC_NO_MAIN:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
TEST_EXE := $(BIN_DIR)/ccompiler-test
TEST_OBJ := $(TEST_SRC:$(TEST_DIR)/%.c=$(TEST_OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall -g -O0
RELEASE_CFLAGS := -Wall -O2
LDFLAGS  := -Llib
LDLIBS   := -lm -Isrc

.PHONY: all clean testexe test test-full

# Compile program
all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR):
	mkdir -p $@
	mkdir -p $@/util

testexe: clean $(TEST_EXE)

$(TEST_EXE): $(TEST_OBJ) $(OBJ_NO_MAIN) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -I./src $^ $(LDLIBS) -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(TEST_OBJ_DIR)
	$(CC) $(CPPFLAGS) -I./src $(CFLAGS) -c $< -o $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR)

# Run unit tests with valgrind and compilation tests without valgrind
test: testexe $(EXE)
	@echo [TEST] Running unit tests...
	./build/ccompiler-test
	@echo "[TEST] \e[0;32mPassed unit tests!\e[0m"
	@echo Running memory leak test...
	valgrind --leak-check=full --error-exitcode=1 --log-fd=2 ./build/ccompiler-test 1>/dev/null
	@echo "[TEST] \e[0;32mPassed memory leak test!\e[0m"
	bash ./test/compilation/test_compilation.sh
	
# Run full tests. This includes running valgrind on every compile, which
# is very slow
test-full: testexe $(EXE)
	@echo [TEST] Running unit tests...
	./build/ccompiler-test
	@echo "[TEST] \e[0;32mPassed unit tests!\e[0m"
	@echo Running memory leak test...
	valgrind --leak-check=full --error-exitcode=1 --log-fd=2 ./build/ccompiler-test 1>/dev/null
	@echo "[TEST] \e[0;32mPassed memory leak test!\e[0m"
	bash ./test/compilation/test_compilation.sh --full

-include $(OBJ:.o=.d)