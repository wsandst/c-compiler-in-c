#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../src/tokens.h"
#include "../../src/preprocess.h"
#include "../../src/util/file_helpers.h"
#include "../../src/util/string_helpers.h"
#include "../../src/symbol_table.h"
#include "../../src/parser.h"
#include "../../src/codegen.h"

void test_codegen();
void test_codegen_helpers();

void test_codegen() {
    printf("[CTEST] Running codegen tests...\n");
    test_codegen_helpers();
    printf("[CTEST] Passed codegen tests!\n");
}

void test_codegen_helpers() {
    AsmContext ctx;
    char* indent_str = str_copy("");
    ctx.asm_indent_str = &indent_str;
    StrVector asm_text_src = str_vec_new(4);
    ctx.asm_text_src = &asm_text_src;
    StrVector asm_data_src = str_vec_new(4);
    ctx.asm_data_src = &asm_data_src;
    ctx.indent_level = 0;
    asm_set_indent(&ctx, 0);

    char* str = "x";
    asm_addf(&ctx, "test %d %s", 5, str);
    char* joined_str = str_vec_join(ctx.asm_text_src);
    assert(strcmp("\ntest 5 x", joined_str) == 0);
    free(joined_str);

    asm_add_sectionf(&ctx, ctx.asm_data_src, "global %s", str);
    joined_str = str_vec_join(ctx.asm_data_src);
    printf("%s\n", joined_str);
    assert(strcmp("\nglobal x", joined_str) == 0);
    free(joined_str);

    free(*ctx.asm_indent_str);
    str_vec_free(ctx.asm_text_src);
    str_vec_free(ctx.asm_data_src);
}