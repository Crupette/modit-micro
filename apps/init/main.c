#include "micro/io.h"

int main(int argc, char **argv){
    syscall_print("Hello, World!\n");

    while(1) {}
    return 0;
}
