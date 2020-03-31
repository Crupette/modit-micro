KRNLDIR		:= kernel
MODDIR		:= module
SYSROOT		:= root
RUNDIR		:= run
ISODIR		:= iso

KRNLOBJS	:= $(patsubst %.c,%.o,$(wildcard $(KRNLDIR)/*.c))
MODS		:= $(wildcard $(MODDIR)/*/.)

DEPS		:= $(patsubst %.c,%.d,$(wildcard $(KRNLDIR)/*.c))


NASM		:= nasm -f elf32
KCC		:= tools/bin/i686-pc-modit-gcc
KAS		:= tools/bin/i686-pc-modit-as
KAR		:= tools/bin/i686-pc-modit-ar
KLD		:= tools/bin/i686-pc-modit-ld

WARNINGS	:= -Wall -Wextra -pedantic -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow
KCFLAGS		:= -std=gnu99 -ffreestanding -MMD -MP $(WARNINGS) -I$(SYSROOT)/usr/include -g -DBITS32

.PHONY: all iso kernel mods $(MODS) initrd tools clean run

all: | tools/

iso: kernel initrd | tools/
	mkdir -p initrd
	mkdir -p mnt
	mkdir -p run
	mkdir -p iso/boot
	
	cp root/boot/kernel.bin iso/boot/modit.bin
	cp root/boot/initrd.tar iso/boot/initrd.tar
	grub-mkrescue -o $(RUNDIR)/moditos.iso $(ISODIR)

kernel: $(KRNLOBJS) | tools/
	mkdir -p root/boot
	$(NASM) $(KRNLDIR)/loader.asm -o $(KRNLDIR)/loader.o
	$(KCC) -T linker.ld -o $(SYSROOT)/boot/kernel.bin -ffreestanding -O2 -nostdlib $(KRNLDIR)/loader.o $(KRNLOBJS)

mods: $(MODS)
$(MODS) : Makefile tools/
	$(MAKE) -C $@

initrd: mods | Makefile tools/
	cd initrd ; tar -cvf ../root/boot/initrd.tar *

tools:
	util/buildtools.sh

clean:
	rm -f $(RUNDIR)/modetos.iso
	rm -f $(KRNLOBJS) $(KRNLDIR)/loader.o
	rm -f initrd/*

run:
	qemu-system-i386 -cdrom run/moditos.iso -s -serial stdio -monitor tcp:127.0.0.1:55555,server,nowait -m 64M

tools/:
	util/buildtools.sh


$(KRNLDIR)/%.o : $(KRNLDIR)/%.c Makefile
	$(KCC) -c $< -o $@ $(KCFLAGS) -g
