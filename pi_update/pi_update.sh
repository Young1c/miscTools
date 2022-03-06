#!/bin/sh

VERSION=$1
FIRMWARE_PATH=/boot/firmware
ROOT_DIR=$(pwd)

su

dpkg -i *.deb

cd /boot

rm vmlinuz
ln -s vmlinuz-$VERSION vmlinuz

rm initrd.img
ln -s initrd.img-$VERSION initrd.img

cp vmlinuz firmware/
cp initrd.img firmware/

