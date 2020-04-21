SYSROOT	:= ../../root
ROOT	:= ../..

OBJS	:= $(patsubst %.c,%.o,$(wildcard *.c))
OBJS	+= $(patsubst %.asm,%.o,$(wildcard *.asm))

LIBS	:= $(ROOT)/lib/libbind.a $(ROOT)/lib/libc/libc.a $(ROOT)/lib/libmicro/libmicro.a

WARN	:= -Wall -Wextra -Wduplicated-cond -Wduplicated-branches
WARN	+= -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow

LD		:= ../../tools/bin/i686-pc-modit-ld
CC		:= ../../tools/bin/i686-pc-modit-gcc
ASM		:= nasm -f elf32

CFLAGS	:= -std=gnu99 -ffreestanding -MMD -MP $(WARN) -I$(SYSROOT)/usr/include

OUT		:= init

all: $(OBJS) $(LIBS)
	$(LD) $^ -o $(OUT)
	mv $(OUT) ../../initrd/$(OUT)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ -g

%.o:%.asm
		$(ASM) $< -o $@
