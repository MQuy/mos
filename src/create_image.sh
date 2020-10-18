#!/bin/bash
set -Eeuo pipefail

unamestr=`uname`

cd libraries/libc && make clean && make && cd ../..

if [[ "$unamestr" == 'Linux' ]]; then
  dd if=/dev/zero of=hdd.img count=20480 bs=512
  DISK_NAME=mos
  mkfs.ext2 hdd.img
  sudo mkdir "/mnt/${DISK_NAME}"
  sudo mount -o loop hdd.img "/mnt/${DISK_NAME}"

  sudo mkdir "/mnt/${DISK_NAME}/dev"
  sudo mkdir "/mnt/${DISK_NAME}/usr"
  sudo mkdir "/mnt/${DISK_NAME}/usr/share"

  sudo cp -R assets/fonts "/mnt/${DISK_NAME}/usr/share"
  sudo cp -R assets/images "/mnt/${DISK_NAME}/usr/share"

  cd apps/window_server && make clean && make
  cd ../..
  cd apps/terminal && make clean && make
  cd ../..
  cd apps/shell && make clean && make
  cd ../..
  cd apps/uname && make clean && make
  cd ../..
  cd apps/host && make clean && make
  cd ../..
  cd apps/calculator && make clean && make
  cd ../..
  cd apps/ld && rm -f ld && i386-mos-gcc -g ld.c -o ld
  cd ../..

  sudo mkdir "/mnt/${DISK_NAME}/bin"
  sudo cp apps/window_server/window_server "/mnt/${DISK_NAME}/bin"
  sudo cp apps/terminal/terminal "/mnt/${DISK_NAME}/bin"
  sudo cp apps/shell/shell "/mnt/${DISK_NAME}/bin"
  sudo cp apps/uname/uname "/mnt/${DISK_NAME}/bin"
  sudo cp apps/host/host "/mnt/${DISK_NAME}/bin"
  sudo cp apps/calculator/calculator "/mnt/${DISK_NAME}/bin"
  sudo cp apps/ld/ld "/mnt/${DISK_NAME}/bin"

  sudo mkdir "/mnt/${DISK_NAME}/etc"
  sudo cp apps/window_server/desktop.ini "/mnt/${DISK_NAME}/etc"

  sudo mkdir "/mnt/${DISK_NAME}/tmp"
  sudo cp assets/book.txt "/mnt/${DISK_NAME}/tmp"

  sudo umount "/mnt/${DISK_NAME}"
  sudo rm -rf "/mnt/${DISK_NAME}"
elif [[ "$unamestr" == 'Darwin' ]]; then
  dd if=/dev/zero of=hdd.img count=20480 bs=512
  VOLUME_NAME=hdd
  DISK_NAME="$(hdiutil attach -nomount hdd.img)"
  $(brew --prefix e2fsprogs)/sbin/mkfs.ext2 $DISK_NAME
  hdiutil detach $DISK_NAME
  hdiutil attach hdd.img -mountpoint /Volumes/$VOLUME_NAME
  mkdir "/Volumes/${VOLUME_NAME}/dev"

  mkdir "/Volumes/${VOLUME_NAME}/usr"
  mkdir "/Volumes/${VOLUME_NAME}/usr/share"
  cp -R assets/fonts "/Volumes/${VOLUME_NAME}/usr/share"
  cp -R assets/images "/Volumes/${VOLUME_NAME}/usr/share"

  cd apps/window_server && make clean && make
  cd ../..
  cd apps/terminal && make clean && make
  cd ../..
  cd apps/shell && make clean && make
  cd ../..
  cd apps/uname && make clean && make
  cd ../..
  cd apps/host && make clean && make
  cd ../..
  cd apps/calculator && make clean && make
  cd ../..
  cd apps/ld && make clean && make
  cd ../..

  mkdir "/Volumes/${VOLUME_NAME}/bin"
  cp apps/window_server/window_server "/Volumes/${VOLUME_NAME}/bin"
  cp apps/terminal/terminal "/Volumes/${VOLUME_NAME}/bin"
  cp apps/shell/shell "/Volumes/${VOLUME_NAME}/bin"
  cp apps/uname/uname "/Volumes/${VOLUME_NAME}/bin"
  cp apps/host/host "/Volumes/${VOLUME_NAME}/bin"
  cp apps/calculator/calculator "/Volumes/${VOLUME_NAME}/bin"
  cp apps/ld/ld "/Volumes/${VOLUME_NAME}/bin"

  mkdir "/Volumes/${VOLUME_NAME}/etc"
  cp apps/window_server/desktop.ini "/Volumes/${VOLUME_NAME}/etc"

  mkdir "/Volumes/${VOLUME_NAME}/tmp"
  cp assets/book.txt "/Volumes/${VOLUME_NAME}/tmp"

  hdiutil detach $DISK_NAME

  DISK_NAME="$(hdiutil attach -nomount hdd.img)"
  $(brew --prefix e2fsprogs)/sbin/dumpe2fs $DISK_NAME
  hdiutil detach $DISK_NAME
fi

# use debugfs to check hdd is created correctly or not
