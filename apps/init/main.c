#include <stdio.h>
#include <sys/proc.h>
#include <sys/msg.h>

int main(int argc, char **argv){   
    spawnl("procsvr", NULL);

    printf("PID of procsvr is %i\n",
        find_process("procsvr"));
    //Requesting procsvr pid
    while(1) {}
    return 0;
}
