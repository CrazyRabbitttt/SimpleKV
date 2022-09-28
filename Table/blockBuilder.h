#ifndef XINDB_TABLE_BLOCKBUILDER_H
#define XINDB_TABLE_BLOCKBUILDER_H

#include "Slice.h"

#include <vector>

namespace xindb {

struct Options;         // TODO:DB的cel

class BlockBuilder {

 public:    
    explicit BlockBuilder(const Options* options);

    // no copy
    BlockBuilder(const BlockBuilder&) = delete;
    BlockBuilder&operator=(const BlockBuilder&) = delete;

    // 如名，将数据按照SST的格式写到其中
    void Add(const Slice& key, const Slice& value);

    // 结束block的创建，最终将buffer封装到Slice中返回
    Slice Finish();

    size_t CurrentSizeEstimate() const;

    // 同名
    void Reset();

    bool empty() const { return buffer_.empty(); }

 private:
    // Options, 创建DB的时候的策略，还是写一个吧，始终采取默认的方式好了
    const Options* options_;
    std::string buffer_;    
    std::vector<uint32_t> restarts_;            // 重启点，盲猜每个Group为边界？
    int counter_;                               // entry的数目
    bool finished_;                             // 是否调用了Finish()
    std::string last_key_;
};


}   // namespace xindb

#endif 