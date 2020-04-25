KRNLDIR		:= kernel
MODDIR		:= module
APPDIR		:= apps
SYSROOT		:= root
RUNDIR		:= run
ISODIR		:= iso

MODS		:= $(wildcard $(MODDIR)/*/.)
APPS		:= $(wildcard $(APPDIR)/*/.)

NASM		:= nasm -f elf32
KCC		:= tools/bin/i686-pc-modit-gcc
KAS		:= tools/bin/i686-pc-modit-as
KAR		:= tools/bin/i686-pc-modit-ar
KLD		:= tools/bin/i686-pc-modit-ld

WARNINGS	:= -Wall -Wextra -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wdouble-promotion -Wshadow
KCFLAGS		:= -std=gnu99 -ffreestanding -MMD -MP $(WARNINGS) -I$(SYSROOT)/usr/include -g
ENVFLAGS	:=

MULTIBOOT	= 2
ARCH		= x86

ifeq ($(ARCH),x86)
	ENVFLAGS += -DBITS_32
	ENVFLAGS += -DARCH_I386
endif

ifeq ($(MULTIBOOT),2)
	ENVFLAGS += -DMULTIBOOT_2
else
	ENVFLAGS += -DMULTIBOOT_1
endif

.PHONY: all iso kernel mods $(MODS) libs apps $(APPS) initrd tools clean run

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
	$(MAKE) -C $(KRNLDIR) build ARCH=$(ARCH) MULTIBOOT=$(MULTIBOOT)
	mkdir -p root/boot
	cp $(KRNLDIR)/kernel.bin root/boot/kernel.bin

initrd: mods libs apps | Makefile tools/
	cd initrd ; tar -cvf ../root/boot/initrd.tar *

mods: $(MODS)
$(MODS) : Makefile tools/
	mkdir -p initrd
	$(MAKE) -C $@ ENVFLAGS="$(ENVFLAGS)"

libs:
	$(MAKE) -C lib/ build
	$(MAKE) -C lib/libc build
	$(MAKE) -C lib/libmicro build

apps: $(APPS) libs
$(APPS) : Makefile tools/ libs
	mkdir -p initrd
	$(MAKE) -C $@ ENVFLAGS="$(ENVFLAGS)"

tools/:
tools:
	util/buildtools.sh

run:
	qemu-system-i386 -cdrom run/moditos.iso -s -serial stdio -monitor tcp:127.0.0.1:55555,server,nowait -m 64M -smp cores=2

clean:
	$(MAKE) -C $(KRNLDIR) clean
	rm $(RUNDIR)/modetos.iso | true
	rm initrd/* | true
	rm module/*/*.o | true
	rm module/*/*.ko | true
	$(MAKE) -C lib/ clean
	$(MAKE) -C lib/libc clean
	$(MAKE) -C lib/libmicro clean
