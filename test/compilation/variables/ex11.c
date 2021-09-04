// Global arrays of arrays

char* rax_modifier_strs[4] = { "al", "ax", "eax", "rax" };
char* rbx_modifier_strs[4] = { "bl", "bx", "ebx", "rbx" };
char* rcx_modifier_strs[4] = { "cl", "cx", "ecx", "rcx" };
char* rdx_modifier_strs[4] = { "dl", "dx", "edx", "rdx" };
char* rsi_modifier_strs[4] = { "sil", "si", "esi", "rsi" };
char* rdi_modifier_strs[4] = { "dil", "di", "edi", "rdi" };
char* r8_modifier_strs[4] = { "r8b", "r8w", "r8d", "r8" };
char* r9_modifier_strs[4] = { "r9b", "r9w", "r9d", "r9" };

char** register_enum_to_modifier_strs[14] = {
    rax_modifier_strs, rbx_modifier_strs, rcx_modifier_strs,
    rdx_modifier_strs, rsi_modifier_strs, rdi_modifier_strs,
    r8_modifier_strs,  r9_modifier_strs,  0
};

int main() {
    return register_enum_to_modifier_strs[0][6][1] == '8';
}