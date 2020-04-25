#include "micro/io.h"

DEFN_SYSCALL0(fork, SYSCALL_FORK)

int main(int argc, char **argv){
    printf("Hello, Userspace! (%s)", "Hi");

    while(1) {}
    return 0;
}
