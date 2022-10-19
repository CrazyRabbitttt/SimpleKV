

#include "table.h"
#include "Comparator.h"
#include "BloomFilter.h"
#include "PosixEnv.h"
#include "Options.h"
#include "filterBlock.h"
#include "two_level_iterator.h"
#include "block.h"
#include "format.h"
#include "Coding.h"

using namespace xindb;


struct Table::Rep {

    ~Rep() {
        delete filter_;
        delete[] filter_data_;
        delete index_block_;
    }

    Options options_;
    Status status_;
    RandomAccessFile* file_;            // 随机读取文件
    uint64_t cache_id;
    FilterBlockReader* filter_;
    const char* filter_data_;

    BlockHandle metaindex_handle_;      // 从 footer 中解析出来
    Block* index_block_;
};


Status Table::Open(const Options& options, RandomAccessFile* file,
                    uint64_t size, Table** table) 
{   
    // 将读取到的数据写到 table 中
    *table = nullptr;   
    if (size < Footer::kEncodedLen) {       // < 48B
        return Status::Corruption("file is too short to be an sstable");
    }

    char footer_space[Footer::kEncodedLen];
    Slice footer_input;         // 最终的读取到的内容写到这里, fixed 48 bytes

    Status s = file->Read(size - Footer::kEncodedLen, Footer::kEncodedLen, &footer_input, footer_space);

    if (!s.ok()) return s;
    Footer footer;
    s = footer.DecodeFrom(&footer_input);
    if (!s.ok()) return s;
// ========================== 上面读取 Footer =================================
// ========================== 下面读取Index Block =============================

    BlockContents index_block_contents;     // 暂时存储 block ?    
    ReadOptions read_option;        
    if (options.paranoid_check) {
        read_option.verify_checksums = true;
    }
    s = ReadBlock(file, read_option, footer.index_handle(), &index_block_contents);
    if (s.ok()) {
        // 目前已经是获取了 footer & index block 的内容了, 可以进行读取解析了呦
        Block* index_block = new Block(index_block_contents);
        Rep* rep = new Table::Rep;

        rep->options_ = options;
        rep->file_    = file;
        rep->metaindex_handle_ = footer.metaindex_handle();
        rep->index_block_ = index_block;
        rep->cache_id = 0;
        rep->filter_data_ = nullptr;
        *table = new Table(rep);
        (*table)->ReadMeta(footer);
    }
    return s;
}


// 解析 Meta block 的内容
void Table::ReadMeta(const Footer& footer) {
    ReadOptions read_options;
    // 严格的进行读取
    if (rep_->options_.paranoid_check) {
        read_options.verify_checksums = true;
    }   
    BlockContents contents;
    Status s = ReadBlock(rep_->file_, read_options, footer.metaindex_handle(), &contents);
    if (!s.ok()) {
        return ;
    }
    // block 就是辅助解析 block 的内容的类，内部有迭代器
    Block* meta = new Block(contents);
    Iterator* iter = meta->NewIterator(GetByteWiseComparator());
    std::string key = "filter.";
    // key.append(rep_->options_.filter_policy->Name());
    iter->Seek(key);                // 从 filter block 中找 "filter.xxx"
    if (iter->Valid() && iter->key() == Slice(key)) {
        ReadFilter(iter->value());
    }

    delete iter;
    delete meta;
}


// 具体的读取filter, 通过 handle ？ 
void Table::ReadFilter(const Slice& filter_handle_value) {
    // filter_handle_value ===> meta block's offset & size

    Slice v = filter_handle_value;
    BlockHandle filter_handle;

    // 解码 handle
    if (!filter_handle.DecodeFrom(&v).ok()) {
        return ;
    }

    ReadOptions read_options;
    if (rep_->options_.paranoid_check) {
        read_options.verify_checksums = true;
    }

    // 将文件偏移处的内容从文件中读取到内存里
    BlockContents block;
    if (!ReadBlock(rep_->file_, read_options, filter_handle, &block).ok()) {
        return ;
    }

    if (block.heap_allocated) {
        rep_->filter_data_ = block.data.data();
    }

    // 生成相应的 filter 
    rep_->filter_ = new FilterBlockReader(rep_->options_.filter_policy, block.data);
}

// convert index iterator's value[压缩过的 blockhandle] into corresponding block's iterator
Iterator* Table::BlockReader(void* arg, const ReadOptions& options, const Slice& index_value) {
    Table* table = reinterpret_cast<Table*>(arg);

    Block* block = nullptr;

    BlockHandle handle;
    Slice input = index_value;
    // 解码 handle
    Status s = handle.DecodeFrom(&input);

    // 将数据从 file 中解析出来，生成 Block 用于数据的访问
    if (s.ok()) {
        BlockContents contents;
        // no cache, 不实现缓存部分了，数据保持一致是很麻烦的，而且不见得有什么效果
        s = ReadBlock(table->rep_->file_, options, handle, &contents);
        if (s.ok()) {
            block = new Block(contents);
        }
    }

    // 生成能够访问 DataBlock 的 迭代器
    Iterator* iter = nullptr;
    if (block != nullptr) {
        iter = block->NewIterator(table->rep_->options_.comparator);
    } else {
        printf("生成二级迭代器失败[Table::BlockReader]\n");
    }

    return iter;
}

// Table 中创建二级迭代器的借口，内部调用 twoleveliterator
Iterator* Table::NewIterator(const ReadOptions& options) const {
    // pass the arguments table have to the function 
    return NewTwoLevelIterator(
        rep_->index_block_->NewIterator(rep_->options_.comparator),
        &Table::BlockReader, const_cast<Table*>(this), options);
}










