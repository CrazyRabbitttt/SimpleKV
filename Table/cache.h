#ifndef XINDB_TABLE_CACHE_H_
#define XINDB_TABLE_CACHE_H_

#include <cstdint>

#include "Slice.h"

namespace xindb {

class Cache;

Cache* NewLRUCache(size_t capacity);



// 接口类
class Cache {
 public:
    Cache() = default;

    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    virtual ~Cache() = default;

    struct Handle {};




 private:
    
    struct Rep;
    Rep* rep_;

};

}   // namespace xindb

#endif