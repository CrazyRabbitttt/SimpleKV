#ifndef XINDB_TABLE_TABLE_H_
#define XINDB_TABLE_TABLE_H_

#include <cstdint>

#include "Status.h"
#include "format.h"
#include "Options.h"
#include "block.h"
#include "iterator"

namespace xindb {


class Block;
class BlockHandle;
class RandomAccessFile;

class Table {

 public:
    static Status Open(const Options& options, RandomAccessFile* file,
                        uint64_t file_size, Table** table);

    Table(const Table&) = delete;
    Table& operator=(const Table&) = delete;

    ~Table() = default;

    Iterator* NewIterator(const ReadOptions&) const;


 private:

    struct Rep;
    explicit Table(Rep* rep) : rep_(rep) {}

    Status InternalGet(const ReadOptions&, const Slice& key, void* arg,
                     void (*handle_result)(void* arg, const Slice& k,
                                           const Slice& v));

   // 通过传入 index_block 中对应的 handle 而确定指定的 block 的 iterator 
    static Iterator* BlockReader(void*, const ReadOptions&, const Slice&);

    void ReadMeta(const Footer& footer);
    void ReadFilter(const Slice& filter_handle_value);

    Rep* const rep_;

};

}   // namespace xindb

#endif 