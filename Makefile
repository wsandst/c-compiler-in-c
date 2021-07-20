SRC_DIR := src
TEST_DIR := test
OBJ_DIR := build/obj
TEST_OBJ_DIR := build/test_obj
BIN_DIR := build

EXE := $(BIN_DIR)/ccompiler
SRC := $(wildcard $(SRC_DIR)/*.c)
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

.PHONY: all clean testexe test

# Compile program
all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR):
	mkdir -p $@

testexe: clean $(TEST_EXE)

$(TEST_EXE): $(TEST_OBJ) $(OBJ_NO_MAIN) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -I./src $^ $(LDLIBS) -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(TEST_OBJ_DIR)
	$(CC) $(CPPFLAGS) -I./src $(CFLAGS) -c $< -o $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR)

# Run unit tests and valgrind tests
test: testexe
	@echo Running tests...
	./build/ccompiler-test
	@echo Running memory leak test...
	valgrind --leak-check=full --error-exitcode=1 --log-fd=2 ./build/ccompiler-test 1>/dev/null
	@echo "\e[1;32mPassed tests!\e[0m"
	
-include $(OBJ:.o=.d)