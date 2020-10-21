DISK_NAME=mos

rm -rf figlet-2.2.5
rm -f figlet-2.2.5.tar.gz

wget http://ftp.figlet.org/pub/figlet/program/unix/figlet-2.2.5.tar.gz
tar -xzvf figlet-2.2.5.tar.gz

cd figlet-2.2.5
patch -p1 < ../figlet-2.2.5.patch

make clean
make CC=i386-mos-gcc LD=i386-mos-gcc
sudo make install DESTDIR=/mnt/${DISK_NAME} BINDIR=/bin MANDIR=/man
