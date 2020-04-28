#include <stdio.h>
#include <sys/proc.h>

int main(int argc, char **argv){   
    fork();
    puts("Hi!");

    while(1) {}
    return 0;
}
