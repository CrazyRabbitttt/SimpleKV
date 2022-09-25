#ifndef XINDB_MEM_ARENA_H_
#define XINDB_MEM_ARENA_H_
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace xindb {


class Arena {
public:
    Arena();

    //no copy 
    Arena(const Arena&) = delete;
    Arena& operator =(const Arena&) = delete;

    ~Arena();

    char* Allocate(size_t bytes);

    //对齐的方式进行内存的分配
    char* AllocateAligned(size_t bytes);

    //分配内存总使用量的估计值
    size_t Memoryusage() const {
        return memory_usage_.load(std::memory_order_relaxed);
    }

private:

    // allocate mem, Return pointer
    char* AllocateFallback(size_t bytes);
    char* AllocateNewBlock(size_t bytes);

    // Allocate state
    char* allc_ptr;
    size_t alloc_bytes_reamining_;

    std::vector<char*> blocks_;

    // Total usage of arean 
    std::atomic<size_t> memory_usage_;

};

}   // namespace of xindb


#endif