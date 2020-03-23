SYSROOT	:= ../../root

OBJS	:= $(patsubst %.c,%.o,$(wildcard *.c))

WARN	:= -Wall -Wextra -pedantic -Wduplicated-cond -Wduplicated-branches
WARN	+= -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow

LD	:= ../../tools/bin/i686-pc-modit-ld
CC	:= ../../tools/bin/i686-pc-modit-gcc

CFLAGS	:= -std=gnu99 -ffreestanding -MMD -MP $(WARN) -I$(SYSROOT)/usr/include

all: $(OBJS)
	$(LD) -r $^ -o ../../initrd/testmod.ko

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ -g
