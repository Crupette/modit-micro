#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

TARGET=i686-pc-modit
PREFIX="$DIR/../tools"
SYSROOT="$DIR/../root"

mkdir $PREFIX
cd $PREFIX

mkdir src
pushd src
	if [ ! -e "binutils-2.32.tar.gz" ]; then
		wget "https://ftp.gnu.org/gnu/binutils/binutils-2.32.tar.gz"
	fi
	if [ ! -e "gcc-8.3.0.tar.gz" ]; then
		wget "https://ftp.gnu.org/gnu/gcc/gcc-8.3.0/gcc-8.3.0.tar.gz"
	fi

	rm -rf "binutils-2.32"
	tar -xf "binutils-2.32.tar.gz"
	
	cp "$DIR/toolchain/binutils.patch" ./
	patch -p0 -i binutils.patch


	rm -rf "gcc-8.3.0"
	tar -xf "gcc-8.3.0.tar.gz"
	
	cp "$DIR/toolchain/gcc.patch" ./
	patch -p0 -i gcc.patch
	
	pushd "gcc-8.3.0"
		./contrib/download_prerequisites
	popd
popd

mkdir -p build/gcc build/binutils

pushd build
	pushd binutils
		$PREFIX/src/binutils-2.32/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-werror
		make
		make install
		rm -f "$PREFIX/src/binutils-2.32.tar.gz"
		rm -rf "$PREFIX/src/binutils-2.32"
	popd

	pushd gcc
		$PREFIX/src/gcc-8.3.0/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot="$SYSROOT" --enable-languages=c
		make all-gcc
		make all-target-libgcc
		make install-gcc
		make install-target-libgcc
		rm -f "$PREFIX/src/gcc-8.3.0.tar.gz"
		rm -rf "$PREFIX/src/gcc-8.3.0"
	popd
popd
