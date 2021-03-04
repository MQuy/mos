#!/bin/bash
set -Eeuo pipefail

PREFIX="$HOME/opt/cross"
TARGET=i386-pc-mos
# SYSROOT cannot locate inside PREFIX
SYSROOT="$HOME/Projects/mos/src/toolchain/sysroot"
JOBCOUNT=$(nproc)

setup() {
  rm -rf sysroot
  rm -rf binutils-2.32
  rm -rf build-binutils
  rm -f binutils-2.32.tar.gz
  rm -rf gcc-9.1.0
  rm -rf build-gcc
  rm -f gcc-9.1.0.tar.gz

  mkdir -p sysroot/usr
  cd sysroot/usr
  ln -s ../../../libraries/libc include
  ln -s ../../../libraries/libc lib
  cd ../..
}

binutils_fetch() {
  wget "https://ftp.gnu.org/gnu/binutils/binutils-2.32.tar.gz"
  tar -xzvf binutils-2.32.tar.gz
}

binutils_build() {
  cd binutils-2.32
  patch -p1 < ../binutils-2.32.patch
  cd ld
  automake
  cd ../.. && mkdir build-binutils
  cd build-binutils
  ../binutils-2.32/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot=$SYSROOT --disable-werror
  make -j $JOBCOUNT
  make install
  cd ..
}

gcc_fetch() {
  wget "https://ftp.gnu.org/gnu/gcc/gcc-9.1.0/gcc-9.1.0.tar.gz"
  tar -xzvf gcc-9.1.0.tar.gz
}

gcc_build() {
  cd gcc-9.1.0
  patch -p1 < ../gcc-9.1.0.patch
  ./contrib/download_prerequisites
  cd .. && mkdir build-gcc
  cd build-gcc
  ../gcc-9.1.0/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot=$SYSROOT --enable-languages=c,c++
  make all-gcc all-target-libgcc -j $JOBCOUNT
  make install-gcc install-target-libgcc
  cd ..
}

setup
binutils_fetch && binutils_build
gcc_fetch && gcc_build
