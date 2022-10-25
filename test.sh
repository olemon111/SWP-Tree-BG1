#!/usr/bin/env bash
mkdir -p build
cd build
rm -rf *
cmake ..
make -j4
cd ..
chmod +x ./tests/run_microbench_swp.sh
sudo ./tests/run_microbench_swp.sh