SRC_DIR := src
BIN_DIR := build
TEST_DIR := test/unit
OBJ_DIR := build/obj
TEST_OBJ_DIR := build/test_obj
LIBC_DIR := build/libc
OBJ_DIR_BS := build/obj_bs
TEST_OBJ_DIR_BS := build/test_obj_bs

EXE := $(BIN_DIR)/ccic
SRC := $(wildcard $(SRC_DIR)/*.c)
SRC += $(wildcard $(SRC_DIR)/util/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
SRC_NO_MAIN := $(filter-out $(SRC_DIR)/compiler.c, $(SRC))
OBJ_NO_MAIN := $(SRC_NO_MAIN:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
TEST_EXE := $(BIN_DIR)/ccic-test
TEST_OBJ := $(TEST_SRC:$(TEST_DIR)/%.c=$(TEST_OBJ_DIR)/%.o)

# Bootstrapping related
OBJ_BS := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR_BS)/%.o)
EXE_BS := $(BIN_DIR)/ccic-bs
# Compiling unit tests related
TEST_EXE_BS := $(BIN_DIR)/ccic-bs-test
TEST_OBJ_BS := $(TEST_SRC:$(TEST_DIR)/%.c=$(TEST_OBJ_DIR_BS)/%.o)
OBJ_NO_MAIN_BS := $(SRC_NO_MAIN:$(SRC_DIR)/%.c=$(OBJ_DIR_BS)/%.o)

CFLAGS   := -Wall -g -O0
LDFLAGS  := -Llib
LDLIBS   := -lm -Isrc
LD := $(CC)

.PHONY: all clean testexe test unit-test test-full test-full-mt bootstrap bootstrap-testexe bootstrap-unit-test bootstrap-test

# ============== Normal Compilation ===================

# Compile program
all: $(EXE) $(LIBC_DIR)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR) $(OBJ_DIR_BS) $(TEST_OBJ_DIR_BS):
	mkdir -p $@
	mkdir -p $@/util

$(LIBC_DIR):
	ln -sr libc $@

# =================== Tests =======================

# Compile unit test executable
testexe: clean $(TEST_EXE) $(LIBC_DIR)

$(TEST_EXE): $(TEST_OBJ) $(OBJ_NO_MAIN) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(TEST_OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR) $(LIBC_DIR) $(OBJ_DIR_BS) $(TEST_OBJ_DIR_BS)

unit-test: testexe
	@echo [TEST] Running unit tests...
	./build/ccic-test
	@echo "[TEST] \e[0;32mPassed unit tests!\e[0m"
	@echo Running memory leak test...
	valgrind --leak-check=full --error-exitcode=1 --log-fd=2 ./build/ccic-test 1>/dev/null
	@echo "[TEST] \e[0;32mPassed memory leak test!\e[0m"

# Run unit tests with valgrind and compilation tests without valgrind
test: unit-test all
	bash ./test/compilation/test_compilation.sh
	
# Run full tests. This includes running valgrind on every compile, which
# is very slow
test-full: unit-test all
	bash ./test/compilation/test_compilation.sh --full

# Multithreaded version, has less output
test-full-mt: unit-test all
	bash ./test/compilation/test_compilation.sh --full --multithreading

# ============== Bootstrapping compilation test ==============

bootstrap: $(EXE) $(EXE_BS)

$(EXE_BS): $(OBJ_BS) | $(BIN_DIR)
	gcc -g -no-pie $^ -o $@

$(OBJ_DIR_BS)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR_BS)
	build/ccic -c $< -o $@

force:
	touch src/compiler.c
	touch test/unit/test.c

# Compile unit test executable
bootstrap-testexe: clean $(EXE) force $(TEST_EXE_BS) $(LIBC_DIR)

$(TEST_EXE_BS): $(TEST_OBJ_BS) $(OBJ_NO_MAIN_BS) | $(BIN_DIR)
	gcc -g -no-pie $^ -o $@

$(TEST_OBJ_DIR_BS)/%.o: $(TEST_DIR)/%.c | $(TEST_OBJ_DIR_BS)
	build/ccic -c $< -o $@

bootstrap-unit-test: bootstrap-testexe
	@echo [TEST] Running unit tests compiled using CCIC...
	./build/ccic-bs-test
	@echo "[TEST] \e[0;32mPassed unit tests!\e[0m"
	@echo Running memory leak test...
	valgrind --leak-check=full --error-exitcode=1 --log-fd=2 ./build/ccic-bs-test 1>/dev/null
	@echo "[TEST] \e[0;32mPassed memory leak test!\e[0m"

# Run unit tests with valgrind and compilation tests without valgrind
bootstrap-test: bootstrap-unit-test bootstrap 
	mv build/ccic-bs build/ccic
	bash ./test/compilation/test_compilation.sh

-include $(OBJ:.o=.d)