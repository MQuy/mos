dd if=/dev/zero of=hdd.img count=20480 bs=512
VOLUME_NAME=hdd
DISK_NAME="$(hdiutil attach -nomount hdd.img)"
$(brew --prefix e2fsprogs)/sbin/mkfs.ext2 $DISK_NAME
hdiutil detach $DISK_NAME
hdiutil attach hdd.img -mountpoint /Volumes/$VOLUME_NAME
mkdir "/Volumes/${VOLUME_NAME}/demo"
cp sample.txt "/Volumes/${VOLUME_NAME}/demo/sample.txt"
cp -R assets/fonts "/Volumes/${VOLUME_NAME}"
mkdir "/Volumes/${VOLUME_NAME}/bin"
mkdir "/Volumes/${VOLUME_NAME}/dev"
cp apps/ui/ui "/Volumes/${VOLUME_NAME}/bin"
hdiutil detach $DISK_NAME

DISK_NAME="$(hdiutil attach -nomount hdd.img)"
$(brew --prefix e2fsprogs)/sbin/dumpe2fs /dev/disk2
hdiutil detach $DISK_NAME