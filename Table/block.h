#ifndef XINDB_TABLE_BLOCK_H_
#define XINDB_TABLE_BLOCK_H_

#include <stdint.h>
#include <stddef.h>

#include "iterator.h"

namespace xindb {

struct BlockContents;
class Comparator;

class Block {                   // 用于对于数据的读取
 public:
    // 指定 content 我们来读取
    explicit Block(const BlockContents& contents);

    Block(const Block&) = delete;
    Block&operator=(const Block&) = delete;

    ~Block();

    size_t size() const { return size_; }

   Iterator* NewIterator(const Comparator* comparator);

 private:
    class Iter;                     // block 中的迭代器

    uint32_t NumRestart() const;

    const char* data_;
    size_t size_;
    uint32_t restart_offset_;       // 在 restart[0] 开始的位置
    bool owend_;                    // Block owned data_[], 是否是堆分配的
};





}   // namespace xindb

#endif