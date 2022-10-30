# SWP-Tree

> 一种新的动态适应各种倾斜负载的基于混合介质的持久化学习索引结构
>
> 本项目为其背景测试

### 测试方案：

- APEX (https://github.com/baotonglu/apex)
- LB+Tree (https://github.com/schencoding/lbtree)
- FAST&FAIR(https://github.com/DICL/FAST_FAIR)         

### 测试工具

- YCSB（https://github.com/basicthinker/YCSB-C）          


### 测试负载
- uniform负载
- zipfian负载 (α参数取值：0.6，0.7，0.8，0.9，0.95，0.99)

### 测试指标

- 吞吐量(Kops/s)

- DRAM空间占用(GB of DRAM )                                

### 环境配置

- 2 million KVs (8B key-8B value )，每个测试项执行10 million指令