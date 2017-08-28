#! /bin/sh

verify_tool()
{
    which "$1" > /dev/null
    if [ "$?" != 0 ]; then
        echo "Can't find executable $1 in path"
        exit
    fi
}

verify_tool dd
verify_tool sfdisk
verify_tool losetup
verify_tool mkfs.vfat
verify_tool mount
verify_tool cp
verify_tool umount
verify_tool printf
verify_tool hexdump
verify_tool grep
verify_tool sed
verify_tool cut
verify_tool echo
verify_tool mkdir

if [ "$(whoami)" != "root" ];then
    echo "You must be root to run this script."
    exit
fi

export DISK_NUM_SECTORS=20480

get_addrs()
{
hexdump -b /tmp/minidisk.bin | grep -v "^[^ ][^ ][^ ][^ ][^ ][^ ][^ ]$" | grep -v "^\*$" | fgrep -v "000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000" | sed 's/ .*//' | sed 's/$/,/' | sed 's/^/0x/'
}
get_data()
{
hexdump -b /tmp/minidisk.bin  | grep -v "^[^ ][^ ][^ ][^ ][^ ][^ ][^ ]$" | grep -v "^\*$" | sed 's/^[^ ][^ ][^ ][^ ][^ ][^ ][^ ] //' | fgrep -v "000 000 000 000 000 000 000 000 000 000 000 000 000 000 000 000" | sed 's/\([0-9][0-9][0-9]\)/0\1/g' | sed 's/^/{/' | sed 's/$/},/' | sed 's/ /,/g'
}
get_partition_begin(){
sect=$(fdisk -l /tmp/minidisk.bin | fgrep "/tmp/minidisk.bin1" | sed 's/  */ /g'| cut -d " " -f 3)
echo "$sect*512" | bc
}
try_umount(){
umount "$1" 2> /dev/null > /dev/null
while [ "$?" -eq "1" ];do
    sleep 1
    umount "$1" 2> /dev/null > /dev/null
done
}

dd if=/dev/zero of=/tmp/minidisk.bin bs=512 count=$DISK_NUM_SECTORS 2> /dev/null
sfdisk /tmp/minidisk.bin < minidisk.fdisk 2> /dev/null > /dev/null

# Find a free loop device
LOOP_D=$(losetup -f)
if [ "$?" != 0 ];then
    echo "Something failed when getting loop device"
    exit
fi

losetup -o $(get_partition_begin) $LOOP_D /tmp/minidisk.bin 2> /dev/null
mkfs.vfat $LOOP_D 2> /dev/null > /dev/null

if [ ! -d /tmp/tmpmnt ];then
    mkdir /tmp/tmpmnt
    if [ "$?" != "0" ]; then
        exit
    fi
fi
mount $LOOP_D /tmp/tmpmnt
cp -r files/* /tmp/tmpmnt

try_umount /tmp/tmpmnt
losetup -d $LOOP_D


printf "// DO NOT EDIT THIS FILE!
// This file was generated using the script build_fake_partition.sh. In order to change this data, edit the files in the directory fakedisk/files and rerun the script.
// If you have just created this file, copy this text into src/libs/USBDevice/USBMSD/fake_disk_data.h
static const unsigned char data[%s+1][16] = {
%s
{}
};

static const uint32_t addrs[] = {%s 0xffffffff};
#define DISK_NUM_SECTORS %s;
" \
"$(get_addrs | wc -l)" \
"$(get_data)" \
"$(get_addrs | xargs echo)" \
$DISK_NUM_SECTORS

# Clean everything
rm /tmp/minidisk.bin
rmdir /tmp/tmpmnt