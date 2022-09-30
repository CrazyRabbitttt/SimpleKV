#ifndef XINDB_BASE_OPTIONS_H
#define XINDB_BASE_OPTIONS_H

namespace xindb {


class Comparator;

enum CompressType {
    kNoCompression  = 0x0,
    kSnappyCompress = 0x1
};


// 数据库运行的时候的参数
struct Options {

    Options() = default;  

    // number of keys between restart pointers for delta 
    // 大致是每个Group的size？ 多少个entry构建成为一个group
    int block_restart_interval = 16;

    // SST 中 block_size 的大小
    size_t block_size = 4 * 1024;               

    // 当 DB 目录不存在的时候进行创建
    bool create_if_missing = false;

    const Comparator* comparator;

    CompressType compression = kSnappyCompress;

    const FilterPolicy* filter_policy = nullptr;
};


}   // namespace xindb

#endif