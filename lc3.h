#include <stdint.h>

enum Register
{
    R_R0 = 0, /* General purpose registers */
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6, /* User Stack Pointer */
    R_R7,
    R_PC,   /* Program counter */
    R_COND, /* Condition codes */
    R_COUNT /* Counter */
};

enum Opcode
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump/ret */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

enum Condition
{
    FL_POS = 1 << 0, /* Positive */
    FL_ZRO = 1 << 1, /* Zero */
    FL_NEG = 1 << 2, /* Negative */
};

enum Trap
{
    TRAP_GETC = 0x20,  /* get character from keyboard */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* input a string */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};

enum Memory
{
    MR_KBSR = 0xFE00,  /* keyboard status */
    MR_KBDR = 0xFE02,  /* keyboard data */
    MR_DSR = 0xFE04,   /* display status */
    MV_DDR = 0xFE06,   /* display data */
    MCR = 0xFFFE       /* machine control */
};

typedef struct _vm_ctx
{
    uint16_t *memory;
    uint16_t *regs;
} vm_ctx;

vm_ctx *init_vm(const char *path)
{
    vm_ctx *curr_vm = malloc(sizeof(vm_ctx));
    curr_vm->memory = read_image(path);
    curr_vm->regs = malloc(sizeof(uint16_t) * R_COUNT);
}

void destory_vm(vm_ctx *curr_vm)
{
    free(curr_vm->memory);
    free(curr_vm->regs);
    free(curr_vm);
}

uint16_t *read_image(const char *path); /* reading image from path*/
int execute_inst(vm_ctx *curr_vm); /* execute instruction */