ROOT	:= ../../../..
SROOT	:= $(ROOT)/root

OBJS	:= $(patsubst %.c,%.o,$(wildcard *.c))
OBJS	+= $(patsubst %.asm,%.o,$(wildcard *.asm))

WARNINGS	:= -Wall -Wextra -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow
CFLAGS		:= -std=gnu99 -ffreestanding -MMD -MP $(WARN) -I$(SROOT)/usr/include -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -z max-page-size=4096
ENVFLAGS	:= -DBITS_64 -DARCH_X86_64

TARGET	:= x86_64

ASM		:= nasm -f elf64
LD		:= $(ROOT)/tools/bin/$(TARGET)-pc-modit-ld
CC		:= $(ROOT)/tools/bin/$(TARGET)-pc-modit-gcc

OUT		:= irq.ko

all: $(OBJS)
	$(LD) -r $^ -o $(OUT)
	cp $(OUT) $(ROOT)/initrd/$(OUT) -r

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ -g $(ENVFLAGS)

%.o:%.asm
	$(ASM) $< -o $@
