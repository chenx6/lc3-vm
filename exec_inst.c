#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include "lc3.h"

/* get keyboard status */
uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

/* convert to 16 bit with sign */
uint16_t sign_extend(uint16_t num, int bit_count)
{
    if ((num >> (bit_count - 1)) & 1)
    {
        num |= (0XFFFF << bit_count);
    }
    return num;
}

/* using regs[index] to update flags */
void update_flags(uint16_t index, uint16_t regs[])
{
    if (regs[index] == 0)
    {
        regs[R_COND] = FL_ZRO;
    }
    else if (regs[index] >> 15)
    {
        regs[R_COND] = FL_NEG;
    }
    else
    {
        regs[R_COND] = FL_POS;
    }
}

uint16_t mem_read(uint16_t address, uint16_t *memory)
{
    /* reading the memory mapped keyboard register triggers a key check */
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

void mem_write(uint16_t address, uint16_t val, uint16_t *memory)
{
    memory[address] = val;
}

/* execute trap routine */
int execute_trap(vm_ctx *curr_vm, uint16_t instr, FILE *in, FILE *out)
{
    int running = 1;
    switch (instr & 0xFF)
    {
    case TRAP_GETC:
    {
        uint16_t c = getc(in);
        curr_vm->regs[R_R0] = c;
    }
    break;
    case TRAP_OUT:
    {
        char c = (char)curr_vm->regs[R_R0 & 0xff];
        putc(c, out);
    }
    break;
    case TRAP_PUTS:
    {
        /* one char per word */
        uint16_t *word = curr_vm->memory + curr_vm->regs[R_R0];
        while (*word)
        {
            putc((char)(*word & 0xff), out);
            word++;
        }
        fflush(out);
    }
    break;
    case TRAP_IN:
    {
        fprintf(out, "Enter a character: ");
        fflush(out);

        uint16_t c = getc(in);
        putc((char)c, out);
        fflush(out);

        curr_vm->regs[R_R0] = c;
    }
    break;
    case TRAP_PUTSP:
    {
        /* two chars per word */
        uint16_t *word = (uint16_t *)curr_vm->memory + curr_vm->regs[R_R0];
        while (*word)
        {
            putc((char)(*word & 0xff), out);

            char c = *word >> 8;
            if (c)
            {
                putc(c, out);
            }
            word++;
        }
        fflush(out);
    }
    break;
    case TRAP_HALT:
    {
        fputs("HALT", out);
        fflush(out);
        running = 0;
    }
    break;
    }
    return running;
}

/* execute lc3 instruction */
int execute_inst(vm_ctx *curr_vm)
{
    int running = 1;                 /* program running status */
    int is_max = R_PC == UINT16_MAX; /* is overflow? */

    uint16_t instr = mem_read(curr_vm->regs[R_PC]++, curr_vm->memory); /* instruction */
    uint16_t op = instr >> 12;                                         /* operation code */

    switch (op)
    {
    case OP_ADD:
        /* using {} to declare varible in labels */
        {
            uint16_t dr = (instr >> 9) & 0x7;
            uint16_t sr1 = (instr >> 6) & 0x7;
            uint16_t is_imm = (instr >> 5) & 0x1; /* check the op2 is sr2 or imm */

            if (is_imm)
            {
                uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                curr_vm->regs[dr] = curr_vm->regs[sr1] + imm5;
            }
            else
            {
                uint16_t sr2 = instr & 0x7;
                curr_vm->regs[dr] = curr_vm->regs[sr1] + curr_vm->regs[sr2];
            }
            update_flags(dr, curr_vm->regs);
        }
        break;
    case OP_AND:
    {
        uint16_t dr = (instr >> 9) & 0x7;
        uint16_t sr1 = (instr >> 6) & 0x7;
        uint16_t is_imm = (instr >> 5) & 0x1;

        if (is_imm)
        {
            uint16_t imm5 = sign_extend(instr & 0x1F, 5);
            curr_vm->regs[dr] = curr_vm->regs[sr1] & imm5;
        }
        else
        {
            uint16_t sr2 = instr & 0x7;
            curr_vm->regs[dr] = curr_vm->regs[sr1] & curr_vm->regs[sr2];
        }
        update_flags(dr, curr_vm->regs);
    }
    break;
    case OP_BR:
    {
        uint16_t n_flag = (instr >> 11) & 0x1;
        uint16_t z_flag = (instr >> 10) & 0x1;
        uint16_t p_flag = (instr >> 9) & 0x1;
        uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

        /* advance program counter if one of the following is true:
         * - n_flag is set and negative condition flag is set
         * - z_flag is set and zero condition flag is set
         * - p_flag is set and positive condition flag is set */
        if ((n_flag && (curr_vm->regs[R_COND] & FL_NEG)) ||
            (z_flag && (curr_vm->regs[R_COND] & FL_ZRO)) ||
            (p_flag && (curr_vm->regs[R_COND] & FL_POS)))
        {

            curr_vm->regs[R_PC] += pc_offset;
        }
    }
    break;
    case OP_JMP:
    {
        uint16_t base_r = (instr >> 6) & 0x7;

        curr_vm->regs[R_PC] = curr_vm->regs[base_r];
    }
    break;
    case OP_JSR:
    {
        /* save PC in R7 to jump back to later */
        curr_vm->regs[R_R7] = curr_vm->regs[R_PC];
        uint16_t imm_flag = (instr >> 11) & 0x1;

        if (imm_flag)
        {
            uint16_t pc_offset = sign_extend(instr & 0x7ff, 11);
            curr_vm->regs[R_PC] += pc_offset;
        }
        else
        {
            /* assign contents of base register directly to program counter */
            uint16_t base_r = (instr >> 6) & 0x7;
            curr_vm->regs[R_PC] = curr_vm->regs[base_r];
        }
    }
    break;
    case OP_LD:
    {
        uint16_t dr = (instr >> 9) & 0x7;
        uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);

        /* add pc_offset to the current PC and load that memory location */
        curr_vm->regs[dr] = mem_read(curr_vm->regs[R_PC] + pc_offset, curr_vm->memory);
        update_flags(dr, curr_vm->regs);
    }
    break;
    case OP_LDI:
    {
        uint16_t dr = (instr >> 9) & 0x7;
        uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);

        /* add pc_offset to the current PC, look at that memory location to
         * get the final address */
        curr_vm->regs[dr] = mem_read(mem_read(curr_vm->regs[R_PC] + pc_offset, curr_vm->memory), curr_vm->memory);
        update_flags(dr, curr_vm->regs);
    }
    break;
    case OP_LDR:
    {
        uint16_t dr = (instr >> 9) & 0x7;
        uint16_t base_r = (instr >> 6) & 0x7;
        uint16_t offset = sign_extend(instr & 0x3f, 6);

        curr_vm->regs[dr] = mem_read(curr_vm->regs[base_r] + offset, curr_vm->memory);
        update_flags(dr, curr_vm->regs);
    }
    break;
    case OP_LEA:
    {
        uint16_t dr = (instr >> 9) & 0x7;
        uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);

        curr_vm->regs[dr] = curr_vm->regs[R_PC] + pc_offset;
        update_flags(dr, curr_vm->regs);
    }
    break;
    case OP_ST:
    {
        uint16_t sr = (instr >> 9) & 0x7;
        uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);

        mem_write(curr_vm->regs[R_PC] + pc_offset, curr_vm->regs[sr], curr_vm->memory);
    }
    break;
    case OP_STI:
    {
        uint16_t sr = (instr >> 9) & 0x7;
        uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);

        mem_write(mem_read(curr_vm->regs[R_PC] + pc_offset, curr_vm->memory), curr_vm->regs[sr], curr_vm->memory);
    }
    break;
    case OP_STR:
    {
        uint16_t sr = (instr >> 9) & 0x7;
        uint16_t base_r = (instr >> 6) & 0x7;
        uint16_t offset = sign_extend(instr & 0x3f, 6);

        mem_write(curr_vm->regs[base_r] + offset, curr_vm->regs[sr], curr_vm->memory);
    }
    break;
    case OP_NOT:
    {
        uint16_t dr = (instr >> 9) & 0x7;
        uint16_t sr = (instr >> 6) & 0x7;

        curr_vm->regs[dr] = ~curr_vm->regs[sr];
        update_flags(dr, curr_vm->regs);
    }
    break;
    case OP_TRAP:
    {
        running = execute_trap(curr_vm, instr, stdin, stdout);
    }
    break;
    default:
        abort();
    }
    if (running && is_max)
    {
        printf("Program counter overflow!");
        running = 0;
    }

    return running;
}