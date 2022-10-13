#ifndef XINDB_TABLE_FILETER_H
#define XINDB_TABLE_FILETER_H

#include <string.h>
#include <string>

namespace xindb {

class Slice;

class FilterPolicy {        // 过滤的策略，一般就是布隆过滤器
 public:
    virtual ~FilterPolicy() = default;

    virtual const char* Name() const = 0;

    virtual void CreateFilter(const Slice* keys, int n, std::string* dst) const = 0;

    virtual bool KeyMayMatch(const Slice& key, const Slice& filter) const = 0;

};



}   // namespace xindb 

#endif