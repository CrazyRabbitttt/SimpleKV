#ifndef XINDB_DB_WRITEBATCH_INTERNAL_H_
#define XINDB_DB_WRITEBATCH_INTERNAL_H_

#include "WriteBatch.h"
#include "dbFormat.h"
#include "Status.h"

namespace xindb {

class MemTable;

// WriteBatch 的友元类, 辅助工具类吧
class WriteBatchInternal {

 public:
    // 放在 batch 中的 entry 的数目
    static int Count(const WriteBatch* batch);

    static void SetCount(WriteBatch* batch, int n);

    // 返回 Sequence Number[batch 开始位置的seq]
    static SequencrNumber Sequence(const WriteBatch* batch);

    // 设置一下 batch 起始位置的 seq
    static void SetSequence(WriteBatch* batch, SequencrNumber seq);

    // 返回batch的具体的存储的数据
    static Slice Contents(const WriteBatch* batch) {
        return Slice(batch->rep_);
    }

    // batch 的size
    static size_t ByteSize(const WriteBatch* batch) {
        return batch->rep_.size();
    }

    static void SetContents(const WriteBatch* batch, const Slice& contents);

    static Status InsertInto(const WriteBatch* batch, MemTable* memtable);

    static void Append(WriteBatch* dst, const WriteBatch* src);

};

}   // namespace xindb 

#endif