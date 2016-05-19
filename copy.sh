#!/bin/bash
mount /dev/sdb1 /media/cd || exit 0
cp out/arm9loaderhax.bin /media/cd/anim/boot/none.bin || exit 0
cp out/arm9loaderhax.bin /media/cd/anim/boot/r.bin || exit 0
cp out/arm9loaderhax.bin /media/cd/anim/boot/l.bin || exit 0
cp -r out/corbenik /media/cd/ || exit 0
cp -r input/corbenik /media/cd/ || exit 0
umount /media/cd || exit 0
eject /dev/sdb || exit 0