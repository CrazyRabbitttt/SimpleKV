

/*

WriteBatch : 
    sequence[64byte] | count[32byte] | data [record]

Record: 
    kTypeValue
    kTypeDeletion

 varstring := len, data

*/


#include "WriteBatch.h"

#include <string>
#include <string.h>

#include "dbFormat.h"
#include "Status.h"
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

// 具体的遍历 rep_ 将数据插入到 meminserter 中
Status WriteBatch::Iterate(Handler* handler) const {
    Slice input(rep_);
    if (input.size() < kHeader) {
        return Status::Corruption("WriteBatch::Iterate error!  The size of writebatch two small..\n");
    }
    // 去掉 kHeader
    input.remove_prefix(kHeader);

    // Entry format : ValueType|KeySize|Key|ValSize|Val
    Slice key, value;
    int cnt = 0;
    while (!input.empty()) {
        cnt ++;
        char type = input[0];
        input.remove_prefix(1);
        switch (type) {
            case kTypeValue:
                if (GetLengthPrefixedSlice(&input, &key) && GetLengthPrefixedSlice(&input, &value)) {
                    // now get the key & value encoded in the rep_
                    handler->Put(key, value);
                } else {
                    return Status::Corruption("bad batch put");
                }
            break;
            case kTypeDeletion: 
                if (GetLengthPrefixedSlice(&input, &key)) {
                    // only one argument: the key 
                    handler->Delete(key);
                } else {
                    return Status::Corruption("bad batch delete");
                }

            break;
            default:
             return Status::Corruption("type of handle the entry is not [kValue] Or [kDele]");  
        }
    }
    if (cnt != WriteBatchInternal::Count(this)) {
        return Status::Corruption("WriteBatch has wrong count,【插入的数目和 rep中存储的不一样】");
    } else {
        return Status::OK();
    }
}


// Status WriteBatch::Iterate(Handler* handler) const {

// }



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

// 将数据进行删除的操作
void WriteBatch::Delete(const Slice& key) {
    WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
    rep_.push_back(static_cast<char>(kTypeDeletion));
    PutLengthPrefixedSlice(&rep_, key);
}

// 将 source 中的数据 append 到 this 中
void WriteBatch::Append(const WriteBatch& source) {
    WriteBatchInternal::Append(this, &source);
}

// 设置一下需要插入到的 MemTable 中, 将 batch 中的数据写到 对应的 MemTable 中
Status WriteBatchInternal::InsertInto(const WriteBatch* batch, MemTable* memtable) {
    MemTableInsert inserter;
    inserter.sequence_ = WriteBatchInternal::Sequence(batch);
    inserter.mem_ = memtable;
    return batch->Iterate(&inserter);
}


void WriteBatchInternal::SetContents(WriteBatch* batch, const Slice& contents) {
    assert(contents.size() >= kHeader);
    // batch->rep_.assign(contents.data(), contents.size());
    batch->rep_.assign(contents.data(), contents.size());
}


void WriteBatchInternal::Append(WriteBatch* dst, const WriteBatch* src) {
    SetCount(dst, Count(dst) + Count(src));
    assert(src->rep_.size() >= kHeader);
    dst->rep_.append(src->rep_.data() + kHeader, src->rep_.size() - kHeader);
}



}   // namespace xindb 


