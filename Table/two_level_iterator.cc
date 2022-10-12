
#include "two_level_iterator.h"
#include "iteratorWrapper.h"
#include "table.h"
#include "block.h"
#include "iterator.h"

namespace xindb {


// 定义 BlockFunction 就是这个函数， 其实可以用 function 来实现吧
// TODO:  use function<> to replace this code 
typedef Iterator* (*BlockFunction)(void*, const ReadOptions&, const Slice&);

class TwoLevelIterator : public Iterator{

 public:
    TwoLevelIterator(Iterator* index_iter, BlockFunction block_function,
                    void* arg, const ReadOptions& options);

    ~TwoLevelIterator() override;

    void Seek(const Slice& target) override;
    void SeekToFirst() override;
    void SeekToLast() override;
    void Next() override;
    void Prev() override;

    bool Valid() const override { return data_iter_.Valid(); }

    Slice key() const override {
        assert(Valid());
        return data_iter_.key();
    }

    Slice value() const override {
        assert(Valid());
        return data_iter_.value();
    }

    Status status() const override {
        if (!index_iter_.status().ok()) {
            return index_iter_.status();
        } else if (data_iter_.iter() != nullptr && !data_iter_.status().ok()) {
            return data_iter_.status();
        } else {
            return status_;
        }
    }

 private:

    // 保存一下出错的原因
    void SaveError(const Status& s) {
        if (status_.ok() && !s.ok())
            status_ = s;
    } 

    // 跳过空的 datablock 的部分
    void SkipEmptyDataBlockForward();
    void SkipEmptyDataBlockBackward();
    void SetDataIterator(Iterator* data_iter);
    void InitDataBlock();


    BlockFunction block_function_;       // 创建 data_iter 的函数，【Table::BlockReader】
    void* arg_;                          // 指向 Table 
    const ReadOptions options_;
    IteratorWrapper index_iter_;          
    IteratorWrapper data_iter_;          // 传入 iter, 内部封装了实现，缓存key 和 vaild 
    Status status_; 

    // 持有 data_iter 中的handle
    std::string data_block_handle_;     // 如果说 data_iter_ 不是空的这里的 handle 才是有效的
};



TwoLevelIterator::TwoLevelIterator(Iterator* index_iter, BlockFunction block_function,
                    void* arg, const ReadOptions& options)
    : block_function_(block_function),
      arg_(arg),            // 传入的是 Table 吧
      options_(options),
      index_iter_(index_iter),
      data_iter_(nullptr) {

        printf("twoleveliterator 的构造函数...\n");
      }
    
TwoLevelIterator::~TwoLevelIterator() = default;


// seek for the target, move the position of iter  
void TwoLevelIterator::Seek(const Slice& target) {
    // 首先一级迭代器找到 group
    index_iter_.Seek(target);
    InitDataBlock();                // 找到对应的 data iter 
    if (data_iter_.iter() != nullptr) data_iter_.Seek(target);
    
}


// 下面的迭代器的操作都是要先操作一级迭代器然后找到二级迭代器之后再进行操作

void TwoLevelIterator::SeekToFirst() {
    // seek for first entry 
    index_iter_.SeekToFirst();
    InitDataBlock();
    if (data_iter_.iter() != nullptr) data_iter_.SeekToFirst();
    SkipEmptyDataBlockForward();
}

void TwoLevelIterator::SeekToLast() {
    // seek for the last entry 
    index_iter_.SeekToLast();
    InitDataBlock();
    if (data_iter_.iter() != nullptr) data_iter_.SeekToLast();
    SkipEmptyDataBlockBackward();
}


void TwoLevelIterator::Next() {
    assert(Valid());
    data_iter_.Next();
    SkipEmptyDataBlockForward();
}


void TwoLevelIterator::Prev() {
    assert(Valid());
    data_iter_.Prev();
    SkipEmptyDataBlockBackward();
}




// 设置 data_iter
void TwoLevelIterator::SetDataIterator(Iterator* data_iter) {
    if (data_iter_.iter() != nullptr) SaveError(data_iter_.status());
    data_iter_.Set(data_iter);
}

// 从index_iterator 生成 二级 iterator
// 使得 data_iter 指向该 datablock, 通过传入的 function 
void TwoLevelIterator::InitDataBlock() {
    if (!index_iter_.Valid()) {
        SetDataIterator(nullptr);
    } else {
        // handle：offset & size 
        Slice handle = index_iter_.value();     // key <====> handle of data block 
        if (data_iter_.iter() != nullptr && handle.compare(data_block_handle_) == 0) {
            // 已经创建过了
        } else {
            Iterator* iter = (*block_function_)(arg_, options_, handle);
            // refresh the handle
            data_block_handle_.assign(handle.data(), handle.size());
            SetDataIterator(iter);
        }
    }
}


void TwoLevelIterator::SkipEmptyDataBlockForward() {
    // 从一个 datablock 跳到下一个 datablock, 中间跳过很多空的部分 
    while (data_iter_.iter() == nullptr || !data_iter_.Valid()) {
        // 移动到下一个 block 
        if (!index_iter_.Valid()) {     // index_iter 都不对那么就直接跳过好了
            SetDataIterator(nullptr);
            return ;
        }
        index_iter_.Next();
        InitDataBlock();
        if (data_iter_.iter() != nullptr) data_iter_.SeekToFirst();
    }
}


// 在想要的方向上 向前走一步
void TwoLevelIterator::SkipEmptyDataBlockBackward() {
    while (data_iter_.iter() == nullptr || !data_iter_.Valid()) {
        if (!index_iter_.Valid()) {
            SetDataIterator(nullptr);
            return ;
        }

        // 如果说有效的话，向前移动到前面的末尾？
        index_iter_.Prev();
        InitDataBlock();
        if (data_iter_.iter() != nullptr) data_iter_.SeekToLast();
    }
}


Iterator* NewTwoLevelIterator(Iterator* index_iter, 
                              BlockFunction block_function, void* arg,
                              const ReadOptions& options) {
    printf("下面初始化一个新的 twoleveliterator\n");
    return new TwoLevelIterator(index_iter, block_function, arg, options);
}


}   // namespace xindb