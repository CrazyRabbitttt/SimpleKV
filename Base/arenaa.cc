#include "arenaa.h"

using namespace xindb;

static const int kBlockSize = 4096;

Arena::Arena() 
    :allc_ptr(nullptr), alloc_bytes_reamining_(0), memory_usage_(0)
    {}

//析构函数直接全部释放掉
Arena::~Arena() {
    for (size_t i = 0; i < blocks_.size(); i++) {
        delete[] blocks_[i];
    }
}


char* Arena::Allocate(size_t bytes) {
    assert(bytes > 0);
    if (bytes <= alloc_bytes_reamining_) {
        //如果说需要分配的小于可供分配的，那就直接拿好了
        char* result = allc_ptr;
        allc_ptr += bytes;
        alloc_bytes_reamining_ -= bytes;
        return result;
    }
    return AllocateFallback(bytes);
}


char* Arena::AllocateFallback(size_t bytes) {
    //如果说需要的内存是 > 1K，重新进行分配，但是不会浪费
    if (bytes > kBlockSize / 4) {
        char* result = AllocateNewBlock(bytes);
        return result;
    }

    //如果不是的话就直接浪费掉（浪费空间数目小于1K）
    allc_ptr = AllocateNewBlock(kBlockSize);
    alloc_bytes_reamining_ = kBlockSize;

    //从新分配的内存中进行分配
    char* result = allc_ptr;
    allc_ptr += bytes;
    alloc_bytes_reamining_ -= bytes;
    return result;
}


//进行内存对齐的分配， 不用太关注
char* Arena::AllocateAligned(size_t bytes) {
    const int align = (sizeof(void*) > 8 ? sizeof(void*) : 8);
    //位运算技巧，判断是否是偶数
    static_assert((align & (align - 1)) == 0, "Pointer size should be a power of 1");
    //位运算技巧：进行取模运算
    size_t current_mod = reinterpret_cast<uintptr_t>(allc_ptr) & (align - 1);
    size_t slop = (current_mod == 0 ? 0 : align - current_mod);

    //下面进行内存的分配
    size_t needed = bytes + slop;
    char* result;
    if(needed <= alloc_bytes_reamining_) {
        result = allc_ptr + slop;
        allc_ptr += needed;
        alloc_bytes_reamining_ -= needed;
    } else {
        //Fall Back 肯定是返回的是对齐了的
        result = AllocateFallback(bytes);
    }

    assert((reinterpret_cast<uintptr_t>(result) & (align - 1)) == 0);
    return result;
}


//进行内存的分配
char* Arena::AllocateNewBlock(size_t block_bytes) {
    char* result = new char[block_bytes];
    blocks_.push_back(result);
    memory_usage_.fetch_add(block_bytes + sizeof(char*), std::memory_order_relaxed);
    return result;
}

