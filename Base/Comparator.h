#ifndef XINDB_BASE_COMPARATOR_H
#define XINDB_BASE_COMPARATOR_H

namespace xindb {

class Slice;

// Base class, Interface 
class Comparator {
 public:
    virtual ~Comparator() = default;

    virtual int Compare(const Slice& a, const Slice& b) const = 0;

    virtual const char* Name() const = 0;    
    

};

}   // namespace xindb
#endif