#!/bin/sh
if [ $# != 1 ] ; then
echo "usage \"$0 version\". \neg: $0 3.00 "
exit 1;
fi

rm home/root/ -rf
mkdir -p home/root/
cp linux_app/build/linux_app.elf home/root
cp rtos_app/code/acur101_app/Debug/app.bin home/root
cp xen-app.cfg home/root
tar czf app.tar.gz home/root
./tools/firmware_package_tool/firmware_package sdk/images/linux/BOOT.BIN sdk/images/linux/boot.scr sdk/images/linux/xen sdk/images/linux/system.dtb sdk/images/linux/Image sdk/images/linux/rootfs.cpio.gz app.tar.gz
#./tools/firmware_package_tool/firmware_package sdk/images/linux/boot.scr sdk/images/linux/xen sdk/images/linux/system.dtb sdk/images/linux/Image sdk/images/linux/rootfs.cpio.gz
python3 rtos_app/build/build_acur101_OTA.py rtos_app/release/header_info.ini out.bin $1
rm out.bin
echo "done........"

