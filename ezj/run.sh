#!/bin/bash

mkdir -p mp sd
fail=n
[ -e stage1.gb ] || {
    echo "Error: Need stage1.gb  (From https://github.com/daid/ezflashjr/blob/master/stage1/FW5/stage1.gb)"
    fail=y
}

[ -e ezj_sd.dat ] || {
    echo "Error: Need ezj_sd.dat (Put files and ezgb.dat to dir 'sd', then run makesd.sh)"
    fail=y
}
[ "${fail}" = "y" ] && exit 1

../gnuboy --source cfg.rc 'stage1.gb'
