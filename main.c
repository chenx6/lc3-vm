#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>
#include "lc3.h"

/* terminal input setup */
struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

int main(int argc, char **argv)
{
    vm_ctx *curr_vm;
    if (argc == 2)
    {
        curr_vm = init_vm(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: %s [program]\n", argv[0]);
        exit(-1);
    }
    
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();
    int running = 1;
    while (running)
    {
        running = execute_inst(curr_vm);
    }
    restore_input_buffering();
    destory_vm(curr_vm);
    return 0;
}