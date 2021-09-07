
enum RegisterEnum {
    RAX,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
};

typedef enum RegisterEnum RegisterEnum;

int main() {
    static RegisterEnum arg_regs1[6] = { RDI, RSI, RDX, RCX, R8, R9 };
    RegisterEnum arg_regs2[6] = { RDI, RSI, RDX, RCX, R8, R9 };
    return arg_regs1[3] + arg_regs2[2];
}