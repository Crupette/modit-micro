#include <stdio.h>
#include <sys/proc.h>

int main(int argc, char **argv){   
    spawnl("procsvr", NULL);

    while(1) {}
    return 0;
}
