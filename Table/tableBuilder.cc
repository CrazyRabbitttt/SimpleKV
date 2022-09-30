#include "tableBuilder.h"
#include "filterBlock.h"
#include "Options.h"
#include "blockBuilder.h"
#include "Comparator.h"
#include "format.h"

using namespace xindb;

// tableBuilder中保存着所有的变量
struct TableBuilder::Rep {

    Rep(const Options& option, WritableFile* file) 
        : options_(option),
         index_block_options_(option),
         file_(file),
         offset_(0),
         data_block_(&option),
         index_block_(&index_block_options_),
         num_entries_(0),
         closed_(false),
         filter_block_(option.filter_policy == nullptr ? nullptr : new FilterBlockBuilder(options_.filter_policy)),
         pending_index_entry_(false)
    {
        index_block_options_.block_restart_interval = 1;
    } 


    Options options_;                        // db 的策略
    Options index_block_options_;            
    WritableFile* file_;
    uint64_t offset_;                        // DataBlock在SST中的偏移量
    Status status_;                          // 操作的状态
    BlockBuilder data_block_;                // datablock
    BlockBuilder index_block_;               // indexblock, 索引block
    std::string last_key_;                   // 创建datablock的最后一个key 
    int64_t num_entries_;                    // 当前datablock记录的数目
    bool closed_;                            // 是否是调用Finish
    FilterBlockBuilder* filter_block_;       // 


    bool pending_index_entry_;              // 是否到达生成 IndexBlock的时机， 写完一个DataBlock之后就为True
    BlockHandle pending_handle_;

    std::string compressed_output_;
};


Status TableBuilder::status() const {
    return rep_->status_;
}


TableBuilder::TableBuilder(const Options& options, WritableFile* file)
    : rep_(new Rep(options, file)) {
        if (rep_->filter_block_ != nullptr) 
            rep_->filter_block_->StartBlock(0);         // init 
    }


TableBuilder::~TableBuilder() {
    assert(rep_->closed_);
    delete rep_->filter_block_;
    delete rep_;
}

// 具体的创建SSTable
void TableBuilder::Add(const Slice& key, const Slice& value) {
    Rep* r = rep_;
    assert(!r->closed_);
    if (!ok()) return;

    if (r->num_entries_ > 0) {
        // Key 应该是升序的
        assert(r->options_.comparator->Compare(key, Slice(r->last_key_)) > 0);
    } 
    // ============= IndexBlock 的写入 =================
    // 前提条件是刚刚写完了 DataBlock, 构建下一个 DataBlock 之前写 IndexBlock
    if (r->pending_index_entry_) {
        // 刚刚写完 DataBlock， 那么应该目前是空的
        assert(r->data_block_.empty());
        // 找到一个较小的字符串作为 边界标识 last_key <= X < key
        r->options_.comparator->FindShortestSeparator(&r->last_key_, key);
        std::string handle_encoding;            
        r->pending_handle_.EncodeTo(&handle_encoding);

        // 添加 Index
        r->index_block_.Add(r->last_key_, Slice(handle_encoding));

        // 写完了 IndexBlock 了
        r->pending_index_entry_ = false;
    }

    // 写 DataBlock 就要同步的进行 Filter 的写入
    if (r->filter_block_ != nullptr) {
        r->filter_block_->AddKey(key);
    }

    // 更新 LastKey 等数据, 写到 DataBlock 中
    r->last_key_.assign(key.data(), key.size());
    r->num_entries_++;
    r->data_block_.Add(key, value);

    // 如果说 DataBlock Size 达到阈值了那么 就Flush
    const size_t etsimated_block_size = r->data_block_.CurrentSizeEstimate();
    if (etsimated_block_size >= r->options_.block_size) {

    }
}

// 能够对于 DataBlock 进行压缩，生成 BlockHandle
void TableBuilder::WriteBlock(BlockBuilder* block, BlockHandle* handle) {
    
}
