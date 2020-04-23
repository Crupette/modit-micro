SYSROOT	:= ../../root

OBJS	:= $(patsubst %.c,%.o,$(wildcard *.c))
OBJS	+= $(patsubst %.asm,%.o,$(wildcard *.asm))

WARN	:= -Wall -Wextra -Wduplicated-cond -Wduplicated-branches
WARN	+= -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow

LD	:= ../../tools/bin/i686-pc-modit-ld
CC	:= ../../tools/bin/i686-pc-modit-gcc
ASM	:= nasm -f elf32

CFLAGS	:= -std=gnu99 -ffreestanding -MMD -MP $(WARN) -I$(SYSROOT)/usr/include
ENVFLAGS :=

OUT	:= gdt.ko

all: $(OBJS)
	$(LD) -r $^ -o $(OUT)
	cp $(OUT) ../../initrd/$(OUT) -r

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ -g $(ENVFLAGS)

%.o:%.asm
	$(ASM) $< -o $@
