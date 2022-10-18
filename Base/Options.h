#ifndef XINDB_BASE_OPTIONS_H
#define XINDB_BASE_OPTIONS_H

#include "Comparator.h"

namespace xindb {

class FilterPolicy;

enum CompressType {
    kNoCompression  = 0x0,
    kSnappyCompress = 0x1
};


// 数据库运行的时候的参数
struct Options {

    Options();  

    // number of keys between restart pointers for delta 
    // 大致是每个Group的size？ 多少个entry构建成为一个group, Restart 的距离
    // int block_restart_interval = 16;
    int block_restart_interval = 4;                 // sample test, the interval assign as 4
    // SST 中 block_size 的大小
    size_t block_size = 4 * 1024;               

    // 当 DB 目录不存在的时候进行创建
    bool create_if_missing = false;

    // const Comparator* comparator;
    const ByteWiseComparator* comparator;

    // Write Buffer Size = 4M 
    size_t write_buffer_size = 4 * 1024 * 1024;

    // 压缩的策略
    CompressType compression = kSnappyCompress;

    // 极尽严格的检查，有一个数据entry不符合可能导致表打开失败
    bool paranoid_check = false;

    const FilterPolicy* filter_policy ;
};


// 读取的策略
struct ReadOptions {
    ReadOptions() = default;

    bool verify_checksums = false;

    // 是否将本迭代器读取到的数据缓存进 memory 中？
    // 批量扫描就设置为 false
    bool fill_cache = true;
};

struct WriteOptions {
  WriteOptions() = default;

  bool sync = false;

};




}   // namespace xindb

#endif