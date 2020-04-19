SYSROOT		:= ../../root

OBJS		:= $(patsubst %.asm, %.o, $(wildcard *.asm))

OUT			:= libbind.a

ASM			:= nasm -f elf32

.PHONY:		build clean

build: $(OBJS)
		$(AR) cr $(OUT) ${OBJS}

%.o:%.asm | Makefile
	$(ASM) $< -o $@

clean:
	rm $(OBJS)
