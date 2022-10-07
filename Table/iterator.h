#ifndef XINDB_TABLE_ITERATOR_H
#define XINDB_TABLE_ITERATOR_H

#include "Slice.h"
#include "Status.h"

namespace xindb {

// Base Class
class Iterator {
 public:
    Iterator();

    Iterator(const Iterator&) = delete;
    Iterator&operator=(const Iterator&) = delete;

    virtual ~Iterator();

    // 是否定位于 key-val 键值对
    virtual bool Valid() const = 0;

    // 寻找第一个key
    virtual void SeekToFirst() = 0;

    // 寻找最后一个 key
    virtual void SeekToLast() = 0;

    // Seek for target 
    virtual void Seek(const Slice& target) = 0;
    
    virtual void Next() = 0;

    virtual void Prev() = 0;

    virtual Slice key() const = 0;

    virtual Slice value() const = 0;

    virtual Status status() const = 0;

    using CleanupFunction = void (*)(void* arg1, void* arg2);
    void RegisterCleanup(CleanupFunction function, void* arg1, void* arg2);
 
 private:
    
    // Cleanup function 存储在一个链表中
    struct CleanupNode {
        bool IsEmpty() const { return function == nullptr; }
        CleanupFunction function;
        void* arg1;
        void* arg2;
        CleanupNode* next;
    };

    CleanupNode cleanup_head_;
};

// 返回一个空的 iterator
Iterator* NewEmptyIterator();

// 返回空的 指定状态的迭代器
Iterator* NewErrorIterator(const Status& status);


}   // namespace xindb

#endif