

/*

WriteBatch : 
    sequence[64byte] | count[32byte] | data [record]

Record: 
    kTypeValue
    kTypeDeletion

 varstring := len, data

*/


#include "WriteBatch.h"

#include "dbFormat.h"
#include "WriteBatchInternal.h"
#include "memTable.h"
#include "Coding.h"


namespace xindb {

static const size_t kHeader = 12;

WriteBatch::WriteBatch() {
    Clear();
}

WriteBatch::~WriteBatch() = default;

WriteBatch::Handler::~Handler() = default;

void WriteBatch::Clear() {
    rep_.clear();
    rep_.resize(kHeader);           // 12 字节长度的【序列号，数目】
}


void WriteBatchInternal::SetCount(WriteBatch* batch, int n) {
    EncodeFixed32(&batch->rep_[8], n);
}

int WriteBatchInternal::Count(const WriteBatch* batch) {
    return DecodeFixed32(batch->rep_.data() + 8);
}

SequencrNumber WriteBatchInternal::Sequence(const WriteBatch* batch) {
    return SequencrNumber(DecodeFixed64(batch->rep_.data()));
}

void WriteBatchInternal::SetSequence(WriteBatch* batch, SequencrNumber seq) {
    EncodeFixed64(&batch->rep_[0], seq);
}


void WriteBatch::Put(const Slice& key, const Slice& value) {
    // 首先设置好 seq， static 成员函数直接调用
    WriteBatchInternal::SetCount(this, WriteBatchInternal::Sequence(this) + 1);
    rep_.push_back(static_cast<char>(kTypeValue));
    PutLengthPrefixedSlice(&rep_, key);
    PutLengthPrefixedSlice(&rep_, value);
}

void WriteBatch::Delete(const Slice& key) {
    WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
    rep_.push_back(static_cast<char>(kTypeDeletion));
    PutLengthPrefixedSlice(&rep_, key);
}



}   // namespace xindb 


