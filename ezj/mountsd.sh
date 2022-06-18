#/bin/bash

mkdir -p sd mp
sudo mount ezj_sd.dat mp -o rw,noauto,users,uid=1000,gid=100,dmask=0000,fmask=0000,utf8
