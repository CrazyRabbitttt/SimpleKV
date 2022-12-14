#ifndef XINDB_TABLE_FORMAT_H
#define XINDB_TABLE_FORMAT_H

#include "Slice.h"
#include "Status.h"

namespace xindb {

class RandomAccessFile;   
class Block;
struct ReadOptions;

 // 也就是封装了offset & size ???
class BlockHandle {
 public:
    enum { kMaxEncodedLength = 10 + 10 };

    BlockHandle();

    // block的offset
    uint64_t offset() const { return offset_; }
    void set_offset(uint64_t offset) { offset_ = offset; }

    uint64_t size() const { return size_; }
    void set_size(uint64_t size) { size_ = size; }

    // 将数据(offset, size)压缩到dst中
    void EncodeTo(std::string* dst) const;
    Status DecodeFrom(Slice* input);

 private:
    uint64_t offset_;
    uint64_t size_;
};

inline BlockHandle::BlockHandle() 
   : offset_(static_cast<uint64_t>(0)), size_(static_cast<uint64_t>(0)) {}

class Footer {
 public:

   // 2 handle + magicnumber
   enum { kEncodedLen = 2 * BlockHandle::kMaxEncodedLength + 8 };

   Footer() = default;

   // Meta Block
   const BlockHandle& metaindex_handle() const { return metaIndex_handle_; }
   void set_metaIndex_handle(const BlockHandle& handle) { metaIndex_handle_ = handle; }

   // IndexBlock
   const BlockHandle& index_handle() const { return index_handle_; }
   void set_index_handle(const BlockHandle& handle) { index_handle_ = handle; }

   // 将Footer中的数据(index..)encode进block的末尾
   void EncodeTo(std::string *dst) const;
   Status DecodeFrom(Slice* input);

 private:
   BlockHandle metaIndex_handle_;
   BlockHandle index_handle_; 
}; 

// datablock 的最后面的位置， type + crc32 
static const size_t kBlockTrailerSize = 5;

// 魔数
static const uint64_t kTableMagicNumber = 0xdb4775248b80fb57ull;


struct BlockContents {
   Slice data;             // 实际上的数据
   bool cacheable;         // 是否能被缓存
   bool heap_allocated;    // 是否是堆分配的， 要delete掉
};

// 读取 block 中的数据
Status ReadBlock(RandomAccessFile* file, const ReadOptions& option,
                  const BlockHandle& handle, BlockContents* result);


}   // namespace xindb 

#endif