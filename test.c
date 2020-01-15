#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lc3.h"

int test_add_1(vm_ctx *curr_vm)
{
    int pass = 1;
    uint16_t add_instr =
        ((OP_ADD & 0xf) << 12) |
        ((R_R0 & 0x7) << 9) |
        ((R_R1 & 0x7) << 6) |
        (R_R2 & 0x7);

    curr_vm->memory[0x3000] = add_instr;
    curr_vm->regs[R_R1] = 1;
    curr_vm->regs[R_R2] = 2;

    int result = execute_inst(curr_vm);
    if (result != 1)
    {
        printf("Expected return value to be 1, got %d\n", result);
        pass = 0;
    }

    if (curr_vm->regs[R_R0] != 3)
    {
        printf("Expected register 0 to contain 3, got %d\n", curr_vm->regs[R_R0]);
        pass = 0;
    }

    if (curr_vm->regs[R_COND] != FL_POS)
    {
        printf("Expected condition flags to be %d, got %d\n", FL_POS, curr_vm->regs[R_COND]);
        pass = 0;
    }

    return pass;
}

int test_add_2(vm_ctx *curr_vm)
{
    int pass = 1;

    uint16_t add_instr =
        ((OP_ADD & 0xf) << 12) |
        ((R_R0 & 0x7) << 9) |
        ((R_R1 & 0x7) << 6) |
        (1 << 5) |
        0x2;

    curr_vm->memory[0x3000] = add_instr;
    curr_vm->regs[R_R1] = 1;

    int result = execute_inst(curr_vm);
    if (result != 1)
    {
        printf("Expected return value to be 1, got %d\n", result);
        pass = 0;
    }

    if (curr_vm->regs[R_R0] != 3)
    {
        printf("Expected register 0 to contain 3, got %d\n", curr_vm->regs[R_R0]);
        pass = 0;
    }

    if (curr_vm->regs[R_COND] != FL_POS)
    {
        printf("Expected condition flags to be %d, got %d\n", FL_POS, curr_vm->regs[R_COND]);
        pass = 0;
    }

    return pass;
}

int test_env()
{
    vm_ctx curr_vm;
    curr_vm.regs = malloc(sizeof(uint16_t) * R_COUNT);
    curr_vm.memory = malloc(UINT16_MAX * sizeof(uint16_t));
    int (*test_case[])(vm_ctx * curr_vm) = {test_add_1, test_add_2};

    int result = 1;
    int case_num = sizeof(test_case) / sizeof(uint16_t *);
    for (int i = 0; i < case_num; i++)
    {
        curr_vm.regs[R_PC] = 0x3000;
        memset(curr_vm.memory, 0, UINT16_MAX);
        memset(curr_vm.regs, 0, R_COUNT);
        result = test_case[i](&curr_vm);
        if (result == 0)
        {
            printf("Test %d fail!\n", i);
            free(curr_vm.memory);
            free(curr_vm.regs);
            return result;
        }

        else if (result == 1)
        {
            printf("Test %d passed!\n", i);
        }
    }
    free(curr_vm.memory);
    free(curr_vm.regs);
    return result;
}

int main()
{
    int result = test_env();
    if (result)
    {
        printf("All test passed!\n");
    }
    else
    {
        printf("Test failed!\n");
    }
    return 0;
}