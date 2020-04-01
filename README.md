# Modit! Micro
An operating system based on the Modit! kernel. Uses modules to make the kernel sufficient for use in a microkernel.

### Cloning
The kernel is added as it's own submodule. To clone, type

```
git clone --recursive https://github.com/Crupette/modit-micro
```

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
