#include "micro/io.h"

DEFN_SYSCALL0(fork, SYSCALL_FORK)

int main(int argc, char **argv){
    syscall_print("Hello, World!\n");

    syscall_fork();

    syscall_print("Hi\n");

    while(1) {}
    return 0;
}
