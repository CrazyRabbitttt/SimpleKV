#ifndef XINDB_TABLE_TABLE_H_
#define XINDB_TABLE_TABLE_H_

#include <cstdint>

#include "Status.h"
#include "Options.h"
#include "iterator.h"

namespace xindb {


class Block;
class BlockHandle;
class RandomAccessFile;
struct Options;
struct ReadOptions;
class Footer;

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

    void ReadMeta(const Footer& footer);
    void ReadFilter(const Slice& filter_handle_value);

    Rep* const rep_;

};

}   // namespace xindb

#endif 