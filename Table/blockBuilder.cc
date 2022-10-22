#include "blockBuilder.h"
#include "Options.h"
#include "Coding.h"
#include "Comparator.h"
#include "Coding.h"

using namespace xindb;

BlockBuilder::BlockBuilder(const Options* options)
    : options_(options), restarts_(), counter_(0), finished_(false)
{
    assert(options->block_restart_interval >= 1);
    restarts_.push_back(0);     // 第一个重启点的Offset是0
}    

Slice BlockBuilder::Finish() {
    // 结束了key value 的插入，restart代表着索引啥的，插入进去
    for (size_t i = 0; i < restarts_.size(); i++) {
        PutFixed32(&buffer_, restarts_[i]);
    }
    PutFixed32(&buffer_, restarts_.size());     // 这样就能够确认首个restart的位置
    finished_ = true;
    return Slice(buffer_);
}


// 计算一下 block 所占用的空间， 
size_t BlockBuilder::CurrentSizeEstimate() const {
    return buffer_.size() + restarts_.size() * sizeof(uint32_t) + sizeof(uint32_t);
}


void BlockBuilder::Reset() {
    buffer_.clear();
    restarts_.clear();
    restarts_.push_back(0);
    counter_ = 0;
    finished_ = false;
    last_key_.clear();
}

// 将Key, Value按照SST的格式写进buffer中
void BlockBuilder::Add(const Slice& key, const Slice& value) {
    Slice last_key_slice(last_key_);
    assert(!finished_);
    assert(counter_ <= options_->block_restart_interval);
    // 下面是要保证传入的key是有序的，
    // assert(buffer_.empty() || options_->comparator->Compare(key, last_key_slice) > 0);
    size_t shared = 0;
    if (counter_ < options_->block_restart_interval) {
        // 看一下相同的前缀的长度
        const size_t minlen = std::min(last_key_slice.size(), key.size());
        while (shared < minlen && last_key_slice[shared] == key[shared]) 
            shared++;
    } else {
        // 本 group 完成了， 下面写写一个 entry group
        restarts_.push_back(buffer_.size());                // 标记一下重启点用于二分查找
        counter_ = 0;
    }

    const size_t no_shared = key.size() - shared;

    PutVarint32(&buffer_, shared);   
    PutVarint32(&buffer_, no_shared);
    PutVarint32(&buffer_, value.size());

    // 将具体的key value存放进去
    buffer_.append(key.data() + shared, no_shared);
    buffer_.append(value.data(), value.size());

    last_key_.resize(shared);
    last_key_.append(key.data() + shared, no_shared);

    assert(Slice(last_key_) == key);

    counter_++;
}

