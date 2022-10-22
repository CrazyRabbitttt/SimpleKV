#include "tableBuilder.h"

#include "filterBlock.h"
#include "Options.h"
#include "blockBuilder.h"
#include "Comparator.h"
#include "Filter.h"
#include "format.h"
#include "CrcChecksum.h"
#include "Comparator.h"
#include "Coding.h"
#include "PosixWrite.h"
#include <stdio.h>

using namespace xindb;
using namespace xindb::tinycrc;

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

uint64_t TableBuilder::NumEntries() const {
    return rep_->num_entries_; 
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

        // 添加 Index [last_key   <=====>   indexhandle]
        r->index_block_.Add(r->last_key_, Slice(handle_encoding));


        // 写完了 IndexBlock 了
        r->pending_index_entry_ = false;
    }

    // 写 DataBlock 就要同步的进行 Filter, Meta Block 的写入
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
        Flush();
    }
}


// 结束 DataBlock 的构建， 当 DataBlock 的数据预估大小超过了Size, 就 Flush 
void TableBuilder::Flush() {
    Rep* r = rep_;
    assert(!r->closed_);
    if (!ok()) return ;
    if (r->data_block_.empty()) return ;
    assert(!r->pending_index_entry_);       // 不是写 IndexBlock 的时机

    // 对于 DataBlock 压缩， 生成 Block Handle， type|crc32
    WriteBlock(&r->data_block_, &r->pending_handle_);
    if (ok()) {
        // 设置一下 下次就是写 IndexBlock 了
        r->pending_index_entry_ = true;         
        r->status_ = r->file_->Flush();         // 刷盘
    }

    // 创建 MetaBlock 
    if (r->filter_block_ != nullptr) {
        r->filter_block_->StartBlock(r->offset_);           // 传入的是 DataBlock 的offset
    }
}

// 具体的 存储数据，能够用 Snappy 进行压缩，但是这里不进行压缩
void TableBuilder::WriteBlock(BlockBuilder* block, BlockHandle* handle) {
    Rep* r = rep_;

    // 获得 DataBlock 的数据， 最终以 Slice【字符串】 形式返回所有的 datablock 内容
    Slice rawdata = block->Finish();        
    Slice block_contents;

    // 默认不进行压缩
    CompressType type = r->options_.compression;

    switch (type) {
        case kNoCompression:
            block_contents = rawdata;
            break;
        case kSnappyCompress: {
            // 没有支持 snappy 的压缩
            block_contents = rawdata;
            break;
        }
    }

    // 真正的写入数据
    WritaRawBlock(block_contents, type, handle);

    // 清空一些数据，用于标识下一个 datablock
    r->compressed_output_.clear();
    block->Reset();
}

// 具体的将 DataBlock 写入到磁盘中去 
void TableBuilder::WritaRawBlock(const Slice& block_contents, CompressType type, BlockHandle* handle) { 
    Rep* r = rep_;
    // 设置 Handle 的值
    handle->set_offset(r->offset_);
    handle->set_size(block_contents.size());

    // 具体的调用 写数据  
    r->status_ = r->file_->Append(block_contents);      // 写到磁盘缓冲区
    if (r->status_.ok()) {
        // 将 Crc, Type 啥的加到尾巴的后面
        char trailter[kBlockTrailerSize];
        trailter[0] = type;
        uint32_t crc = crc32(block_contents.data(), block_contents.size());  
        EncodeFixed32(trailter + 1, crc);
        r->status_ = r->file_->Append(Slice(trailter, kBlockTrailerSize));  
        if (r->status_.ok()) {
            r->offset_ += block_contents.size() + kBlockTrailerSize;  // block 的 offset, 
        }
    }
}


// SST 最终的结尾
Status TableBuilder::Finish() {
    Rep* r = rep_;

    // 将最后的 DataBlock 写进去
    Flush();
    assert(!r->closed_); r->closed_ = true;
    BlockHandle filterblock_handle, metaindexblock_handle, indexblock_handle;

    // 1.写 FilterBlock
    if (ok() && r->filter_block_ != nullptr) {
        // 将 filter block 的数据写到磁盘, 填充 handle 
        WritaRawBlock(r->filter_block_->Finish(), kNoCompression, &filterblock_handle);
    }

    // 2. 写 MetaIndexBlock
    if (ok()) {
        BlockBuilder meta_index_block(&r->options_);
        if (r->filter_block_ != nullptr) {
            // Key <====> handle 
            std::string key = "filter.";
            printf("Write MetaBlock, the filter policy name : [%s]\n", r->options_.filter_policy->Name());
            key.append(r->options_.filter_policy->Name());

            std::string handle_encode;
            filterblock_handle.EncodeTo(&handle_encode);        // 将 handle 数据写到 encode 
            meta_index_block.Add(key, handle_encode);           // 将 encode 写到 block
        }
        WriteBlock(&meta_index_block, &metaindexblock_handle);
    }

    // 3. Write index Block 
    if (ok()) {
        if (r->pending_index_entry_) {
            // Key   <======>   key对应的位置(handle)
            r->options_.comparator->FindShortSuccessor(&r->last_key_);
            std::string handle_encode;
            // 将 offset 等写进encode中
            r->pending_handle_.EncodeTo(&handle_encode);
            r->index_block_.Add(r->last_key_, handle_encode);
            r->pending_index_entry_ = false;
        }
        WriteBlock(&r->index_block_, &indexblock_handle);
    }

    // 4. Write Footer

    if (ok()) {
        Footer footer;
        footer.set_metaIndex_handle(metaindexblock_handle);
        footer.set_index_handle(indexblock_handle);
        std::string footer_encoding;            // 将 handle 数据编码写进字符串中
        footer.EncodeTo(&footer_encoding);
        r->status_ = r->file_->Append(footer_encoding);
        if (r->status_.ok()) {
            r->offset_ += footer_encoding.size();
        }
    }
    return r->status_;
}


uint64_t TableBuilder::FileSize() const { return rep_->offset_; }

