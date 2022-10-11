#ifndef XINDB_TABLE_TWO_LEVEL_ITERATOR_H_
#define XINDB_TABLE_TWO_LEVEL_ITERATOR_H_

#include "iterator.h"
#include "PosixEnv.h"

namespace xindb {

struct ReadOptions;

Iterator* NewTwoLevelIterator(
    Iterator* index_iter,
    Iterator* (*block_function) (void* arg, const ReadOptions& options, const Slice& index_value),
    void* arg, 
    const ReadOptions& options);


}   // namespace xindb

#endif