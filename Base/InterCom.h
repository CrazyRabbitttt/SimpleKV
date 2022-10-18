#ifndef XINDB_BASE_INTERNALCOM_H
#define XINDB_BASE_INTERNALCOM_H

#include "Comparator.h"
#include "Slice.h"

namespace xindb {


// 从InternalKey 中提取出Key出来
inline Slice ExtractUserKey(const Slice& internal_key) {
    return Slice(internal_key.data(), internal_key.size() - 8);
} 


class InternalKeyCom : Comparator{
 public:
    explicit InternalKeyCom(const Comparator* com) : user_com_(com) {}

    const char* Name() const override;

    int Compare(const Slice& a, const Slice& b) const override;

    const Comparator* user_comparator() const { return user_com_; }

    int operator()(const Slice& a, const Slice& b);

    void FindShortestSeparator(std::string* start, const Slice& limit) const override {
        user_com_->FindShortestSeparator(start, limit);
    }
    
    void FindShortSuccessor(std::string* key) const override {
        user_com_->FindShortSuccessor(key);
    }


 private:
    const Comparator* user_com_;

};
}   // namespace xindb

#endif