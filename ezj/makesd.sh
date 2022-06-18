#!/bin/bash

sudo echo "make files in dir 'sd' to ezj_sd.dat ..."
mkdir -p sd mp
[ -n "$(ls -A sd)" ] || echo "Warn: dir 'sd' is empty."
[ -e sd/ezgb.dat ] || echo "Warn: Need ezgb.dat (from ezflash.cn) in dir 'sd'."

rm -f ezj_sd.dat
dd if=/dev/zero of=ezj_sd.dat bs=1M count=1 seek=$((4 * 1024 -1))
mkfs.fat -F 32 -s 32 ezj_sd.dat
sudo mount ezj_sd.dat mp -o utf8
sudo cp -r sd/* mp/
sudo umount mp
echo "Real size of ezj_sd.dat"
du -h ezj_sd.dat
