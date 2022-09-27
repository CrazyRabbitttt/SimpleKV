#ifndef XINDB_BASE_OPTIONS_H
#define XINDB_BASE_OPTIONS_H

namespace xindb {


class Comparator;

// 数据库运行的时候的参数
struct Options {


    // number of keys between restart pointers for delta 
    // 大致是每个Group的size？ 多少个entry构建成为一个group
    int block_restart_interval = 16;


    const Comparator* comparator;

};


}   // namespace xindb

#endif