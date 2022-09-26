#ifndef XINDB_BASE_COMPARATOR_H
#define XINDB_BASE_COMPARATOR_H

#include "Slice.h"
#include <string.h>
#include <string>

namespace xindb {

class ByteWiseComparator;

// Base class, Interface 
class Comparator {
 public:
    virtual ~Comparator() = default;

    virtual int Compare(const Slice& a, const Slice& b) const = 0;

    virtual const char* Name() const = 0;    
    
    //[start, limit)之间的一个短的串， 降低存储的空间
    // virtual void FindShortestSeparator(std::string* start, const Slice& limit) const = 0;

    //没有上端的限制
    // virtual void FindShortSuccessor(std::string* key) const = 0;

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

    // 没有写FindShort啥的
    
};



}   // namespace xindb
#endif