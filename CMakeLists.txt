project(WOLI)
cmake_minimum_required(VERSION 3.0)

# cmake -D CMAKE_C_COMPILER=/usr/local/bin/gcc CMAKE_CXX_COMPILER=/usr/local/bin/g++ ../
# SET(CMAKE_C_COMPILER /usr/local/bin/gcc)
# SET(CMAKE_CXX_COMPILER /usr/local/bin/g++)
set(MKL_LINK_DIRECTORY "/opt/intel/mkl/lib/intel64")
set(MKL_INCLUDE_DIRECTORY "/opt/intel/mkl/include")
include(FetchContent)
include(ExternalProject)

# use `cmake -DSERVER:BOOL=ON ..` when running in server
option(SERVER "Run in server" ON)
option(USE_LIBPMEM "libpmem or libvmmalloc" ON)
option(BUF_SORT "Sort buffer in KVBufer" OFF)
option(STREAMING_LOAD "Use Non-temporal Load" OFF)
option(STREAMING_STORE "Use Non-temporal Store" OFF)
option(NO_LOCK "Don't use lock" OFF)
option(BRANGE "Multi-thread expanding" ON)
option(NO_ENTRY_BUF "BEntry without KVBuffer" ON)

# ComboTree Configuration
# use `make clean && make CXX_DEFINES="-DNAME=VALUE"` to override during compile
if(SERVER)
  set(PMEM_DIR \"/mnt/pmem0/\")
  set(CLEVEL_PMEM_FILE_SIZE "(1024*1024*1024*32UL)")
  set(CLEVEL_PMEM_FILE \"/mnt/pmem0/combotree-clevel-\")
  set(BLEVEL_PMEM_FILE \"/mnt/pmem0/combotree-blevel-\")
  set(ALEVEL_PMEM_FILE \"/mnt/pmem0/combotree-alevel-\")
  set(PGM_INDEX_PMEM_FILE \"/mnt/pmem0/combotree-pgmindex-\")
  set(COMMON_PMEM_FILE \"/mnt/pmem0/common-alloctor\")
else()
  set(PMEM_DIR \"/mnt/pmem0/\")
  set(CLEVEL_PMEM_FILE_SIZE "(1024*1024*512UL)")
  set(CLEVEL_PMEM_FILE \"/mnt/pmem0/combotree-clevel-\")
  set(BLEVEL_PMEM_FILE \"/mnt/pmem0/combotree-blevel-\")
  set(ALEVEL_PMEM_FILE \"/mnt/pmem0/combotree-alevel-\")
  set(PGM_INDEX_PMEM_FILE \"/mnt/pmem0/combotree-pgmindex-\")
  set(COMMON_PMEM_FILE \"/mnt/pmem0/common-alloctor\")
endif(SERVER)

if(BRANGE)
  set(EXPAND_THREADS 4)
endif(BRANGE)

if(NO_ENTRY_BUF)
  set(BLEVEL_EXPAND_BUF_KEY 4)
else()
  set(BLEVEL_EXPAND_BUF_KEY 6)
endif(NO_ENTRY_BUF)

set(EXPANSION_FACTOR 2)
set(DEFAULT_SPAN 2)
set(PMEMKV_THRESHOLD 10000)
set(ENTRY_SIZE_FACTOR 1.2)

configure_file(
  "${PROJECT_SOURCE_DIR}/src/combotree_config.h.in"
  "${PROJECT_SOURCE_DIR}/src/combotree_config.h"
)

# -march=native used to detect CPU flags and use corrosponding clflush instruction
set(CMAKE_CXX_FLAGS "-march=native -Wall -Wno-reorder -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mrtm -msse4.1 -mavx2")

# Require C++17
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE(OpenMP REQUIRED)

if(OPENMP_FOUND)
  message("OPENMP FOUND")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_definitions(-DNDEBUG -DNDEBUGGING)

# MK for RMI
link_directories(${MKL_LINK_DIRECTORY})
include_directories(${MKL_INCLUDE_DIRECTORY})

# combotree shared library
include_directories(include)

list(APPEND libs_to_link pmemobj pmem tbb papi tcmalloc_minimal mkl_rt pthread)

set(COMBO_TREE_SRC
  src/alevel.cc
  src/enviroment.cc
  src/pgm_index.cc
  src/rmi_index.cc
  src/learn_index.cc
  src/blevel.cc
  src/bentry.cc
  src/clevel.cc
  src/combotree.cc
  src/pmemkv.cc

  src/lbtree.cc
  src/mempool.cc
  src/nvm-common.cc
  src/tree.cc

  # src/lbtree_wrapper.cc

  # src/lbtree_wrapper.hpp
)

add_library(combotree SHARED ${COMBO_TREE_SRC})
target_link_libraries(combotree pmem pmemobj mkl_rt pthread)

# # benchmark
# add_executable(benchmark tests/benchmark.cc)
# target_link_libraries(benchmark combotree)

# # multi_benchmark
# add_executable(multi_benchmark tests/multi_benchmark.cc)
# target_link_libraries(multi_benchmark combotree)

# # multi_benchmark
# add_executable(multi_bench tests/multi_bench.cc)
# target_link_libraries(multi_bench combotree)

# # scalability_test
# add_executable(scalability_test tests/scalability_test.cc)
# target_link_libraries(scalability_test combotree)

# Unit Test
enable_testing()

include_directories(src)

# # # generate_data
# add_executable(generate_data tests/generate_data.cc)

# # # sortbuffer_test
# add_executable(sortbuffer_test tests/sortbuffer_test.cc)

# # # clevel_test
# add_executable(clevel_test tests/clevel_test.cc src/clevel.cc)
# target_link_libraries(clevel_test pmem)
# add_test(clevel_test clevel_test)

# # # combotree_test
# add_executable(combotree_test tests/combotree_test.cc)
# target_link_libraries(combotree_test combotree)
# add_test(combotree_test combotree_test)

# # # multi_combotree_test
# add_executable(multi_combotree_test tests/multi_combotree_test.cc)
# target_link_libraries(multi_combotree_test combotree)
# add_test(multi_combotree_test multi_combotree_test)

# # # pgm_index_test
# add_executable(pgm_index_test tests/pgm_index_test.cc)
# target_link_libraries(pgm_index_test combotree)
# add_test(pgm_index_test pgm_index_test)

# # # pgm_index_test
# add_executable(xindex_test tests/xindex_test.cc)
# target_link_libraries(xindex_test combotree)
# add_test(xindex_test xindex_test)

# # # learn_index_test
# add_executable(learn_index_test tests/learn_index_test.cc)
# target_link_libraries(learn_index_test combotree)
# add_test(learn_index_test learn_index_test)

# # # alex_test
# add_executable(alex_test tests/alex_test.cc)
# target_link_libraries(alex_test combotree)
# add_test(alex_test alex_test)

# # bucket_test
# add_executable(bucket_test tests/bucket_test.cc)
# target_link_libraries(bucket_test combotree)
# add_test(bucket_test bucket_test)

# # # nvm_test
# add_executable(nvm_test tests/nvm_test.cc)
# target_link_libraries(nvm_test pmem)
# add_test(nvm_test nvm_test)

# # # microbench
# add_executable(microbench tests/micro_bench.cc)
# target_link_libraries(microbench combotree)
# add_test(microbench microbench)

# # microbench_swp
add_executable(microbench_swp tests/micro_bench_swp.cc)

# add_dependencies(microbench_swp PMDK)
# target_link_libraries(microbench_swp PRIVATE ${libs_to_link})
target_link_libraries(microbench_swp combotree)
add_test(microbench_swp microbench_swp)

# # # ycsb
# add_executable(ycsb tests/ycsb.cc)
# target_link_libraries(ycsb combotree)
# add_test(ycsb ycsb)

# add_executable(ycsb-mem tests/ycsb-mem.cc)
# target_link_libraries(ycsb-mem combotree)
# add_test(ycsb-mem ycsb-mem)
add_definitions(-DPMEM)
add_definitions(-DPMDK)
message(STATUS "Persistence support: PMDK")

# # #################### PMDK ####################
# set(PMDK_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/pmdk")
# ExternalProject_Add(PMDK
# GIT_REPOSITORY git@github.com:XiangpengHao/pmdk.git
# GIT_TAG addr-patch
# BUILD_IN_SOURCE 1
# BUILD_COMMAND $(MAKE) EXTRA_CFLAGS=-Wno-error
# PREFIX ${PMDK_PREFIX}
# CONFIGURE_COMMAND ""
# INSTALL_COMMAND ""
# LOG_DOWNLOAD ON
# LOG_CONFIGURE ON
# LOG_BUILD ON
# )

# include_directories(${PMDK_PREFIX}/src/PMDK/src/include)

# # if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
# # link_directories(${PMDK_PREFIX}/src/PMDK/src/debug)
# # else()
# link_directories(${PMDK_PREFIX}/src/PMDK/src/nondebug)

# # endif()

# #################### Epoch Reclaim ####################
FetchContent_Declare(
  epoch_reclaimer
  GIT_REPOSITORY git@github.com:XiangpengHao/VeryPM.git
  GIT_TAG master
)

# GIT_REPOSITORY https://github.com/XiangpengHao/epoch-reclaimer.git
if(NOT epoch_reclaimer_POPULATED)
  FetchContent_Populate(epoch_reclaimer)
  include_directories(${epoch_reclaimer_SOURCE_DIR})
endif()