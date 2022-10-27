#!/usr/bin/env bash
sudo rm -f /mnt/pmem1/lbl/apex*
sudo ./tests/../build/microbench_swp --dbname apex --load-size 15 --put-size 0 --get-size 10000000 --loadstype 1 --theta 0.99 -t 1