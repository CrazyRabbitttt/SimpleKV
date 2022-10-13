#ifndef XINDB_TABLE_ITERATOR_WRAPPER_H
#define XINDB_TABLE_ITERATOR_WRAPPER_H

#include "iterator.h"
#include "Slice.h"

namespace xindb {

// 包装了一下 iterator
class IteratorWrapper {

 public:
    IteratorWrapper()
        : iter_(nullptr), valid_(false) {}
    explicit IteratorWrapper(Iterator* iter)        // 传进来的 iter 是堆区开辟的 ???
        : iter_(nullptr) {
            Set(iter);                              // nnd， 忘了这一步了，全是 nullptr 了
        }
    ~IteratorWrapper() { delete iter_; }
    Iterator* iter() const { return iter_; }


    // 设置 iter 为新传入的 iterator
    void Set(Iterator* iter) {
        delete iter_;
        iter_ = iter;
        if (iter_ == nullptr) {
            valid_ = false;
        } else {
            Update();
        }
    }

    bool Valid() const { return valid_; }

    Slice key() const {
        assert(Valid());
        return key_;
    }

    Slice value() const {
        assert(Valid());
        return iter_->value();
    }

    Status status() const {
        assert(iter_);
        return iter_->status();
    }

    // 将值暂时存储到key中， 就不用一直调用 key(), vlaue() 这些虚函数了咩
    void Next() {
        assert(iter_);
        iter_->Next();
        Update();
    }

    void Prev() {
        assert(iter_);
        iter_->Prev();
        Update();
    }

    void Seek(const Slice& k) {
        assert(iter_);
        iter_->Seek(k);
        Update();
    }

    void SeekToFirst() {
        assert(iter_);
        iter_->SeekToFirst();
        Update();
    }

    void SeekToLast() {
        assert(iter_);
        iter_->SeekToLast();
        Update();
    }


 private:

    void Update() {
        valid_ = iter_->Valid();
        if (valid_) {
            key_ = iter_->key();        // 暂时存储 key 
        }
    }


    Iterator* iter_;
    bool valid_;
    Slice key_;
};

}   // namesapce xindb

#endif