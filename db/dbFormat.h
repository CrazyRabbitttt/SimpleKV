#ifndef XINDB_BASE_DBFORMAT_H
#define XINDB_BASE_DBFORMAT_H

#include "Slice.h"
#include <unistd.h>

namespace xindb {

typedef uint64_t SequencrNumber;    

enum ValueType { kTypeDeletion = 0x0, kTypeValue = 0x1 }; 

static const ValueType kValueTypeForSeek = kTypeValue;

// 辅助类：帮助实现GET中查询数据, InternalKeySize ➕ InternalKey
class LookUpKey {
 public:
    LookUpKey(const Slice& user_key, SequencrNumber sequence);

    // no copy
    LookUpKey(const LookUpKey&) = delete;
    LookUpKey& operator=(const LookUpKey&) = delete;

    ~LookUpKey() {
        if (start_ != space_) delete[] start_;
    }

    Slice memtable_key() const { return Slice(start_, end_ - start_); }

    Slice internal_key() const { return Slice(kstart_, end_ - kstart_); }

    // User Key， Internal Key 去掉8字节的Tag
    Slice user_key() const { return Slice(kstart_, end_ - kstart_ - 8); }

 private:
    //   | klen[varint32] | UserKey[char*] | Tag[8 bytes]  |
    // start             kstart                  end 

    const char* start_;
    const char* kstart_;
    const char* end_;
    char space_[200];

};  





}   // namespace xindb

#endif