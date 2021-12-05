#!/bin/bash
set -Eeuo pipefail

CURRENT_DIR=`pwd`
COREUTILS_DIR=./coreutils-8.31
DISK_NAME=mos

if [ "${1-default}" == "make" ]
then
  if [ ! -d $COREUTILS_DIR ]; then
    git clone https://github.com/coreutils/coreutils coreutils-8.31
    cd coreutils-8.31
    git checkout v8.31

    ./bootstrap
    git submodule foreach git pull origin master
    ./bootstrap

    patch -p1 < $CURRENT_DIR/coreutils-8.31.patch

    cd gnulib
    patch -p1 < $CURRENT_DIR/gnulib.patch
    cd ..

    # cross-compiling has the issue of missing primes.h -> we use native tool to generate primes.h
    ./configure ac_cv_header_sys_cdefs_h=yes ac_cv_header_sys_sysctl_h=xyes
    make -j4 CFLAGS="-Wno-error=suggest-attribute=const -Wno-error=stringop-overflow"
    make distclean

    # ports/coreutils
    cd $CURRENT_DIR
    rm -rf build-coreutils && mkdir build-coreutils && cd build-coreutils
  fi

  # ports/coreutils
  cd $CURRENT_DIR/build-coreutils

  ../$COREUTILS_DIR/configure --host=i386-pc-mos --disable-acl --disable-threads --disable-libsmack --disable-xattr --disable-libcap --without-selinux --without-linux-crypto --without-openssl --without-gmp ac_cv_func_link=yes ac_cv_func_linkat=yes gl_cv_func_frexp_works=yes gl_cv_long_double_equals_double=yes stdbuf_supported=no
  make clean && make -j4 CFLAGS="-Wno-error=suggest-attribute=const -Wno-error=maybe-uninitialized -Wno-error=type-limits -Wno-error=implicit-function-declaration -Wno-error=return-type -Wno-error=misleading-indentation -Wno-error=unused-const-variable= -Wno-error=int-conversion -Wno-error=switch-unreachable -Wno-error=cast-align -Wno-error=unused-variable -Wno-error=incompatible-pointer-types -Wno-error=pointer-sign -Wno-error=shadow -Wno-error=unused-but-set-variable -Wno-error=empty-body -g -O0" CC=i386-pc-mos-gcc LD=i386-pc-mos-gcc
else
  cd build-coreutils
fi

unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
  DISK_NAME=mos

  sudo mkdir "/mnt/${DISK_NAME}"
  sudo mount -o loop ../../../hdd.img "/mnt/${DISK_NAME}"

  sudo make install prefix=/mnt/${DISK_NAME}

  sudo umount "/mnt/${DISK_NAME}"
  sudo rm -rf "/mnt/${DISK_NAME}"

  cd ../../

elif [[ "$unamestr" == 'Darwin' ]]; then
  VOLUME_NAME=hdd
  DISK_NAME="$(hdiutil attach -nomount ../../../hdd.img)"
  hdiutil attach ../../../hdd.img -mountpoint /Volumes/$VOLUME_NAME

  sudo make install prefix=/Volumes/${VOLUME_NAME}

  hdiutil detach $DISK_NAME

  cd ../../
fi