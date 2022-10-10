#ifndef XINDB_BASE_COMPARATOR_H
#define XINDB_BASE_COMPARATOR_H

#include "Slice.h"
#include <string.h>
#include <string>

namespace xindb {

// 抽象基类, 都是虚函数
class Comparator {
 public:
    virtual ~Comparator() = default;

    virtual int Compare(const Slice& a, const Slice& b) const = 0;

    virtual const char* Name() const = 0;    
    
    // [start, limit)之间的一个短的串， 用于标识DataBlock 的边界，在写 IndexBlock 的时候创建
    virtual void FindShortestSeparator(std::string* start, const Slice& limit) const = 0;

    //没有上端的限制
    virtual void FindShortSuccessor(std::string* key) const = 0;

};

class ByteWiseComparator : public Comparator{
 public:    
    ByteWiseComparator() = default;

    ~ByteWiseComparator() = default;

    const char* Name() const override {
        return "xindb.byteWiseComparator";
    }

    int Compare(const Slice&a, const Slice& b) const override {
        return a.compare(b);
    }
    
    int operator() (const Slice& a, const Slice& b) {
        return a.compare(b);
    }

    void FindShortestSeparator(std::string* start, const Slice& limit) const override;
    
    void FindShortSuccessor(std::string* key) const override; 
};


// const Comparator* GetByteWiseComparator() {
//     static ByteWiseComparator singleton;
//     return &singleton;
// }

// const ByteWiseComparator* GetByteComparator() {
//     static ByteWiseComparator singleton;
//     return &singleton;
// }


}   // namespace xindb
#endif