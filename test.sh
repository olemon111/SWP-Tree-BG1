#!/usr/bin/env bash
mkdir -p build
cd build
rm -rf *
cmake ..
make -j16
cd ..
sudo rm -f /mnt/pmem1/lbl/apex*
chmod +x ./tests/run_microbench_swp.sh
sudo ./tests/run_microbench_swp.sh
sudo rm -f /mnt/pmem1/lbl/apex*
