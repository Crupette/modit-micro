SYSROOT		:= ../../root

OBJS		:= $(patsubst %.c, %.o, $(wildcard *.c))
OBJS		+= $(patsubst %.asm, %.o, $(wildcard *.asm))

OUT			:= libmicro.a

CC			:= ../../tools/bin/i686-pc-modit-gcc
AR			:= ../../tools/bin/i686-pc-modit-ar
ASM			:= nasm -f elf32

WARN		:= -Wall -Wextra -pedantic -Wduplicated-cond -Wduplicated-branches
WARN		+= -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow

CFLGS		:= -std=gnu99 -ffreestanding -MMD -MP $(WARNING) -I$(SYSROOT)/usr/include

.PHONY:		build clean

build: $(OBJS)
	$(AR) cr $(OUT) ${OBJS}

%.o:%.c | Makefile
	$(CC) -c $< -o $@ $(CFLGS) -g

%.o:%.asm | Makefile
	$(ASM) $< -o $@

clean:
	rm $(OBJS)
