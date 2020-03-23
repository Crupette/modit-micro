# Modit! OS
A micro-kernel with a modular kernel framework.

I'm doing this as a hobby, so don't expect Linux 2.0

Besides, it's not even going to be POSIX!

### Building
The makefile can create it's own toolchain
```
make tools
```
This gets run when the tools/ directory is not found

It builds GCC (8.3.0) and Binutils (2.32)

To make the kernel, type
```
make iso
```
and to run, type
```
make run
```
Running requires qemu-system-i386, so install that if you haven't

## Acknowledgments
* The nice people at the [OSDev Wiki](https://wiki.osdev.org/Main_Page) and [Forums](https://forum.osdev.org/)
