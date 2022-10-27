#!/usr/bin/env bash
mkdir -p build
cd build
rm -rf *
cmake ..
make -j16
cd ..
sudo rm /mnt/lbl/apex*
chmod +x ./tests/run_microbench_swp.sh