#!/bin/bash
set -Eeuo pipefail

if [ "${1-default}" == "make" ]
then
  rm -rf bash-5.0
  rm -rf build-bash

  if [ ! -f bash-5.0.tar.gz ]; then
    wget ftp://ftp.gnu.org/gnu/bash/bash-5.0.tar.gz
  fi

  tar -xzvf bash-5.0.tar.gz

  cd bash-5.0
  patch -p1 < ../bash-5.0.patch
  cd ..

  mkdir build-bash && cd build-bash
  ../bash-5.0/configure CFLAGS="-g" --host=i386-pc-mos --disable-nls --without-bash-malloc \
    bash_cv_getenv_redef=no bash_cv_getcwd_malloc=yes \
    ac_cv_func_sysconf=no ac_cv_func_select=no ac_cv_func_uname=no # remove these options when are supported in libc

  make clean && make CC=i386-pc-mos-gcc LD=i386-pc-mos-gcc -j4
else
  cd build-bash
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