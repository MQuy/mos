#!/bin/bash
set -Eeuo pipefail

DISK_NAME=mos

sudo mkdir "/mnt/${DISK_NAME}"
sudo mount -o loop ../../hdd.img "/mnt/${DISK_NAME}"

rm -rf bash-5.0
rm -rf build-bash
rm -f bash-5.0.tar.gz

wget ftp://ftp.gnu.org/gnu/bash/bash-5.0.tar.gz
tar -xzvf bash-5.0.tar.gz

cd bash-5.0
patch -p1 < ../bash-5.0.patch
cd ..

mkdir build-bash && cd build-bash
../bash-5.0/configure --host=i386-pc-mos --disable-nls --without-bash-malloc bash_cv_getenv_redef=no bash_cv_getcwd_malloc=yes
make clean && make CC=i386-pc-mos-gcc LD=i386-pc-mos-gcc
sudo make install prefix=/mnt/${DISK_NAME}
cd ..

sudo umount "/mnt/${DISK_NAME}"
sudo rm -rf "/mnt/${DISK_NAME}"
