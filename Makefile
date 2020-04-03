KRNLDIR		:= kernel
MODDIR		:= module
SYSROOT		:= root
RUNDIR		:= run
ISODIR		:= iso

MODS		:= $(wildcard $(MODDIR)/*/.)

NASM		:= nasm -f elf32
KCC		:= tools/bin/i686-pc-modit-gcc
KAS		:= tools/bin/i686-pc-modit-as
KAR		:= tools/bin/i686-pc-modit-ar
KLD		:= tools/bin/i686-pc-modit-ld

WARNINGS	:= -Wall -Wextra -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow
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

kernel: | tools/
	ln -s ../tools -t $(KRNLDIR) | true
	ln -s ../../../kernel/include -T root/usr/include/kernel | true
	$(MAKE) -C $(KRNLDIR) build
	cp $(KRNLDIR)/kernel.bin root/boot/kernel.bin

mods: $(MODS)
$(MODS) : Makefile tools/
	$(MAKE) -C $@

initrd: mods | Makefile tools/
	cd initrd ; tar -cvf ../root/boot/initrd.tar *

tools/:
tools:
	util/buildtools.sh

clean:
	$(MAKE) -C $(KRNLDIR) clean
	rm -f $(RUNDIR)/modetos.iso
	rm -f initrd/*

run:
	qemu-system-i386 -cdrom run/moditos.iso -s -serial stdio -monitor tcp:127.0.0.1:55555,server,nowait -m 64M
