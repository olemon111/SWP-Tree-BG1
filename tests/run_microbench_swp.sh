#!/bin/bash
BUILDDIR=$(dirname "$0")/../build
Loadname="ycsb-swp"
function Run() {
    dbname=$1
    loadnum=$2 
    opnum=$3
    scansize=$4
    thread=$5
    theta=$6

    # # gdb --args #

    # microbench_swp

    # # rm -f /mnt/AEP0/*
    # Loadname="ycsb-read"
    # date | tee microbench-swp-${dbname}-${Loadname}.txt
    # LD_PRELOAD=libhugetlbfs.so HUGETLB_MORECORE=yes numactl --cpubind=0 --membind=0 ${BUILDDIR}/microbench_swp --dbname ${dbname} --load-size ${loadnum} \
    # --put-size 0 --get-size ${opnum} --workload ${WorkLoad} \
    # --loadstype 1 --theta ${theta} -t $thread | tee -a microbench-swp-${dbname}-${Loadname}.txt

    # echo "${BUILDDIR}/microbench_swp --dbname ${dbname} --load-size ${loadnum} "\
    # "--put-size ${0} --get-size ${opnum} --workload ${WorkLoad} --loadstype 1 --theta ${theta} -t $thread"
    # rm -f /mnt/AEP0/*
    Loadname="ycsb-read"
    date | tee microbench-swp-${dbname}-${Loadname}.txt
    ${BUILDDIR}/microbench_swp --dbname ${dbname} --load-size ${loadnum} \
    --put-size 0 --get-size ${opnum} \
    --loadstype 1 --theta ${theta} -t $thread | tee -a microbench-swp-${dbname}-${Loadname}.txt

    echo "${BUILDDIR}/microbench_swp --dbname ${dbname} --load-size ${loadnum} "\
    "--put-size 0 --get-size ${opnum} --loadstype 1 --theta ${theta} -t $thread"
}

function run_all() {
    dbs="fastfair apex lbtree"
    for dbname in $dbs; do
        echo "Run: " $dbname
        Run $dbname $1 $2 $3 1 $5
        sleep 100
    done
}

function main() {
    dbname="fastfair"
    loadnum=2000000
    opnum=10000000
    scansize=0
    theta=0.99
    thread=1
    if [ $# -ge 1 ]; then
        dbname=$1
    fi
    if [ $# -ge 2 ]; then
        loadnum=$2
    fi
    if [ $# -ge 3 ]; then
        opnum=$3
    fi
    if [ $# -ge 4 ]; then
        scansize=$4
    fi
    if [ $# -ge 5 ]; then
        thread=$5
    fi
    if [ $# -ge 6 ]; then
        theta=$6
    fi
    if [ $dbname == "all" ]; then
        run_all $loadnum $opnum $scansize $thread $theta
    else
        echo "Run $dbname $loadnum $opnum $scansize $thread $theta"
        Run $dbname $loadnum $opnum $scansize $thread $theta
    fi 
}

main fastfair 2000000 10000000 0 1 0.99
# main apex 2000000 10000000 0 1
# main lbtree 2000000 10000000 0 1
# main all 2000000 10000000 0 1