#ifndef XINDB_TABLE_FILTERBLOCK_H
#define XINDB_TABLE_FILTERBLOCK_H

#include "Slice.h"

#include <string.h>
#include <string>
#include <vector>

namespace xindb {

class FilterPolicy;

// Meta block 
class FilterBlockBuilder{
 public:
    explicit FilterBlockBuilder(const FilterPolicy*);

    // no copy 
    FilterBlockBuilder(const FilterBlockBuilder&) = delete;
    FilterBlockBuilder&operator=(const FilterBlockBuilder&) = delete;

    void StartBlock(uint64_t block_offset);
    void AddKey(const Slice& key);
    Slice Finish();

 private:

    void GenerateFilter();                      // 创建一个Filter 

    const FilterPolicy* policy_;                // 布隆过滤器
    std::string keys_;                          // User Key都塞到里面
    std::vector<size_t> start_;                 // Key的起始位置【因为是string存储的而不是vec】
    std::string result_;                        // 通过布隆计算出来的 filted data
    std::vector<Slice> tmp_keys_;               // Create Bloom的参数
    
    std::vector<uint32_t> filter_offsets_;      // filter在result中的位置，size就是布隆过滤器的个数
};


}   // namespace xindb

#endif