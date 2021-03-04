#!/bin/bash
set -Eeuo pipefail

DISK_NAME=mos

rm -rf figlet-2.2.5

if [ ! -f figlet-2.2.5.tar.gz ]; then
  wget http://ftp.figlet.org/pub/figlet/program/unix/figlet-2.2.5.tar.gz
fi

tar -xzvf figlet-2.2.5.tar.gz

cd figlet-2.2.5
patch -p1 < ../figlet-2.2.5.patch

make clean
make CC=i386-pc-mos-gcc LD=i386-pc-mos-gcc

sudo mkdir "/mnt/${DISK_NAME}"
sudo mount -o loop ../../../hdd.img "/mnt/${DISK_NAME}"

sudo make install DESTDIR=/mnt/${DISK_NAME} BINDIR=/bin MANDIR=/man

sudo umount "/mnt/${DISK_NAME}"
sudo rm -rf "/mnt/${DISK_NAME}"

cd ..
