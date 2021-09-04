// Global arrays of arrays

static char* rax_modifier_strs[4] = { "al", "ax", "eax", "rax" };
static char* rbx_modifier_strs[4] = { "bl", "bx", "ebx", "rbx" };
static char* rcx_modifier_strs[4] = { "cl", "cx", "ecx", "rcx" };
static char* rdx_modifier_strs[4] = { "dl", "dx", "edx", "rdx" };
static char* rsi_modifier_strs[4] = { "sil", "si", "esi", "rsi" };
static char* rdi_modifier_strs[4] = { "dil", "di", "edi", "rdi" };
static char* r8_modifier_strs[4] = { "r8b", "r8w", "r8d", "r8" };
static char* r9_modifier_strs[4] = { "r9b", "r9w", "r9d", "r9" };

static char** register_enum_to_modifier_strs[14] = {
    rax_modifier_strs, rbx_modifier_strs, rcx_modifier_strs,
    rdx_modifier_strs, rsi_modifier_strs, rdi_modifier_strs,
    r8_modifier_strs,  r9_modifier_strs,  0
};

int main() {
    return register_enum_to_modifier_strs[0][6][1] == '8';
}