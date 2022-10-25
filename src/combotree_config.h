#pragma once

#define SERVER
#define USE_LIBPMEM
/* #undef BUF_SORT */
/* #undef STREAMING_STORE */
/* #undef STREAMING_LOAD */
/* #undef NO_LOCK */
#define BRANGE
#define NO_ENTRY_BUF

#ifndef PMEM_DIR
#define PMEM_DIR "/mnt/pmem0/"
#endif
#ifndef CLEVEL_PMEM_FILE_SIZE
#define CLEVEL_PMEM_FILE_SIZE (1024*1024*1024*32UL)
#endif
#ifndef CLEVEL_PMEM_FILE
#define CLEVEL_PMEM_FILE      "/mnt/pmem0/combotree-clevel-"
#endif
#ifndef BLEVEL_PMEM_FILE
#define BLEVEL_PMEM_FILE      "/mnt/pmem0/combotree-blevel-"
#endif
#ifndef ALEVEL_PMEM_FILE
#define ALEVEL_PMEM_FILE      "/mnt/pmem0/combotree-alevel-"
#endif
#ifndef PGM_INDEX_PMEM_FILE
#define PGM_INDEX_PMEM_FILE      "/mnt/pmem0/combotree-pgmindex-"
#endif
#ifndef COMMON_PMEM_FILE
#define COMMON_PMEM_FILE      "/mnt/pmem0/common-alloctor"
#endif
#ifndef BLEVEL_EXPAND_BUF_KEY
#define BLEVEL_EXPAND_BUF_KEY 4
#endif
#ifndef DEFAULT_SPAN
#define DEFAULT_SPAN          2
#endif
#ifndef PMEMKV_THRESHOLD
#define PMEMKV_THRESHOLD      10000
#endif
#ifndef EXPANSION_FACTOR
#define EXPANSION_FACTOR      2
#endif
#ifndef ENTRY_SIZE_FACTOR
#define ENTRY_SIZE_FACTOR     1.2
#endif
#if defined(BRANGE) && !defined(EXPAND_THREADS)
#define EXPAND_THREADS        4
#endif
